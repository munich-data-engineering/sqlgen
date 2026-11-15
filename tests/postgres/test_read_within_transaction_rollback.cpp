#ifndef SQLGEN_BUILD_DRY_TESTS_ONLY

#include <gtest/gtest.h>

#include <rfl.hpp>
#include <rfl/json.hpp>
#include <sqlgen.hpp>
#include <sqlgen/postgres.hpp>
#include <vector>
#include "test_helpers.hpp"

namespace test_read_within_transaction_rollback {

struct Person {
  sqlgen::PrimaryKey<uint32_t> id;
  std::string first_name;
  std::string last_name;
  int age;
};

TEST(postgres, test_read_within_transaction_rollback) {
  const auto existing = std::vector<Person>(
      {Person{.id = 0, .first_name = "Homer", .last_name = "Simpson", .age = 45}});

  const auto to_insert = std::vector<Person>(
      {Person{.id = 1, .first_name = "Marge", .last_name = "Simpson", .age = 44}});

  const auto credentials = sqlgen::postgres::test::make_credentials();

  using namespace sqlgen;
  using namespace sqlgen::literals;

  const auto conn =
      sqlgen::postgres::connect(credentials).and_then(drop<Person> | if_exists);

  sqlgen::write(conn, existing).value();

  const auto read_all = sqlgen::read<std::vector<Person>> | order_by("id"_c);

  const auto read_before = begin_transaction(sqlgen::postgres::connect(credentials))
                               .and_then(commit)
                               .and_then(read_all)
                               .value();

  // Within a transaction: write, then read (which goes through a cursor
  // iterator). The read must not commit the transaction, so a subsequent
  // rollback undoes the write. If the read committed the transaction, the
  // inserted row would survive the rollback and read_after would differ from
  // read_before.
  const auto txn =
      begin_transaction(sqlgen::postgres::connect(credentials)).value();

  insert(std::ref(to_insert))(txn->conn()).value();

  const auto read_in_txn = read_all(txn->conn()).value();
  EXPECT_EQ(read_in_txn.size(), 2);

  txn->rollback().value();

  const auto read_after = begin_transaction(sqlgen::postgres::connect(credentials))
                              .and_then(commit)
                              .and_then(read_all)
                              .value();

  EXPECT_EQ(rfl::json::write(read_before), rfl::json::write(read_after));
}

}  // namespace test_read_within_transaction_rollback

#endif

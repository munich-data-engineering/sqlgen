#ifndef SQLGEN_BUILD_DRY_TESTS_ONLY

#include <gtest/gtest.h>

#include <rfl.hpp>
#include <rfl/json.hpp>
#include <sqlgen.hpp>
#include <sqlgen/mysql.hpp>
#include <vector>
#include "test_helpers.hpp"

namespace test_in_with_member_pointers {

struct Person {
  sqlgen::PrimaryKey<uint32_t> id;
  std::string first_name;
  std::string last_name;
  int age;
};

TEST(mysql, test_in_with_member_pointers) {
  using namespace sqlgen;
  const auto people1 = std::vector<Person>(
      {Person{
           .id = 0, .first_name = "Homer", .last_name = "Simpson", .age = 45},
       Person{.id = 1, .first_name = "Bart", .last_name = "Simpson", .age = 10},
       Person{.id = 2, .first_name = "Lisa", .last_name = "Simpson", .age = 8},
       Person{
           .id = 3, .first_name = "Maggie", .last_name = "Simpson", .age = 0},
       Person{
           .id = 4, .first_name = "Hugo", .last_name = "Simpson", .age = 10}});

  const auto credentials = sqlgen::mysql::test::make_credentials();

  const auto people2 =
      mysql::connect(credentials)
          .and_then(drop<Person> | if_exists)
          .and_then(write(std::ref(people1)))
          .and_then(sqlgen::read<std::vector<Person>> |
                    where(col<&Person::first_name>
                              .in("Bart", "Lisa", "Maggie")) |
                    order_by(col<&Person::age>))
          .value();

  const std::string expected_query =
      R"(SELECT `id`, `first_name`, `last_name`, `age` FROM `Person` WHERE `Person`.`first_name` IN ('Bart', 'Lisa', 'Maggie') ORDER BY `Person`.`age`)";
  const std::string expected1 =
      R"([{"id":3,"first_name":"Maggie","last_name":"Simpson","age":0},{"id":2,"first_name":"Lisa","last_name":"Simpson","age":8},{"id":1,"first_name":"Bart","last_name":"Simpson","age":10}])";

  EXPECT_EQ(mysql::to_sql(sqlgen::read<std::vector<Person>> |
                               where(col<&Person::first_name>
                                         .in("Bart", "Lisa", "Maggie")) |
                               order_by(col<&Person::age>)),
            expected_query);
  EXPECT_EQ(rfl::json::write(people2), expected1);
}

}  // namespace test_in_with_member_pointers

#endif

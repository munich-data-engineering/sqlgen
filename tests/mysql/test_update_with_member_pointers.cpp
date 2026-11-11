#ifndef SQLGEN_BUILD_DRY_TESTS_ONLY

#include <gtest/gtest.h>

#include <rfl.hpp>
#include <rfl/json.hpp>
#include <sqlgen.hpp>
#include <sqlgen/mysql.hpp>
#include <vector>
#include "test_helpers.hpp"

namespace test_update_with_member_pointers {

struct Person {
  sqlgen::PrimaryKey<uint32_t> id;
  std::string first_name;
  std::string last_name;
  int age;
};

TEST(mysql, test_update_with_member_pointers) {
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

  using namespace sqlgen;
  using namespace sqlgen::literals;

  const auto query = update<Person>(
                         col<&Person::first_name>.set(col<&Person::last_name>),
                         col<&Person::age>.set(100)) |
                     where(col<&Person::first_name> == "Hugo");

  const auto result =
      mysql::connect(credentials)
          .and_then(drop<Person> | if_exists)
          .and_then(write(std::ref(people1)))
          .and_then(query)
          .and_then(sqlgen::read<std::vector<Person>>)
          .value();

  const std::string expected =
      R"([{"id":0,"first_name":"Homer","last_name":"Simpson","age":45},{"id":1,"first_name":"Bart","last_name":"Simpson","age":10},{"id":2,"first_name":"Lisa","last_name":"Simpson","age":8},{"id":3,"first_name":"Maggie","last_name":"Simpson","age":0},{"id":4,"first_name":"Simpson","last_name":"Simpson","age":100}])";

  EXPECT_EQ(rfl::json::write(result), expected);
}

}  // namespace test_update_with_member_pointers

#endif

#ifndef SQLGEN_BUILD_DRY_TESTS_ONLY

#include <gtest/gtest.h>

#include <rfl.hpp>
#include <rfl/json.hpp>
#include <sqlgen.hpp>
#include <sqlgen/mysql.hpp>
#include <vector>
#include "test_helpers.hpp"

namespace test_where_with_member_pointers {

struct Person {
  sqlgen::PrimaryKey<uint32_t> id;
  std::string first_name;
  std::string last_name;
  int age;
};

struct PersonSummary {
  std::string first_name;
  int age;
};

TEST(mysql, test_where_with_member_pointers) {
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

  const auto query =
      select_from<Person>(col<&Person::first_name> | as<"first_name">,
                          col<&Person::age> | as<"age">) |
      where(col<&Person::age> >= 8 and col<&Person::first_name> != "Hugo") |
      order_by(col<&Person::age>) |
      to<std::vector<PersonSummary>>;

  const auto people2 =
      mysql::connect(credentials)
          .and_then(drop<Person> | if_exists)
          .and_then(write(std::ref(people1)))
          .and_then(query)
          .value();

  const std::string expected_query =
      R"(SELECT `Person`.`first_name` AS `first_name`, `Person`.`age` AS `age` FROM `Person` WHERE (`Person`.`age` >= 8) AND (`Person`.`first_name` != 'Hugo') ORDER BY `Person`.`age`)";

  EXPECT_EQ(mysql::to_sql(query), expected_query);
  EXPECT_EQ(rfl::json::write(people2),
            R"([{"first_name":"Lisa","age":8},{"first_name":"Bart","age":10},{"first_name":"Homer","age":45}])");
}

}  // namespace test_where_with_member_pointers

#endif

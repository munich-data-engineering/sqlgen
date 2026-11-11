#include <gtest/gtest.h>

#include <rfl.hpp>
#include <rfl/json.hpp>
#include <sqlgen.hpp>
#include <sqlgen/mysql.hpp>

namespace test_full_join_member_ptr_dry {

struct Person {
  sqlgen::PrimaryKey<uint32_t> id;
  std::string first_name;
  std::string last_name;
};

struct Pet {
  sqlgen::PrimaryKey<uint32_t> id;
  std::string name;
  uint32_t owner_id;
};

// MySQL cannot execute a FULL JOIN, so this test only verifies that the
// member-pointer syntax transpiles to the expected SQL for the mysql dialect.
TEST(mysql, test_full_join_member_ptr_dry) {
  using namespace sqlgen;
  using namespace sqlgen::literals;

  const auto get_all =
      select_from<Person>(col<&Person::first_name> | as<"first_name">,
                          col<&Person::last_name> | as<"last_name">,
                          col<&Pet::name> | as<"pet_name">) |
      full_join<Pet>(col<&Person::id> == col<&Pet::owner_id>) |
      order_by(col<&Person::id>, col<&Pet::id>);

  const std::string expected_query =
      R"(SELECT `Person`.`first_name` AS `first_name`, `Person`.`last_name` AS `last_name`, `Pet`.`name` AS `pet_name` FROM `Person` FULL JOIN `Pet`  ON `Person`.`id` = `Pet`.`owner_id` ORDER BY `Person`.`id`, `Pet`.`id`)";

  EXPECT_EQ(mysql::to_sql(get_all), expected_query);
}

}  // namespace test_full_join_member_ptr_dry

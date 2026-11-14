
#include <gtest/gtest.h>

#include <ranges>
#include <rfl.hpp>
#include <rfl/json.hpp>
#include <sqlgen.hpp>
#include <sqlgen/sqlite.hpp>
#include <vector>

namespace test_group_by_with_table_names {

struct Person {
  sqlgen::PrimaryKey<uint32_t> id;
  std::string first_name;
  std::string last_name;
  int age;
};

TEST(sqlite, test_group_by_with_table_names) {
  const auto people1 = std::vector<Person>(
      {Person{
           .id = 0, .first_name = "Homer", .last_name = "Simpson", .age = 45},
       Person{.id = 1, .first_name = "Bart", .last_name = "Simpson", .age = 10},
       Person{.id = 2, .first_name = "Lisa", .last_name = "Simpson", .age = 8},
       Person{
           .id = 3, .first_name = "Maggie", .last_name = "Simpson", .age = 0}});

  using namespace sqlgen;
  using namespace sqlgen::literals;

  struct Children {
    std::string last_name;
    int num_children;
    int num_last_names;
    double avg_age;
    double max_age;
    double min_age;
    double sum_age;
  };

  const auto get_children =
      select_from<Person>(
          col<&Person::last_name>, avg(col<&Person::age>).as<"avg_age">(),
          count().as<"num_children">(), max(col<&Person::age>).as<"max_age">(),
          min(col<&Person::age>).as<"min_age">(),
          sum(col<&Person::age>).as<"sum_age">(),
          count_distinct(col<&Person::last_name>).as<"num_last_names">()) |
      where(col<&Person::age> < 18) | group_by(col<&Person::last_name>) |
      to<std::vector<Children>>;

  const auto children = sqlite::connect()
                            .and_then(drop<Person> | if_exists)
                            .and_then(write(std::ref(people1)))
                            .and_then(get_children)
                            .value();

  EXPECT_EQ(children.size(), 1);
  EXPECT_EQ(children.at(0).last_name, "Simpson");
  EXPECT_EQ(children.at(0).num_children, 3);
  EXPECT_EQ(children.at(0).num_last_names, 1);
  EXPECT_EQ(children.at(0).avg_age, 6.0);
  EXPECT_EQ(children.at(0).max_age, 10.0);
  EXPECT_EQ(children.at(0).min_age, 0.0);
  EXPECT_EQ(children.at(0).sum_age, 18.0);
}

}  // namespace test_group_by_with_table_names

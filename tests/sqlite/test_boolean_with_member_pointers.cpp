
#include <gtest/gtest.h>

#include <rfl.hpp>
#include <rfl/json.hpp>
#include <sqlgen.hpp>
#include <sqlgen/sqlite.hpp>
#include <vector>

namespace test_boolean_with_member_pointers {

struct Person {
  sqlgen::PrimaryKey<uint32_t> id;
  std::string first_name;
  std::string last_name;
  bool has_children;
};

TEST(sqlite, test_boolean_with_member_pointers) {
  using namespace sqlgen;
  const auto people1 = std::vector<Person>({Person{.id = 0,
                                                   .first_name = "Homer",
                                                   .last_name = "Simpson",
                                                   .has_children = true},
                                            Person{.id = 1,
                                                   .first_name = "Bart",
                                                   .last_name = "Simpson",
                                                   .has_children = false},
                                            Person{.id = 2,
                                                   .first_name = "Lisa",
                                                   .last_name = "Simpson",
                                                   .has_children = false},
                                            Person{.id = 3,
                                                   .first_name = "Maggie",
                                                   .last_name = "Simpson",
                                                   .has_children = false}});

  const auto conn = sqlite::connect();

  const auto with_children =
      conn.and_then(write(std::ref(people1)))
          .and_then(sqlgen::read<std::vector<Person>> |
                    where(col<&Person::has_children> == true) |
                    order_by(col<&Person::id>))
          .value();

  const auto without_children =
      conn.and_then(sqlgen::read<std::vector<Person>> |
                    where(col<&Person::has_children> == false) |
                    order_by(col<&Person::id>))
          .value();

  EXPECT_EQ(rfl::json::write(with_children),
            R"([{"id":0,"first_name":"Homer","last_name":"Simpson","has_children":true}])");
  EXPECT_EQ(rfl::json::write(without_children),
            R"([{"id":1,"first_name":"Bart","last_name":"Simpson","has_children":false},{"id":2,"first_name":"Lisa","last_name":"Simpson","has_children":false},{"id":3,"first_name":"Maggie","last_name":"Simpson","has_children":false}])");
}

}  // namespace test_boolean_with_member_pointers

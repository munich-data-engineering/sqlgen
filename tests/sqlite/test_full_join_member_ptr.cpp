
#include <gtest/gtest.h>

#include <optional>
#include <rfl.hpp>
#include <rfl/json.hpp>
#include <sqlgen.hpp>
#include <sqlgen/sqlite.hpp>
#include <vector>

namespace test_full_join_member_ptr {

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

TEST(sqlite, test_full_join_member_ptr) {
  struct PersonAndPet {
    std::optional<std::string> first_name;
    std::optional<std::string> last_name;
    std::optional<std::string> pet_name;
  };

  const auto people = std::vector<Person>({
      Person{.id = 1, .first_name = "Homer", .last_name = "Simpson"},
      Person{.id = 2, .first_name = "Marge", .last_name = "Simpson"},
      Person{.id = 3, .first_name = "Bart", .last_name = "Simpson"},
      Person{.id = 4, .first_name = "Lisa", .last_name = "Simpson"},
  });

  const auto pets = std::vector<Pet>({
      Pet{.id = 1, .name = "Santa's Little Helper", .owner_id = 1},
      Pet{.id = 2, .name = "Snowball", .owner_id = 4},
      Pet{.id = 3, .name = "Mr. Teeny", .owner_id = 99},
  });

  using namespace sqlgen;
  using namespace sqlgen::literals;

  const auto get_all =
      select_from<Person>(col<&Person::first_name> | as<"first_name">,
                          col<&Person::last_name> | as<"last_name">,
                          col<&Pet::name> | as<"pet_name">) |
      full_join<Pet>(col<&Person::id> == col<&Pet::owner_id>) |
      order_by(col<&Person::id>, col<&Pet::id>) |
      to<std::vector<PersonAndPet>>;

  const auto result = sqlite::connect()
                          .and_then(write(std::ref(people)))
                          .and_then(write(std::ref(pets)))
                          .and_then(get_all)
                          .value();

  const std::string expected_query =
      R"(SELECT "Person"."first_name" AS "first_name", "Person"."last_name" AS "last_name", "Pet"."name" AS "pet_name" FROM "Person" FULL JOIN "Pet"  ON "Person"."id" = "Pet"."owner_id" ORDER BY "Person"."id", "Pet"."id")";
  const std::string expected_json =
      R"([{"pet_name":"Mr. Teeny"},{"first_name":"Homer","last_name":"Simpson","pet_name":"Santa's Little Helper"},{"first_name":"Marge","last_name":"Simpson"},{"first_name":"Bart","last_name":"Simpson"},{"first_name":"Lisa","last_name":"Simpson","pet_name":"Snowball"}])";

  EXPECT_EQ(sqlite::to_sql(get_all), expected_query);
  EXPECT_EQ(rfl::json::write(result), expected_json);
}

}  // namespace test_full_join_member_ptr

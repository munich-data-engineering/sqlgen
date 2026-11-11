
#include <gtest/gtest.h>

#include <optional>
#include <rfl.hpp>
#include <rfl/json.hpp>
#include <sqlgen.hpp>
#include <sqlgen/sqlite.hpp>
#include <vector>

namespace test_inner_join_with_alias {

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

TEST(sqlite, test_inner_join_with_alias) {
  struct PersonWithPet {
    std::string first_name;
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

  const auto get_people_with_pets =
      select_from<Person, "per">(
          col<&Person::first_name, "per"> | as<"first_name">,
          col<&Pet::name, "pet"> | as<"pet_name">) |
      inner_join<Pet, "pet">(col<&Person::id, "per"> ==
                             col<&Pet::owner_id, "pet">) |
      order_by(col<&Person::id, "per">) | to<std::vector<PersonWithPet>>;

  const auto result = sqlite::connect()
                          .and_then(write(std::ref(people)))
                          .and_then(write(std::ref(pets)))
                          .and_then(get_people_with_pets)
                          .value();

  const std::string expected_query =
      R"(SELECT per."first_name" AS "first_name", pet."name" AS "pet_name" FROM "Person" per INNER JOIN "Pet" pet ON per."id" = pet."owner_id" ORDER BY per."id")";
  const std::string expected_json =
      R"([{"first_name":"Homer","pet_name":"Santa's Little Helper"},{"first_name":"Lisa","pet_name":"Snowball"}])";

  EXPECT_EQ(sqlite::to_sql(get_people_with_pets), expected_query);
  EXPECT_EQ(rfl::json::write(result), expected_json);
}

}  // namespace test_inner_join_with_alias

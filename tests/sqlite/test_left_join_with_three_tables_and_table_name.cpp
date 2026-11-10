#include <gtest/gtest.h>

#include <optional>
#include <rfl.hpp>
#include <rfl/json.hpp>
#include <sqlgen.hpp>
#include <sqlgen/sqlite.hpp>
#include <vector>

namespace test_join_with_three_tables_and_table_name {

TEST(sqlite, test_join_with_three_tables_and_table_name) {
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

  struct Toy {
    sqlgen::PrimaryKey<uint32_t> id;
    std::string description;
    uint32_t pet_id;
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

  const auto toys = std::vector<Toy>({
      Toy{.id = 1, .description = "Red Ball", .pet_id = 1},
      Toy{.id = 2, .description = "Blue Ball", .pet_id = 1},
  });

  using namespace sqlgen;

  struct PersonWithPet {
    std::string first_name;
    std::string last_name;
    std::optional<std::string> pet_name;
    std::optional<std::string> toy_description;
  };

  const auto get_people_with_pets =
      select_from<Person>(col<&Person::first_name> | as<"first_name">,
                          col<&Person::last_name> | as<"last_name">,
                          col<&Pet::name> | as<"pet_name">,
                          col<&Toy::description> | as<"toy_description">) |
      left_join<Pet>(col<&Person::id> == col<&Pet::owner_id>) |
      left_join<Toy>(col<&Pet::id> == col<&Toy::pet_id>) |
      order_by(col<&Person::id>) | to<std::vector<PersonWithPet>>;

  const auto result = sqlite::connect()
                          .and_then(write(std::ref(people)))
                          .and_then(write(std::ref(pets)))
                          .and_then(write(std::ref(toys)))
                          .and_then(get_people_with_pets)
                          .value();

  const std::string expected_query =
      R"(SELECT "Person"."first_name" AS "first_name", "Person"."last_name" AS "last_name", "Pet"."name" AS "pet_name", "Toy"."description" AS "toy_description" FROM "Person" LEFT JOIN "Pet"  ON "Person"."id" = "Pet"."owner_id" LEFT JOIN "Toy"  ON "Pet"."id" = "Toy"."pet_id" ORDER BY "Person"."id")";
  const std::string expected_json =
      R"([{"first_name":"Homer","last_name":"Simpson","pet_name":"Santa's Little Helper","toy_description":"Blue Ball"},{"first_name":"Homer","last_name":"Simpson","pet_name":"Santa's Little Helper","toy_description":"Red Ball"},{"first_name":"Marge","last_name":"Simpson"},{"first_name":"Bart","last_name":"Simpson"},{"first_name":"Lisa","last_name":"Simpson","pet_name":"Snowball"}])";

  EXPECT_EQ(sqlite::to_sql(get_people_with_pets), expected_query);
  EXPECT_EQ(rfl::json::write(result), expected_json);
}

}  // namespace test_join_with_three_tables_and_table_name

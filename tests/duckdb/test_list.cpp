#include <gtest/gtest.h>

#include <optional>
#include <sqlgen.hpp>
#include <sqlgen/duckdb.hpp>

namespace test_list {

struct ListTable {
  int32_t id;
  std::vector<int32_t> numbers;
  std::vector<std::string> labels;
  std::vector<std::vector<int32_t>> nested;
  std::optional<std::vector<int32_t>> nullable_list;
  std::vector<std::optional<int32_t>> nullable_elements;

  bool operator==(const ListTable&) const = default;
};

TEST(duckdb, native_list_round_trip) {
  EXPECT_EQ(
      sqlgen::duckdb::to_sql(sqlgen::create_table<ListTable>),
      R"(CREATE TABLE IF NOT EXISTS "ListTable" ("id" INTEGER NOT NULL, "numbers" INTEGER[] NOT NULL, "labels" TEXT[] NOT NULL, "nested" INTEGER[][] NOT NULL, "nullable_list" INTEGER[], "nullable_elements" INTEGER[] NOT NULL);)");

  const auto conn = sqlgen::duckdb::connect().value();
  ASSERT_TRUE(sqlgen::create_table<ListTable>(conn));

  const std::vector<ListTable> expected{
      {.id = 1,
       .numbers = {1, 2, 3},
       .labels = {"a", "comma,value", "NULL", "quote\"slash\\"},
       .nested = {{1, 2}, {}, {3}},
       .nullable_list = std::vector<int32_t>{4, 5},
       .nullable_elements = {6, std::nullopt, 7}},
      {.id = 2,
       .numbers = {},
       .labels = {},
       .nested = {},
       .nullable_list = std::nullopt,
       .nullable_elements = {}}};
  ASSERT_TRUE(sqlgen::insert(expected)(conn));
  const auto actual = sqlgen::read<std::vector<ListTable>>(conn);
  ASSERT_TRUE(actual);
  EXPECT_EQ(*actual, expected);
}

}  // namespace test_list

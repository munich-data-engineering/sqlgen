#include <gtest/gtest.h>

#include <sqlgen.hpp>
#include <sqlgen/parsing/Parser.hpp>
#include <sqlgen/postgres.hpp>

namespace test_array_dry {

struct ArrayTable {
  std::vector<int32_t> numbers;
  std::vector<std::string> labels;
  std::vector<std::vector<int64_t>> matrix;
};

TEST(postgres, array_schema_uses_native_types) {
  const auto expected =
      R"(CREATE TABLE IF NOT EXISTS "ArrayTable" ("numbers" INTEGER[] NOT NULL, "labels" TEXT[] NOT NULL, "matrix" BIGINT[][] NOT NULL);)";
  EXPECT_EQ(sqlgen::postgres::to_sql(sqlgen::create_table<ArrayTable>),
            expected);
}

TEST(postgres, array_text_format_round_trip) {
  const std::vector<std::string> expected{"comma,value", "quote\"value",
                                          "back\\slash", "NULL", ""};
  using Values = std::remove_cvref_t<decltype(expected)>;
  const auto encoded = sqlgen::parsing::Parser<Values>::write(expected);
  ASSERT_TRUE(encoded);
  const auto decoded = sqlgen::parsing::Parser<Values>::read(encoded);
  ASSERT_TRUE(decoded);
  EXPECT_EQ(*decoded, expected);
}

}  // namespace test_array_dry

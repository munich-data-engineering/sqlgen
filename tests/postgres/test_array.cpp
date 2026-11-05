#ifndef SQLGEN_BUILD_DRY_TESTS_ONLY

#include <gtest/gtest.h>

#include <rfl/json.hpp>
#include <sqlgen.hpp>
#include <sqlgen/postgres.hpp>

#include "test_helpers.hpp"

namespace test_array {

struct ArrayRecord {
  sqlgen::PrimaryKey<int32_t> id;
  std::vector<int32_t> numbers;
  std::vector<std::string> labels;
  std::vector<std::optional<int64_t>> nullable_elements;
  std::optional<std::vector<double>> nullable_array;
  std::vector<std::vector<int16_t>> matrix;
};

TEST(postgres, native_array_round_trip) {
  const std::vector<ArrayRecord> expected{
      {.id = 1,
       .numbers = {1, 2, 3},
       .labels = {"comma,value", "quote\"value", "back\\slash", "NULL", "",
                  "tab\tvalue", "line\nvalue"},
       .nullable_elements = {10, std::nullopt, 30},
       .nullable_array = std::vector<double>{1.25, 2.5},
       .matrix = {{1, 2}, {3, 4}}},
      {.id = 2,
       .numbers = {},
       .labels = {},
       .nullable_elements = {},
       .nullable_array = std::nullopt,
       .matrix = {}}};

  const auto conn =
      sqlgen::postgres::connect(sqlgen::postgres::test::make_credentials())
          .and_then(sqlgen::drop<ArrayRecord> | sqlgen::if_exists);
  const auto actual = sqlgen::write(conn, expected)
                          .and_then(sqlgen::read<std::vector<ArrayRecord>>)
                          .value();

  EXPECT_EQ(rfl::json::write(actual), rfl::json::write(expected));
}

}  // namespace test_array

#endif

#ifndef SQLGEN_DUCKDB_WRITE_DATA_CHUNK_HPP_
#define SQLGEN_DUCKDB_WRITE_DATA_CHUNK_HPP_

#include <duckdb.h>

#include <memory>
#include <optional>
#include <rfl.hpp>
#include <type_traits>
#include <vector>

#include "../Result.hpp"
#include "../internal/remove_auto_incr_primary_t.hpp"
#include "../transpilation/is_vector.hpp"
#include "parsing/Parser.hpp"

namespace sqlgen::duckdb::chunk_writer {

template <class T>
struct contains_vector : transpilation::is_vector<std::remove_cvref_t<T>> {};

template <class T>
struct contains_vector<std::optional<T>> : contains_vector<T> {};

template <class T>
struct contains_vector<std::shared_ptr<T>> : contains_vector<T> {};

template <class T>
struct contains_vector<std::unique_ptr<T>> : contains_vector<T> {};

template <class NamedTuple>
struct named_tuple_contains_vector;

template <class... Fields>
struct named_tuple_contains_vector<rfl::NamedTuple<Fields...>>
    : std::bool_constant<(contains_vector<typename Fields::Type>::value ||
                          ...)> {};

template <class T>
inline constexpr bool has_vector_field_v =
    named_tuple_contains_vector<rfl::named_tuple_t<T>>::value;

/// Stores `_value` in row `_index` of `_vector`. The type-specific logic
/// lives in the root `parsing::Parser<T>`.
template <class T>
Result<Nothing> assign(duckdb_vector _vector, idx_t _index, const T& _value) {
  using Type = std::remove_cvref_t<T>;
  return parsing::Parser<Type>::write_to_vector(_vector, _index, _value);
}

template <class T>
Result<Nothing> assign_row(duckdb_data_chunk _chunk, idx_t _row,
                           const T& _value) {
  using View = internal::remove_auto_incr_primary_t<rfl::view_t<const T>>;
  try {
    idx_t column = 0;
    std::optional<Error> error_value;
    View(rfl::to_view(_value)).apply([&](const auto&... fields) {
      (
          [&] {
            if (error_value) return;
            auto result = assign(duckdb_data_chunk_get_vector(_chunk, column++),
                                 _row, *fields.value());
            if (!result) error_value = result.error();
          }(),
          ...);
    });
    if (error_value) return error(error_value->what());
    return Nothing{};
  } catch (const std::exception& exception) {
    return error(exception.what());
  }
}

}  // namespace sqlgen::duckdb::chunk_writer

#endif

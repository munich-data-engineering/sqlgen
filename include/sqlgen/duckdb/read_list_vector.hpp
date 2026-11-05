#ifndef SQLGEN_DUCKDB_READ_LIST_VECTOR_HPP_
#define SQLGEN_DUCKDB_READ_LIST_VECTOR_HPP_

#include <duckdb.h>

#include <optional>
#include <type_traits>
#include <vector>

#include "../Result.hpp"
#include "../transpilation/is_nullable.hpp"
#include "../transpilation/is_vector.hpp"
#include "check_duckdb_type.hpp"
#include "parsing/Parser.hpp"

namespace sqlgen::duckdb {

/// A `std::optional` wrapping a LIST. Its NULL flag has to be resolved
/// structurally here, because the root `Parser` reads through a pointer,
/// which cannot address a nested LIST value.
template <class T>
struct is_optional_vector : std::false_type {};

template <class T>
struct is_optional_vector<std::optional<T>> : transpilation::is_vector<T> {};

template <class T>
Result<T> read_list_vector_value(duckdb_vector _vector, idx_t _index) {
  const auto validity = duckdb_vector_get_validity(_vector);
  const bool valid =
      !validity || duckdb_validity_row_is_valid(validity, _index);
  if constexpr (is_optional_vector<T>::value) {
    if (!valid) return T{};
    return read_list_vector_value<typename T::value_type>(_vector, _index)
        .transform([](auto&& _value) -> T {
          return T(std::move(_value));
        });
  } else if constexpr (transpilation::is_vector_v<T>) {
    if (!valid) return error("NULL encountered in non-nullable DuckDB LIST.");
    const auto entries =
        static_cast<const duckdb_list_entry*>(duckdb_vector_get_data(_vector));
    const auto entry = entries[_index];
    const auto child = duckdb_list_vector_get_child(_vector);
    T result;
    result.reserve(entry.length);
    for (idx_t i = 0; i < entry.length; ++i) {
      auto value = read_list_vector_value<typename T::value_type>(
          child, entry.offset + i);
      if (!value) return error(value.error().what());
      result.emplace_back(std::move(*value));
    }
    return result;
  } else {
    // Scalar elements (including `std::optional` ones) are read through the
    // root `Parser`, which owns the NULL handling for their type.
    using Stored = typename parsing::Parser<T>::ResultingType;
    const auto data =
        static_cast<const Stored*>(duckdb_vector_get_data(_vector));
    return parsing::Parser<T>::read(valid ? data + _index : nullptr);
  }
}

template <class T>
bool check_list_vector_type(duckdb_vector _vector) {
  if constexpr (transpilation::is_optional<std::remove_cvref_t<T>>::value) {
    return check_list_vector_type<typename T::value_type>(_vector);
  } else {
    auto logical_type = duckdb_vector_get_column_type(_vector);
    const auto type = duckdb_get_type_id(logical_type);
    duckdb_destroy_logical_type(&logical_type);
    if constexpr (transpilation::is_vector_v<T>) {
      return type == DUCKDB_TYPE_LIST &&
             check_list_vector_type<typename T::value_type>(
                 duckdb_list_vector_get_child(_vector));
    } else {
      using Stored = typename parsing::Parser<T>::ResultingType;
      return check_duckdb_type<Stored>(type);
    }
  }
}

}  // namespace sqlgen::duckdb

#endif

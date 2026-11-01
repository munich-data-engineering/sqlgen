#ifndef SQLGEN_DUCKDB_PARSING_PARSER_VECTOR_HPP_
#define SQLGEN_DUCKDB_PARSING_PARSER_VECTOR_HPP_

#include <duckdb.h>

#include <vector>

#include "../../Result.hpp"
#include "Parser_base.hpp"

namespace sqlgen::duckdb::parsing {

template <class T, class Allocator>
struct Parser<std::vector<T, Allocator>> {
  using Type = std::vector<T, Allocator>;
  using ResultingType = Type;

  static Result<Type> read(const Type* _value) noexcept {
    if (!_value) return error("LIST value cannot be NULL.");
    return *_value;
  }

  static Result<Nothing> write_to_vector(duckdb_vector _vector, idx_t _index,
                                         const Type& _value) {
    const auto offset = duckdb_list_vector_get_size(_vector);
    const auto length = static_cast<idx_t>(_value.size());
    if (duckdb_list_vector_reserve(_vector, offset + length) == DuckDBError) {
      return error("Could not reserve DuckDB LIST child vector.");
    }
    auto child = duckdb_list_vector_get_child(_vector);
    for (idx_t i = 0; i < length; ++i) {
      auto result = Parser<T>::write_to_vector(child, offset + i, _value[i]);
      if (!result) return result;
    }
    if (duckdb_list_vector_set_size(_vector, offset + length) == DuckDBError) {
      return error("Could not set DuckDB LIST child vector size.");
    }
    auto entries =
        static_cast<duckdb_list_entry*>(duckdb_vector_get_data(_vector));
    entries[_index] = duckdb_list_entry{.offset = offset, .length = length};
    return Nothing{};
  }
};

}  // namespace sqlgen::duckdb::parsing

#endif

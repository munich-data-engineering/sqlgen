#ifndef SQLGEN_DUCKDB_PARSING_PARSER_FOREIGN_KEY_HPP_
#define SQLGEN_DUCKDB_PARSING_PARSER_FOREIGN_KEY_HPP_

#include <duckdb.h>

#include <rfl.hpp>
#include <type_traits>

#include "../../Result.hpp"
#include "../../transpilation/has_reflection_method.hpp"
#include "Parser_base.hpp"

namespace sqlgen::duckdb::parsing {

template <class T>
  requires transpilation::has_reflection_method<std::remove_cvref_t<T>>
struct Parser<T> {
  using Type = std::remove_cvref_t<T>;
  using ResultingType = typename Parser<
      std::remove_cvref_t<typename Type::ReflectionType>>::ResultingType;

  static Result<T> read(const ResultingType* _r) noexcept {
    return Parser<std::remove_cvref_t<typename Type::ReflectionType>>::read(_r)
        .transform([](auto&& _t) { return T(std::move(_t)); });
  }

  static Result<Nothing> write(const T& _t,
                               duckdb_appender _appender) noexcept {
    return Parser<std::remove_cvref_t<typename Type::ReflectionType>>::write(
        _t.reflection(), _appender);
  }

  static Result<Nothing> write_to_vector(duckdb_vector _vector, idx_t _index,
                                         const T& _value) {
    return Parser<std::remove_cvref_t<typename Type::ReflectionType>>::
        write_to_vector(_vector, _index, _value.reflection());
  }
};

}  // namespace sqlgen::duckdb::parsing

#endif

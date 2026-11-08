#ifndef SQLGEN_TRANSPILATION_GET_TABLENAME_HPP_
#define SQLGEN_TRANSPILATION_GET_TABLENAME_HPP_

#include <rfl/Literal.hpp>
#include <rfl/internal/StringLiteral.hpp>
#include <rfl/type_name_t.hpp>
#include <string>
#include <string_view>
#include <type_traits>

#include "../internal/remove_namespaces.hpp"

namespace sqlgen::transpilation {

template <class T>
consteval std::string_view get_tablename_str_view() noexcept {
  using Type = std::remove_cvref_t<T>;

  constexpr bool has_tablename = requires {
    { Type::tablename } -> std::convertible_to<std::string_view>;
  };

  constexpr bool has_viewname = requires {
    { Type::viewname } -> std::convertible_to<std::string_view>;
  };

  if constexpr (has_tablename) {
    static_assert(
        !has_viewname,
        "A struct can either have a tablename or a viewname, but not both.");
    return Type::tablename;

  } else if constexpr (has_viewname) {
    return Type::viewname;

  } else {
    return internal::remove_namespaces<rfl::type_name_t<Type>>();
  }
}

template <class T>
std::string get_tablename() noexcept {
  return std::string(get_tablename_str_view<T>());
}

}  // namespace sqlgen::transpilation

#endif

#ifndef SQLGEN_INTERNAL_GET_NAME_FROM_LITERAL_HPP_
#define SQLGEN_INTERNAL_GET_NAME_FROM_LITERAL_HPP_

#include <rfl/Literal.hpp>
#include <string_view>

namespace sqlgen::internal {

template <class T>
struct GetNameFromLiteral;

template <rfl::internal::StringLiteral _name>
struct GetNameFromLiteral<rfl::Literal<_name>> {
  static constexpr std::string_view value = _name.string_view();
};

template <class T>
constexpr std::string_view get_name_from_literal() noexcept {
  return GetNameFromLiteral<T>::value;
}

}  // namespace sqlgen::internal

#endif

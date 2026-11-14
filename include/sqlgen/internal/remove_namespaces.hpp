#ifndef SQLGEN_TRANSPILATION_REMOVE_NAMESPACES_HPP_
#define SQLGEN_TRANSPILATION_REMOVE_NAMESPACES_HPP_

#include <rfl/Literal.hpp>
#include <rfl/internal/StringLiteral.hpp>
#include <string_view>

#include "get_name_from_literal.hpp"

namespace sqlgen::internal {

template <class LiteralT>
consteval inline std::string_view remove_namespaces() {
  const auto str = get_name_from_literal<LiteralT>();
  const auto pos = str.find_last_of(':');
  if (pos == std::string_view::npos) {
    return str;
  }
  return str.substr(pos + 1);
}

}  // namespace sqlgen::internal

#endif

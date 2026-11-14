#ifndef SQLGEN_TRANSPILATION_TO_ALIAS_OR_TABLE_HPP_
#define SQLGEN_TRANSPILATION_TO_ALIAS_OR_TABLE_HPP_

#include <optional>
#include <rfl.hpp>
#include <string>
#include <type_traits>

#include "../Literal.hpp"
#include "../Result.hpp"
#include "../dynamic/AliasOrTable.hpp"

namespace sqlgen::transpilation {

template <class T, bool _is_table>
struct ToAliasOrTable;

template <bool _is_table>
struct ToAliasOrTable<Nothing, _is_table> {
  dynamic::AliasOrTable operator()() const {
    return dynamic::alias_or_table::Neither{};
  }
};

template <bool _is_table>
struct ToAliasOrTable<Literal<"">, _is_table> {
  dynamic::AliasOrTable operator()() const {
    return dynamic::alias_or_table::Neither{};
  }
};

template <rfl::internal::StringLiteral _alias, bool _is_table>
struct ToAliasOrTable<Literal<_alias>, _is_table> {
  dynamic::AliasOrTable operator()() const {
    if constexpr (_is_table) {
      return dynamic::alias_or_table::Table{.name = Literal<_alias>().str()};
    } else {
      return dynamic::alias_or_table::Alias{.name = Literal<_alias>().str()};
    }
  }
};

template <class T, bool _is_table>
dynamic::AliasOrTable to_alias_or_table() {
  return ToAliasOrTable<std::remove_cvref_t<T>, _is_table>{}();
}

}  // namespace sqlgen::transpilation

#endif

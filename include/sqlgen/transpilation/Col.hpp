#ifndef SQLGEN_TRANSPILATION_COL_HPP_
#define SQLGEN_TRANSPILATION_COL_HPP_

#include <rfl.hpp>
#include <string>

namespace sqlgen::transpilation {

template <rfl::internal::StringLiteral _name,
          rfl::internal::StringLiteral _alias_or_table, bool _is_table>
struct Col {
  using ColType = Col<_name, _alias_or_table, _is_table>;
  using Name = rfl::Literal<_name>;
  using AliasOrTable = rfl::Literal<_alias_or_table>;

  /// Returns the column alias or table name.
  std::string alias_or_table() const noexcept { return AliasOrTable().str(); }

  /// Returns the column name.
  std::string name() const noexcept { return Name().str(); }

  /// Returns whether alias_or_table is a table name.
  static constexpr bool is_table() noexcept { return _is_table; }

  /// Returns the table alias.
};

}  // namespace sqlgen::transpilation

#endif

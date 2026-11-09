#ifndef SQLGEN_DYNAMIC_ALIASORTABLE_HPP_
#define SQLGEN_DYNAMIC_ALIASORTABLE_HPP_

#include <optional>
#include <rfl/Variant.hpp>
#include <string>

namespace sqlgen::dynamic {

namespace alias_or_table {
struct Neither {
  std::nullopt_t name = std::nullopt;
};

struct Alias {
  std::string name;
};

struct Table {
  std::string name;
};

}  // namespace alias_or_table

using AliasOrTable = rfl::Variant<alias_or_table::Neither,
                                  alias_or_table::Alias, alias_or_table::Table>;

}  // namespace sqlgen::dynamic

#endif

#ifndef SQLGEN_DYNAMIC_COLUMN_HPP_
#define SQLGEN_DYNAMIC_COLUMN_HPP_

#include <optional>
#include <string>

#include "AliasOrTable.hpp"
#include "Type.hpp"
#include "types.hpp"

namespace sqlgen::dynamic {

struct Column {
  AliasOrTable alias_or_table;
  std::string name;
  Type type = types::Unknown{};
};

}  // namespace sqlgen::dynamic

#endif

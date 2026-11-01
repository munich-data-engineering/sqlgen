#ifndef SQLGEN_DYNAMIC_COLUMN_HPP_
#define SQLGEN_DYNAMIC_COLUMN_HPP_

#include <cstddef>
#include <optional>
#include <string>

#include "Type.hpp"
#include "types.hpp"

namespace sqlgen::dynamic {

struct Column {
  std::optional<std::string> alias;
  std::string name;
  Type type = types::Unknown{};
  size_t collection_depth = 0;
};

}  // namespace sqlgen::dynamic

#endif

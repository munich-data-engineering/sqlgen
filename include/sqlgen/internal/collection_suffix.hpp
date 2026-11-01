#ifndef SQLGEN_INTERNAL_COLLECTION_SUFFIX_HPP_
#define SQLGEN_INTERNAL_COLLECTION_SUFFIX_HPP_

#include <cstddef>
#include <string>

namespace sqlgen::internal {

inline std::string collection_suffix(const size_t _depth) {
  std::string suffix;
  suffix.reserve(_depth * 2);
  for (size_t i = 0; i < _depth; ++i) suffix += "[]";
  return suffix;
}

}  // namespace sqlgen::internal

#endif

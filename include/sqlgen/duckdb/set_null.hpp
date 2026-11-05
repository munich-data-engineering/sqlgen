#ifndef SQLGEN_DUCKDB_SET_NULL_HPP_
#define SQLGEN_DUCKDB_SET_NULL_HPP_

#include <duckdb.h>

namespace sqlgen::duckdb {

/// Marks the row `_index` of `_vector` as NULL.
inline void set_null(duckdb_vector _vector, idx_t _index) {
  duckdb_vector_ensure_validity_writable(_vector);
  duckdb_validity_set_row_invalid(duckdb_vector_get_validity(_vector), _index);
}

}  // namespace sqlgen::duckdb

#endif

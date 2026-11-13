#ifndef SQLGEN_INTERNAL_SUPPORTS_COLLECTIONS_HPP_
#define SQLGEN_INTERNAL_SUPPORTS_COLLECTIONS_HPP_

#include <type_traits>

#include "../Session.hpp"
#include "../Transaction.hpp"
#include "../transpilation/make_columns.hpp"

namespace sqlgen::internal {

template <class Connection>
struct supports_collections : std::false_type {};

template <class Connection>
struct supports_collections<Session<Connection>>
    : supports_collections<Connection> {};

template <class Connection>
struct supports_collections<Transaction<Connection>>
    : supports_collections<Connection> {};

template <class Connection, class T>
consteval void assert_collections_supported() {
  static_assert(
      supports_collections<std::remove_cvref_t<Connection>>::value ||
          !transpilation::has_collection_field<std::remove_cvref_t<T>>::value,
      "std::vector fields are not supported by this database.");
}

}  // namespace sqlgen::internal

#endif

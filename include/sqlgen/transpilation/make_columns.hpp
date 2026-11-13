#ifndef SQLGEN_TRANSPILATION_MAKE_COLUMNS_HPP_
#define SQLGEN_TRANSPILATION_MAKE_COLUMNS_HPP_

#include <rfl.hpp>
#include <string>
#include <type_traits>

#include "../dynamic/Column.hpp"
#include "../dynamic/Type.hpp"
#include "../dynamic/types.hpp"
#include "../parsing/Parser.hpp"
#include "has_reflection_method.hpp"
#include "is_nullable.hpp"
#include "is_primary_key.hpp"

namespace sqlgen::transpilation {

template <class T>
struct collection_traits {
  using value_type = T;
  static constexpr size_t depth = 0;
};

template <class T, class Allocator>
struct collection_traits<std::vector<T, Allocator>> {
  using nested = collection_traits<T>;
  using value_type = typename nested::value_type;
  static constexpr size_t depth = nested::depth + 1;
};

// Element nullability does not make the collection column itself nullable.
template <class T, class VectorAllocator>
struct collection_traits<std::vector<std::optional<T>, VectorAllocator>> {
  using nested = collection_traits<T>;
  using value_type = typename nested::value_type;
  static constexpr size_t depth = nested::depth + 1;
};

template <class T>
struct collection_traits<std::optional<T>> {
  using nested = collection_traits<T>;
  using value_type = std::optional<typename nested::value_type>;
  static constexpr size_t depth = nested::depth;
};

/// Whether `T` has at least one field that is wrapped in a collection
/// (e.g. `std::vector`) and therefore needs a collection column type.
/// Only the reflectable aggregates that `to_create_table(...)` and friends
/// accept are inspected; anything else cannot declare a collection field.
template <class T, class = void>
struct has_collection_field : std::false_type {};

template <class T>
struct has_collection_field<
    T, std::enable_if_t<std::is_class_v<std::remove_cvref_t<T>> &&
                        std::is_aggregate_v<std::remove_cvref_t<T>>>> {
 private:
  using NamedTupleType = rfl::named_tuple_t<std::remove_cvref_t<T>>;
  using Fields = typename NamedTupleType::Fields;

  template <int... _is>
  static constexpr bool check(std::integer_sequence<int, _is...>) {
    return ((collection_traits<std::remove_cvref_t<
                 typename rfl::tuple_element_t<_is, Fields>::Type>>::depth >
             0) ||
            ...);
  }

 public:
  static constexpr bool value =
      check(std::make_integer_sequence<int, rfl::tuple_size_v<Fields>>());
};

template <class Name>
std::string to_colname() {
  return Name().str();
}

template <class Type>
dynamic::Type to_type() {
  using T = std::remove_cvref_t<Type>;
  return parsing::Parser<T>::to_type();
}

template <class FieldType>
dynamic::Column to_column() {
  using Type = std::remove_cvref_t<typename FieldType::Type>;
  using Traits = collection_traits<Type>;
  return dynamic::Column{.name = to_colname<typename FieldType::Name>(),
                         .type = to_type<typename Traits::value_type>(),
                         .collection_depth = Traits::depth};
}

template <class Fields, int... _is>
std::vector<dynamic::Column> make_columns(std::integer_sequence<int, _is...>) {
  return std::vector<dynamic::Column>(
      {to_column<rfl::tuple_element_t<_is, Fields>>()...});
}

}  // namespace sqlgen::transpilation

#endif

#ifndef SQLGEN_TRANSPILATION_WRAPINOPTIONALT_HPP_
#define SQLGEN_TRANSPILATION_WRAPINOPTIONALT_HPP_

#include <optional>
#include <rfl.hpp>
#include <rfl/internal/StringLiteral.hpp>
#include <type_traits>

#include "get_tablename.hpp"
#include "is_nullable.hpp"

namespace sqlgen::transpilation {

template <class T, class U>
struct WrapInOptional;

template <class T, class... FieldTypes>
struct WrapInOptional<T, rfl::NamedTuple<FieldTypes...>> {
  static constexpr std::string_view tablename = get_tablename_str_view<T>();
  using NamedTupleType = rfl::NamedTuple<rfl::Field<
      FieldTypes::name_,
      std::conditional_t<is_nullable_v<typename FieldTypes::Type>,
                         typename FieldTypes::Type,
                         std::optional<typename FieldTypes::Type>>>...>;
};

template <class T>
using wrap_in_optional_t = WrapInOptional<T, rfl::named_tuple_t<T>>;

}  // namespace sqlgen::transpilation

#endif

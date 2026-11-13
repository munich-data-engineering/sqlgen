#ifndef SQLGEN_TRANSPILATION_GETTABLET_HPP_
#define SQLGEN_TRANSPILATION_GETTABLET_HPP_

#include <rfl.hpp>
#include <tuple>
#include <type_traits>
#include <utility>

#include "../Literal.hpp"
#include "../Result.hpp"
#include "../dynamic/JoinType.hpp"
#include "../internal/get_name_from_literal.hpp"
#include "TableWrapper.hpp"
#include "get_tablename.hpp"
#include "wrap_in_optional_t.hpp"

namespace sqlgen::transpilation {

template <class T>
struct RemoveWrapInOptional {
  using Type = T;
};

template <class T, class U>
struct RemoveWrapInOptional<WrapInOptional<T, U>> {
  using Type = typename WrapInOptional<T, U>::NamedTupleType;
};

template <class T>
using remove_wrap_in_optional_t = typename RemoveWrapInOptional<T>::Type;

template <class AliasOrTable, class _TableType, class _AliasOrTableType,
          bool _is_table>
struct PairWrapper {
  using TableType = remove_wrap_in_optional_t<_TableType>;
  using AliasOrTableType = _AliasOrTableType;

  template <class OtherTableType, class OtherAliasOrTableType>
  friend consteval auto operator||(
      const PairWrapper&,
      const PairWrapper<AliasOrTable, OtherTableType, OtherAliasOrTableType,
                        _is_table>&) noexcept {
    if constexpr (!_is_table) {
      if constexpr (std::is_same_v<AliasOrTable, std::remove_cvref_t<
                                                     OtherAliasOrTableType>>) {
        return PairWrapper<AliasOrTable, OtherTableType, OtherAliasOrTableType,
                           _is_table>{};
      } else {
        return PairWrapper{};
      }

    } else if constexpr (_is_table) {
      if constexpr (internal::get_name_from_literal<AliasOrTable>() ==
                    get_tablename_str_view<OtherTableType>()) {
        static_assert(
            std::is_same_v<OtherAliasOrTableType, Literal<"">> ||
                std::is_same_v<OtherAliasOrTableType, Nothing>,
            "When you reference the column via the table name (in "
            "other words, via "
            "member pointers), you cannot set an alias for the table.");
        return PairWrapper<AliasOrTable, OtherTableType, OtherAliasOrTableType,
                           _is_table>{};
      } else {
        return PairWrapper{};
      }
    }
  }
};

template <class AliasOrTable, class T, bool _is_table>
struct GetTableType;

template <rfl::internal::StringLiteral _table_or_alias, class... TableTypes,
          class... AliasOrTableTypes, bool _is_table>
struct GetTableType<Literal<_table_or_alias>,
                    rfl::Tuple<std::pair<TableTypes, AliasOrTableTypes>...>,
                    _is_table> {
  static constexpr auto wrapper =
      (PairWrapper<Literal<_table_or_alias>, Nothing, Nothing, _is_table>{} ||
       ... ||
       PairWrapper<Literal<_table_or_alias>, TableTypes, AliasOrTableTypes,
                   _is_table>{});

  using TableType = std::remove_cvref_t<typename decltype(wrapper)::TableType>;

  static_assert(!std::is_same_v<TableType, Nothing>,
                "Alias could not be identified.");
};

template <rfl::internal::StringLiteral _table_or_alias, class T, bool _is_table>
struct GetTableType<Literal<_table_or_alias>, TableWrapper<T>, _is_table> {
  using TableType = T;
};

template <rfl::internal::StringLiteral _table_or_alias, class T, bool _is_table>
struct GetTableType<Literal<_table_or_alias>, T, _is_table> {
  using TableType = T;
};

template <size_t _i, class... TableTypes, class... AliasOrTableTypes,
          bool _is_table>
struct GetTableType<std::integral_constant<size_t, _i>,
                    rfl::Tuple<std::pair<TableTypes, AliasOrTableTypes>...>,
                    _is_table> {
  using TableType = typename rfl::tuple_element_t<
      _i, rfl::Tuple<std::pair<TableTypes, AliasOrTableTypes>...>>::first_type;

  static_assert(!std::is_same_v<TableType, Nothing>,
                "Alias could not be identified.");
};

template <size_t _i, class T, bool _is_table>
struct GetTableType<std::integral_constant<size_t, _i>, TableWrapper<T>,
                    _is_table> {
  using TableType = T;
};

template <size_t _i, class T, bool _is_table>
struct GetTableType<std::integral_constant<size_t, _i>, T, _is_table> {
  using TableType = T;
};

template <class AliasOrTable, class T, bool _is_table>
using get_table_t =
    typename GetTableType<std::remove_cvref_t<AliasOrTable>,
                          std::remove_cvref_t<T>, _is_table>::TableType;

}  // namespace sqlgen::transpilation

#endif

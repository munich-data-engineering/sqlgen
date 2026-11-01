#ifndef SQLGEN_TRANSPILATION_IS_VECTOR_HPP_
#define SQLGEN_TRANSPILATION_IS_VECTOR_HPP_

#include <type_traits>
#include <vector>

namespace sqlgen::transpilation {

template <class T>
class is_vector : public std::false_type {};

template <class T, class Allocator>
class is_vector<std::vector<T, Allocator>> : public std::true_type {};

template <class T>
constexpr bool is_vector_v = is_vector<std::remove_cvref_t<T>>::value;

}  // namespace sqlgen::transpilation

#endif

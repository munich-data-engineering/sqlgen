#ifndef SQLGEN_PARSING_PARSER_VECTOR_HPP_
#define SQLGEN_PARSING_PARSER_VECTOR_HPP_

#include <cctype>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "../Result.hpp"
#include "../dynamic/Type.hpp"
#include "../transpilation/is_vector.hpp"
#include "Parser_base.hpp"

namespace sqlgen::parsing {

namespace vector_parser {

inline Result<std::vector<std::optional<std::string>>> split(
    std::string_view _input) noexcept {
  if (const auto equals = _input.find('='); equals != std::string_view::npos &&
                                            !_input.empty() &&
                                            _input.front() == '[') {
    _input.remove_prefix(equals + 1);
  }
  if (_input.size() < 2 || _input.front() != '{' || _input.back() != '}') {
    return error("Invalid PostgreSQL array value.");
  }
  _input.remove_prefix(1);
  _input.remove_suffix(1);
  std::vector<std::optional<std::string>> result;
  size_t start = 0;
  size_t depth = 0;
  bool quoted = false;
  bool escaped = false;
  for (size_t i = 0; i <= _input.size(); ++i) {
    const bool at_end = i == _input.size();
    const char c = at_end ? ',' : _input[i];
    if (!at_end && escaped) {
      escaped = false;
      continue;
    }
    if (!at_end && c == '\\') {
      escaped = true;
      continue;
    }
    if (!at_end && c == '"') {
      quoted = !quoted;
      continue;
    }
    if (!quoted && !at_end && c == '{') ++depth;
    if (!quoted && !at_end && c == '}') {
      if (depth == 0) return error("Invalid PostgreSQL array dimensions.");
      --depth;
    }
    if (!quoted && depth == 0 && c == ',') {
      auto token = _input.substr(start, i - start);
      if (token.empty() && _input.empty()) return result;
      if (token == "NULL") {
        result.emplace_back(std::nullopt);
      } else {
        std::string value;
        size_t begin = 0;
        size_t end = token.size();
        if (end >= 2 && token.front() == '"' && token.back() == '"') {
          begin = 1;
          --end;
        }
        value.reserve(end - begin);
        bool token_escaped = false;
        for (size_t j = begin; j < end; ++j) {
          if (token_escaped) {
            value += token[j];
            token_escaped = false;
          } else if (token[j] == '\\') {
            token_escaped = true;
          } else {
            value += token[j];
          }
        }
        if (token_escaped) return error("Invalid escape in PostgreSQL array.");
        result.emplace_back(std::move(value));
      }
      start = i + 1;
    }
  }
  if (quoted || escaped || depth != 0)
    return error("Invalid PostgreSQL array value.");
  return result;
}

inline std::string quote(std::string_view _value) {
  std::string result = "\"";
  result.reserve(_value.size() + 2);
  for (const char c : _value) {
    if (c == '"' || c == '\\') result += '\\';
    result += c;
  }
  result += '"';
  return result;
}

}  // namespace vector_parser

template <class T, class Allocator>
struct Parser<std::vector<T, Allocator>> {
  using Vector = std::vector<T, Allocator>;

  static Result<Vector> read(const std::optional<std::string>& _str) noexcept {
    if (!_str) return error("NULL value encountered: array cannot be NULL.");
    return vector_parser::split(*_str).and_then(
        [](auto&& _tokens) -> Result<Vector> {
          Vector result;
          result.reserve(_tokens.size());
          for (const auto& token : _tokens) {
            auto value = Parser<T>::read(token);
            if (!value) return error(value.error().what());
            result.emplace_back(std::move(*value));
          }
          return result;
        });
  }

  static std::optional<std::string> write(const Vector& _values) noexcept {
    std::string result = "{";
    for (size_t i = 0; i < _values.size(); ++i) {
      if (i != 0) result += ',';
      const T value = _values[i];
      const auto encoded = Parser<T>::write(value);
      if (!encoded) {
        result += "NULL";
      } else if constexpr (transpilation::is_vector_v<T>) {
        result += *encoded;
      } else {
        result += vector_parser::quote(*encoded);
      }
    }
    result += '}';
    return result;
  }

  static dynamic::Type to_type() noexcept { return Parser<T>::to_type(); }
};

}  // namespace sqlgen::parsing

#endif

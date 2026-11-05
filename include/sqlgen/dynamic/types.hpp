#ifndef SQLGEN_DYNAMIC_TABLE_TYPES_HPP_
#define SQLGEN_DYNAMIC_TABLE_TYPES_HPP_

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace sqlgen::dynamic::types {

struct ForeignKeyReference {
  std::string table;
  std::string column;
};

struct Properties {
  bool auto_incr = false;
  bool primary = false;
  bool nullable = false;
  bool unique = false;
  std::optional<ForeignKeyReference> foreign_key_reference = std::nullopt;
};

// To be used as the default value.
struct Unknown {
  Properties properties = Properties{};
};

struct Boolean {
  Properties properties = Properties{};
};

struct Dynamic {
  std::string type_name;
  Properties properties = Properties{};
};

struct Float32 {
  Properties properties = Properties{};
};

struct Float64 {
  Properties properties = Properties{};
};

struct Int8 {
  Properties properties = Properties{};
};

struct Int16 {
  Properties properties = Properties{};
};

struct Int32 {
  Properties properties = Properties{};
};

struct Int64 {
  Properties properties = Properties{};
};

struct JSON {
  Properties properties = Properties{};
};

struct UInt8 {
  Properties properties = Properties{};
};

struct UInt16 {
  Properties properties = Properties{};
};

struct UInt32 {
  Properties properties = Properties{};
};

struct UInt64 {
  Properties properties = Properties{};
};

struct Enum {
  std::string name;
  std::vector<std::string> values;
  Properties properties = Properties{};
};

struct Text {
  Properties properties = Properties{};
};

struct Date {
  std::string tz;
  Properties properties = Properties{};
};

struct Timestamp {
  std::string tz;
  Properties properties = Properties{};
};

struct TimestampWithTZ {
  std::string tz;
  Properties properties = Properties{};
};

struct VarChar {
  uint16_t length;
  Properties properties = Properties{};
};

}  // namespace sqlgen::dynamic::types

#endif

# `sqlgen::JSON`

`sqlgen::JSON<T>` stores structured JSON data in your database while preserving full C++ type-safety. It integrates with reflectcpp (`rfl`) so you can read and write arbitrary structured data without manual (de)serialization.

Critically, `rfl::JSON` is fully serializable with reflectcpp and supports any data type also supported by reflectcpp (including nested structs, `std::optional`, containers like `std::vector`, and more), enabling seamless end-to-end integration.

## Usage

### Basic Definition

Define a JSON field in your struct by wrapping the underlying C++ type with `sqlgen::JSON`:

```cpp
struct Person {
    std::string first_name;
    std::string last_name;
    int age;
    sqlgen::JSON<std::optional<std::vector<Person>>> children;  // Nested, optional JSON
};
```

This generates a table with a JSON-compatible column type (dialect-specific):

```sql
-- PostgreSQL
CREATE TABLE IF NOT EXISTS "Person"(
    "first_name" TEXT NOT NULL,
    "last_name"  TEXT NOT NULL,
    "age"        INTEGER NOT NULL,
    "children"   JSONB
);

-- MySQL / MariaDB
CREATE TABLE IF NOT EXISTS `Person`(
    `first_name` TEXT NOT NULL,
    `last_name`  TEXT NOT NULL,
    `age`        INT NOT NULL,
    `children`   JSON
);

-- SQLite 
CREATE TABLE IF NOT EXISTS "Person"(
    "first_name" TEXT NOT NULL,
    "last_name"  TEXT NOT NULL,
    "age"        INTEGER NOT NULL,
    "children"   JSONB
);
```

### Construction and Assignment

Assign any reflectcpp-supported value to the JSON field. For example, a nested vector of the same type:

```cpp
const auto children = std::vector<Person>({
    Person{.first_name = "Bart", .last_name = "Simpson", .age = 10},
    Person{.first_name = "Lisa", .last_name = "Simpson", .age = 8},
    Person{.first_name = "Maggie", .last_name = "Simpson", .age = 0}
});

const auto homer = Person{
    .first_name = "Homer",
    .last_name  = "Simpson",
    .age        = 45,
    .children   = children  // Automatically serialized to JSON
};
```

You can store any `T` that reflectcpp can serialize, such as:
- `std::optional<T>`
- `std::vector<T>`, `std::map<K,V>`, and other standard containers
- Nested `struct` types reflected with reflectcpp

### Reading and Writing

Use the regular `sqlgen::write` and `sqlgen::read` APIs. JSON values are transparently serialized/deserialized.

{{< callout type="info" >}}
The example below uses the optional `sqlgen::postgres` backend, which is disabled by default. Enable it by configuring CMake with `-DSQLGEN_POSTGRES=ON` before building.
{{< /callout >}}

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

// PostgreSQL example (analogous for MySQL/SQLite)
const auto credentials = sqlgen::postgres::Credentials{
    .user = "postgres", .password = "password", .host = "localhost", .dbname = "postgres"};

const auto people = sqlgen::postgres::connect(credentials)
    .and_then(drop<Person> | if_exists)
    .and_then(write(std::ref(homer)))
    .and_then(sqlgen::read<std::vector<Person>>)
    .value();
```

### Accessing Values

Access the underlying C++ value via familiar methods:

```cpp
// Underlying value access
people[0].children();
people[0].children.get();
people[0].children.value();

// Serialize entire result to JSON with reflectcpp
const std::string json = rfl::json::write(people);
```

### Template Parameters

The `sqlgen::JSON` template takes one parameter:

1. T: Any reflectcpp-supported C++ type to store as JSON

```cpp
sqlgen::JSON<T> field_name;
```

### Database Integration

`sqlgen::JSON<T>` selects the appropriate JSON-capable storage per dialect:
- **PostgreSQL**: `JSONB`
- **MySQL/MariaDB**: `JSON`
- **SQLite**: `JSONB` 

All (de)serialization is handled by reflectcpp (`rfl`) underneath, ensuring type-safe transformations without manual glue code.

## Notes

- Works with any reflectcpp-serializable type (`rfl::JSON` support), including deeply nested structures
- Integrates with `sqlgen::read` and `sqlgen::write` like any other field
- Database JSON type is chosen automatically per dialect
- Supports move and copy semantics
- Provides multiple access methods for the underlying value
- Ideal for flexible, schema-evolving nested attributes

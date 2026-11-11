# `sqlgen::Pattern`

The `sqlgen::Pattern` class provides a type-safe way to validate string fields against regular expressions in C++. It's particularly useful for preventing SQL injection by ensuring data conforms to expected patterns before it reaches the database.

Note that sqlgen already prevents some forms of SQL injection by using prepared statements or the copy operator for inserting data.

## Usage

### Basic Definition

Define a pattern using a regular expression and a descriptive name:

```cpp
// Define a pattern for usernames (alphanumeric with underscores)
using Username = sqlgen::Pattern<"[a-zA-Z0-9_]+", "username">;

struct User {
    sqlgen::PrimaryKey<uint32_t> id;
    Username username;
    sqlgen::Email email;
};
```

### Built-in Patterns

sqlgen provides several commonly used patterns out of the box:

```cpp
// Alphanumeric strings
sqlgen::AlphaNumeric name;

// Base64 encoded strings
sqlgen::Base64Encoded encoded_data;

// Email addresses
sqlgen::Email email;

// UUID formats
sqlgen::UUIDv1 uuid1;
sqlgen::UUIDv2 uuid2;
sqlgen::UUIDv3 uuid3;
sqlgen::UUIDv4 uuid4;
```

### Assignment and Validation

Assign values to pattern-validated fields:

```cpp
struct Person {
    sqlgen::PrimaryKey<uint32_t> id;
    sqlgen::AlphaNumeric first_name;
    sqlgen::AlphaNumeric last_name;
    int age;
};

const auto person = Person{
    .id = 1,
    .first_name = "Homer",  // Valid: contains only alphanumeric characters
    .last_name = "Simpson", // Valid: contains only alphanumeric characters
    .age = 45
};
```

For exception-free construction, you can use `.from_value(...)`:

```cpp
// sqlgen::Result<sqlgen::AlphaNumeric>
const auto res = sqlgen::AlphaNumeric::from_value("Homer");
```

### Accessing Values

Access the underlying string value:

```cpp
const auto person = Person{
    .first_name = "Homer",
    .last_name = "Simpson"
};

// Get the value
const std::string& value1 = person.first_name();
const std::string& value2 = person.first_name.get();
const std::string& value3 = person.first_name.value();
```

## SQL Injection Prevention

Pattern validation is a crucial security feature that helps prevent SQL injection attacks by:

1. Enforcing strict input validation before data reaches the database
2. Ensuring data conforms to expected formats
3. Rejecting potentially malicious input that doesn't match the pattern

###  Example

Consider this example that demonstrates safe query filtering:

```cpp
struct Person {
    sqlgen::PrimaryKey<uint32_t> id;
    sqlgen::AlphaNumeric first_name;
    sqlgen::AlphaNumeric last_name;
    int age;
};

// Safe query function using AlphaNumeric for filtering
std::vector<Person> get_people(const auto& conn, 
                               const sqlgen::AlphaNumeric& first_name) {
    using namespace sqlgen;
    using namespace sqlgen::literals;
    const auto query = sqlgen::read<std::vector<Person>> |
                        where(col<&Person::first_name> == first_name);
    return query(conn).value();
}
```

Without `AlphaNumeric` validation, this code would be vulnerable to SQL injection during query filtering:

```cpp
// Malicious query parameter that would be rejected by AlphaNumeric
get_people(conn, "Homer' OR '1'='1");  // Attempt to bypass filtering
```

The `AlphaNumeric` pattern prevents these attacks by:
- Only allowing alphanumeric characters
- Rejecting special characters like `'`, ` `, `=` that are commonly used in SQL injection
- Validating the input before it reaches the database layer

The test demonstrates safe usage with valid data:
```cpp
// Safe query filtering
const auto people = get_people(conn, "Homer");
```

This example shows how `AlphaNumeric` provides an additional layer of security for query parameters.

## Notes

- Pattern validation occurs at runtime when values are assigned
- The template parameters are:
  - `_regex`: The regular expression pattern to match against
  - `_name`: A descriptive name for the pattern (used in error messages)
- The class supports:
  - Direct string assignment with validation
  - Multiple access methods for the underlying value
  - Reflection for SQL operations
  - Move and copy semantics
- Built-in patterns provide common validation rules
- Custom patterns can be defined for specific use cases
- Pattern validation is an important part of a defense-in-depth security strategy

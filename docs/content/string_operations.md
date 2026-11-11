# String Operations

The `sqlgen` library provides a set of string functions for use in queries. These functions are type-safe and map to the appropriate SQL operations for the target database.

## Motivating Example
Suppose you want to create a report of people with their full names in uppercase, and also want to trim any whitespace from their last names:

```cpp
struct PersonReport {
  std::string full_name;
  std::string last_name_trimmed;
};

const auto get_reports =
    select_from<Person>(
        concat(upper("last_name"_c), ", ", "first_name"_c).as<"full_name">(),
        trim("last_name"_c).as<"last_name_trimmed">()
    )
    | to<std::vector<PersonReport>>;
```

This query produces a list of people with their full names in the format "LASTNAME, Firstname" (with the last name in uppercase) and ensures the last name has no leading or trailing whitespace.

## Using member pointers

Every column literal above (`"first_name"_c`, `"last_name"_c`, …) can be replaced
with a member pointer to the corresponding struct field through the `col`
template, e.g. `col<&Person::first_name>`. The motivating example rewritten with
member pointers:

```cpp
const auto get_reports =
    select_from<Person>(
        concat(upper(col<&Person::last_name>), ", ", col<&Person::first_name>).as<"full_name">(),
        trim(col<&Person::last_name>).as<"last_name_trimmed">()
    )
    | to<std::vector<PersonReport>>;
```

## String Functions

### `length`
Returns the length of a string.

```cpp
length(trim("first_name"_c)) | as<"length_first_name">
```

### `lower` / `upper`
Converts a string to lowercase or uppercase.

```cpp
lower("first_name"_c) | as<"first_name_lower">
upper("first_name"_c) | as<"first_name_upper">
```

### `ltrim`, `rtrim`, `trim`
Removes whitespace (or a specified character) from the left, right, or both sides of a string.

```cpp
ltrim("first_name"_c) | as<"ltrimmed_name">
rtrim("last_name"_c) | as<"rtrimmed_name">
trim("nickname"_c) | as<"trimmed_nickname">
// With custom characters:
ltrim("field"_c, "_ ") | as<"ltrimmed_field">
```

### `replace`
Replaces all occurrences of a substring with another substring.

```cpp
replace("first_name"_c, "Bart", "Hugo") | as<"first_name_replaced">
```

### `concat`
Concatenates multiple strings or expressions.

```cpp
concat("first_name"_c, " ", "last_name"_c) | as<"full_name">
concat(upper("last_name"_c), ", ", "first_name"_c) | as<"full_name">
``` 
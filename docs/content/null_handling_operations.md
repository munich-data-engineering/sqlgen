# Null Handling Operations

The `sqlgen` library provides functions for handling null values in a type-safe way. These functions allow you to work with nullable columns and propagate nullability correctly in your queries.

## Null Handling

### `coalesce`
Returns the first non-null value in the argument list.

```cpp
coalesce("last_name"_c, "none") | as<"last_name_or_none">
coalesce(upper("last_name"_c), "none") | as<"last_name_or_none">
```

Each of these column literals can instead be a member pointer to the struct
field through the `col` template, e.g. `col<&Person::last_name>`:

```cpp
coalesce(col<&Person::last_name>, "none") | as<"last_name_or_none">
coalesce(upper(col<&Person::last_name>), "none") | as<"last_name_or_none">
```

---

## Nullable Values

When using these operations on nullable columns (e.g., `std::optional<T>`), the result will also be nullable if any operand is nullable. For example, adding two `std::optional<int>` columns will yield a `std::optional<int>`. The `coalesce` function is especially useful for providing default values for nullable columns.

---

## Nullability Propagation and `coalesce` Semantics

### General Nullability Rules

- **Unary operations** (e.g., `abs`, `upper`, `sqrt`):
  - If the operand is nullable (`std::optional<T>`), the result is also nullable.
  - If the operand is not nullable, the result is not nullable.
- **Binary or ternary operations** (e.g., `+`, `concat`, `replace`, etc.):
  - If *any* operand is nullable, the result is nullable (`std::optional<ResultType>`).
  - If *all* operands are non-nullable, the result is non-nullable.
- **Type conversion (`cast`)**:
  - If the source is nullable, the result is nullable of the target type.
  - If the source is not nullable, the result is not nullable.
- **String operations** (e.g., `concat`, `replace`, `ltrim`, `rtrim`, `trim`):
  - If any input is nullable, the result is nullable.
  - All string operands must have the same underlying type (checked at compile time).

### `coalesce` Nullability Semantics

The `coalesce` function returns the first non-null value from its arguments. Its nullability is determined as follows:

- If **all** arguments are nullable, the result is nullable (`std::optional<T>`).
- If **any** argument is non-nullable, the result is non-nullable (`T`).
- All arguments must have the same underlying type (ignoring nullability), enforced at compile time.

#### Examples

```cpp
// All arguments nullable: result is nullable
coalesce(std::optional<int>{}, std::optional<int>{}) // -> std::optional<int>

// At least one argument non-nullable: result is non-nullable
coalesce(std::optional<int>{}, 42) // -> int
coalesce(42, std::optional<int>{}) // -> int

// All arguments non-nullable: result is non-nullable
coalesce(1, 2) // -> int

// Mixed string example
coalesce(std::optional<std::string>{}, "default") // -> std::string

// Compile-time error: mismatched types
// coalesce(std::optional<int>{}, std::optional<double>{}) // Error
```

#### Practical Usage

```cpp
// Provide a default for a nullable column
coalesce("last_name"_c, "none") | as<"last_name_or_none"> // Result is std::string
coalesce("middle_name"_c, "nickname"_c) | as<"any_name"> 
```

### Advanced: How sqlgen Enforces Nullability

The nullability rules are enforced at compile time using template metaprogramming (see `underlying_t.hpp`). This ensures that:
- You cannot accidentally assign a nullable result to a non-nullable field.
- All arguments to `coalesce` must have the same base type (e.g., all `int` or all `std::string`).
- The result type of any operation is always correct and safe to use in your result structs. 
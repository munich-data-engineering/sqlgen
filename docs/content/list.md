# `std::vector`

`std::vector<T>` fields map directly to native collection types in supported databases.

Currently supported for DuckDB and PostgreSQL. On other databases, `std::vector` fields fail with a compile-time error.

## Usage

### Basic Definition

Define a list field directly as `std::vector<T>`:

```cpp
struct ListTable {
    int32_t id;
    std::vector<int32_t> numbers;
    std::vector<std::string> labels;
};
```

This generates the following SQL schema:

```sql
-- DuckDB/PostgreSQL
CREATE TABLE IF NOT EXISTS "ListTable"("id" INTEGER NOT NULL, "numbers" INTEGER[] NOT NULL, "labels" TEXT[] NOT NULL);
```

### Nested Lists

Nested vectors are supported to arbitrary depth, with one `[]` suffix added per nesting level:

```cpp
struct Matrix {
    int32_t id;
    std::vector<std::vector<int32_t>> nested;
};
```

```sql
-- DuckDB/PostgreSQL
CREATE TABLE IF NOT EXISTS "Matrix"("id" INTEGER NOT NULL, "nested" INTEGER[][] NOT NULL);
```

### Nullable Lists and Nullable Elements

Nullability is controlled the same way as for scalar fields:

- `std::optional<std::vector<T>>`: the entire list may be `NULL`
- `std::vector<std::optional<T>>`: the list is `NOT NULL`, but individual elements may be `NULL`

```cpp
struct NullableLists {
    int32_t id;
    std::optional<std::vector<int32_t>> nullable_list;
    std::vector<std::optional<int32_t>> nullable_elements;
};
```

```sql
-- DuckDB / PostgreSQL
CREATE TABLE IF NOT EXISTS "NullableLists"("id" INTEGER NOT NULL, "nullable_list" INTEGER[], "nullable_elements" INTEGER[] NOT NULL);
```

### Reading and Writing

Use the regular `sqlgen::write` and `sqlgen::read` APIs.

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

struct Person {
    int32_t id;
    std::string first_name;
    std::vector<std::string> children;
};

const auto people = std::vector<Person>({
    Person{.id = 1, .first_name = "Homer", .children = {"Bart", "Lisa", "Maggie"}},
    Person{.id = 2, .first_name = "Bart", .children = {}}
});

// DuckDB example (analogous for PostgreSQL)
const std::vector<Person> result = sqlgen::duckdb::connect()
    .and_then(create_table<Person>)
    .and_then(write(std::ref(people)))
    .and_then(sqlgen::read<std::vector<Person>>)
    .value();

// result is identical to people
```

Empty lists and missing values round-trip correctly.

### Database Integration

`std::vector<T>` maps to the dialect's native collection type:

- **DuckDB**: `LIST` (variable-length, no shape restrictions)
- **PostgreSQL**: `ARRAY` (governed by a strict structural rule)

**Important**: PostgreSQL arrays must be rectangular, so every sub-array within a dimension must have the same length: `{{1, 2}, {3, 4}}` is valid, while `{{1, 2}, {3}}` is not ([PostgreSQL docs](https://www.postgresql.org/docs/current/arrays.html#ARRAYS-INPUT)). sqlgen cannot check this at compile-time, so a non-rectangular value only surfaces as an error when the database rejects it on write.

## Notes

- Only supported for DuckDB and PostgreSQL. Compilation fails with `"std::vector fields are not supported by this database."` if the database doesn't support it.
- Works with type-safe operations such as `create_table`, `write`/`insert` and `read`
- Nested vectors (`std::vector<std::vector<T>>`) supported to arbitrary depth

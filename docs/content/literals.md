# `sqlgen::literals`

The `sqlgen::literals` namespace provides user-defined literal operators for referencing columns and table aliases in a type-safe, expressive, and concise way when building SQL queries with sqlgen. These literals are essential for writing readable and maintainable query expressions, especially when working with complex queries involving joins and table aliases.

## Usage

### Column Literals: `_c`

Reference a column by name using the `_c` literal:

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

// Reference the "age" column
const auto age_col = "age"_c;

// Use in a query
const auto query = sqlgen::read<std::vector<Person>> | where("age"_c < 18);
```

This generates SQL like:

```sql
SELECT "id", "first_name", "last_name", "age"
FROM "Person"
WHERE "age" < 18;
```

If the column name does not exist in the struct, the code will fail to compile, ensuring type safety.

### Table Alias Literals: `_t1`, `_t2`, ..., `_t99`

When working with joins or subqueries, you often need to disambiguate columns by table alias. The `sqlgen::literals` namespace provides `_tN` suffixes for this purpose, where `N` ranges from 1 to 99:

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

// Reference the "id" column from table alias t1
const auto id_t1 = "id"_t1;

// Reference the "first_name" column from table alias t2
const auto first_name_t2 = "first_name"_t2;
```

#### Example: Joins with Aliases

```cpp
const auto get_people =
    select_from<Person, "t1">(
        "last_name"_t1 | as<"last_name">,
        "first_name"_t1 | as<"first_name_parent">,
        "first_name"_t3 | as<"first_name_child">,
        ("age"_t1 - "age"_t3) | as<"parent_age_at_birth">
    )
    | inner_join<Relationship, "t2">("id"_t1 == "parent_id"_t2)
    | inner_join<Person, "t3">("id"_t3 == "child_id"_t2)
    | order_by("id"_t1, "id"_t3)
    | to<std::vector<ParentAndChild>>;
```

This produces SQL like:

```sql
SELECT t1."last_name" AS "last_name", t1."first_name" AS "first_name_parent", t3."first_name" AS "first_name_child", (t1."age") - (t3."age") AS "parent_age_at_birth"
FROM "Person" t1
INNER JOIN "Relationship" t2 ON t1."id" = t2."parent_id"
INNER JOIN "Person" t3 ON t3."id" = t2."child_id"
ORDER BY t1."id", t3."id"
```

### Column Member Pointers

As an alternative to the literals above, a column can be referenced with a member
pointer to the struct field through the `col` template. This derives the column
name from the struct rather than a string literal:

```cpp
using namespace sqlgen;

// References the "age" column of the Person struct
const auto age_col = col<&Person::age>;
```

Column member pointers can be combined with table
aliases by passing the alias as the second template argument. This is especially
handy for self-joins, where the same table appears more than once:

```cpp
const auto get_people =
    select_from<Person, "t1">(
        col<&Person::first_name, "t1">,
        col<&Person::age, "t1"> - col<&Person::age, "t3">)
    | inner_join<Person, "t3">(col<&Person::id, "t1"> == col<&Person::id, "t3">);
```

This produces SQL like:

```sql
SELECT t1."first_name", (t1."age") - (t3."age")
FROM "Person" t1
INNER JOIN "Person" t3 ON t1."id" = t3."id"
```

For a full rundown of the operations supported on columns, see
[`sqlgen::col`](col.md).

### Supported Suffixes

- `_c` — Reference a column by name (e.g., `"age"_c`)
- `_t1`, `_t2`, ..., `_t99` — Reference a column by name and table alias (e.g., `"id"_t2`)

Instead of these literals, you can reference columns with a member pointer using
the `col` template, e.g. `col<&Person::age>` (see [Column Member Pointers](#column-member-pointers)).

All these literals are defined in the `sqlgen::literals` namespace. It is strongly recommended to use:

```cpp
using namespace sqlgen::literals;
```

in your query files for convenience.

## Type Safety

- Column names are checked at compile time against the struct definition.
- Table aliases are enforced by the literal suffix, preventing ambiguous references in joins.
- If you use a column or alias that does not exist, the code will fail to compile.

## Advanced: Custom Aliases

If you need an alias outside the provided `_tN` range, you can use the `col` template directly:

```cpp
const auto custom_col = col<"id", "custom_alias">;
```

## Notes

- The `_c` and `_tN` literals return `Col` objects, which support all standard SQL operations (comparison, ordering, etc.).
- These literals are composable with all sqlgen query builder functions, such as `where`, `order_by`, `group_by`, and join conditions.
- The use of these literals is required for type-safe, readable, and maintainable query construction in sqlgen.
- All operations are thread-safe and have no mutable state.

## Example: Full Query Composition

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

const auto query = sqlgen::read<std::vector<Person>>
                   | where("age"_c >= 18 and "last_name"_c == "Simpson")
                   | order_by("first_name"_c.desc())
                   | limit(10)
                   | offset(3);
```

This generates:

```sql
SELECT "id", "first_name", "last_name", "age"
FROM "Person"
WHERE ("age" >= 18) AND ("last_name" = 'Simpson')
ORDER BY "first_name" DESC
LIMIT 10
OFFSET 3;
```

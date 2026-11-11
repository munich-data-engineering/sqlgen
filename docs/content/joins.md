# `sqlgen::inner_join`, `sqlgen::left_join`, `sqlgen::right_join`, `sqlgen::full_join`

The `sqlgen` library provides a type-safe, composable interface for expressing SQL joins in C++. It supports all standard SQL join types (inner, left, right, full) and allows for both simple and nested join queries, including joining on subqueries.

## Join Types

The following join types are available:

- `inner_join` — Returns rows when there is a match in both tables
- `left_join` — Returns all rows from the left table, and matched rows from the right table
- `right_join` — Returns all rows from the right table, and matched rows from the left table
- `full_join` — Returns all rows when there is a match in one of the tables (not supported by MariaDB)

Each join type can be used with either a table or a subquery, and can be aliased for use in complex queries.

## Usage

### Basic Join Example

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

struct Person {
    sqlgen::PrimaryKey<uint32_t> id;
    std::string first_name;
    std::string last_name;
    double age;
};

struct Relationship {
    sqlgen::PrimaryKey<uint32_t> parent_id;
    sqlgen::PrimaryKey<uint32_t> child_id;
};

// Join two tables: Relationship (as t1) and Person (as t2)
const auto get_children =
    select_from<Relationship, "t1">(
        "parent_id"_t1 | as<"id">,
        "first_name"_t2 | as<"first_name">,
        "age"_t2 | as<"age">
    ) |
    left_join<Person, "t2">("id"_t2 == "child_id"_t1);
```

This produces a query equivalent to:

```sql
SELECT t1."parent_id" AS "id", t2."first_name" AS "first_name", t2."age" AS "age"
FROM "Relationship" t1
LEFT JOIN "Person" t2 ON t2."id" = t1."child_id"
```

### Chaining Multiple Joins

You can chain multiple joins to build more complex queries:

```cpp
const auto get_people =
    select_from<Person, "t1">(
        "last_name"_t1 | as<"last_name">,
        "first_name"_t3 | as<"first_name_child">,
        avg("age"_t1 - "age"_t3) | as<"avg_parent_age_at_birth">
    ) |
    inner_join<Relationship, "t2">("id"_t1 == "parent_id"_t2) |
    left_join<Person, "t3">("id"_t3 == "child_id"_t2) |
    group_by("last_name"_t1, "first_name"_t3) |
    order_by("last_name"_t1, "first_name"_t3) |
    to<std::vector<ParentAndChild>>;
```

This produces a query with both an inner and a left join, grouping and ordering the results.

### Nested Joins and Subqueries

`sqlgen` allows you to join on subqueries, enabling nested join patterns:

```cpp
const auto get_children =
    select_from<Relationship, "t1">(
        "parent_id"_t1 | as<"id">,
        "first_name"_t2 | as<"first_name">,
        "age"_t2 | as<"age">
    ) |
    left_join<Person, "t2">("id"_t2 == "child_id"_t1);

const auto get_people =
    select_from<Person, "t1">(
        "last_name"_t1 | as<"last_name">,
        "first_name"_t2 | as<"first_name_child">,
        avg("age"_t1 - "age"_t2) | as<"avg_parent_age_at_birth">
    ) |
    inner_join<"t2">(get_children, "id"_t1 == "id"_t2) |
    group_by("last_name"_t1, "first_name"_t2) |
    order_by("first_name"_t2) |
    to<std::vector<ParentAndChild>>;
```

This produces a query with a subquery in the join clause:

```sql
SELECT t1."last_name" AS "last_name", t2."first_name" AS "first_name_child", AVG((t1."age") - (t2."age")) AS "avg_parent_age_at_birth"
FROM "Person" t1
INNER JOIN (
    SELECT t1."parent_id" AS "id", t2."first_name" AS "first_name", t2."age" AS "age"
    FROM "Relationship" t1
    INNER JOIN "Person" t2 ON t2."id" = t1."child_id"
) t2 ON t1."id" = t2."id"
GROUP BY t1."last_name", t2."first_name"
ORDER BY t2."first_name"
```

## Join Syntax

- `inner_join<Table, Alias>(condition)` — Inner join with a table
- `left_join<Table, Alias>(condition)` — Left join with a table
- `right_join<Table, Alias>(condition)` — Right join with a table
- `full_join<Table, Alias>(condition)` — Full join with a table
- `inner_join<Alias>(subquery, condition)` — Inner join with a subquery
- `left_join<Alias>(subquery, condition)` — Left join with a subquery
- `right_join<Alias>(subquery, condition)` — Right join with a subquery
- `full_join<Alias>(subquery, condition)` — Full join with a subquery

Where:
- `Table` is the C++ struct representing the table
- `Alias` is a string literal alias for the table or subquery
- `condition` is a boolean expression relating columns from the joined tables
- `subquery` is a previously constructed query expression

## Aliasing and Column References

When joining tables or subqueries, you must use aliases to disambiguate columns. Use the `_t1`, `_t2`, etc. 
suffixes to refer to columns from different tables or subqueries, which are defined in the 
namespace `sqlgen::literals`:

```cpp
"id"_t1, "first_name"_t2, "age"_t3
```

The library includes predefined suffixes from `_t1` to `_t99`.

If you want to use your own alias, you can use this syntax instead:

```cpp
col<"id", "your_alias">, col<"first_name", "your_alias">, col<"age", "your_alias">
```

### Column Member Pointers

As an alternative to the `_tN` suffixes or the `col<"col", "alias">` syntax, the
columns in a join can be referenced with a member pointer to a struct field,
passing the alias as the second template argument. This is especially handy for
self-joins, where the same table appears more than once:

```cpp
const auto get_people =
    select_from<Person, "t1">(
        col<&Person::last_name, "t1"> | as<"last_name">,
        col<&Person::first_name, "t3"> | as<"first_name_child">,
        avg(col<&Person::age, "t1"> - col<&Person::age, "t3">) | as<"avg_parent_age_at_birth">
    ) |
    inner_join<Relationship, "t2">(col<&Person::id, "t1"> == col<&Relationship::parent_id, "t2">) |
    left_join<Person, "t3">(col<&Person::id, "t3"> == col<&Relationship::child_id, "t2">) |
    group_by(col<&Person::last_name, "t1">, col<&Person::first_name, "t3">) |
    order_by(col<&Person::last_name, "t1">, col<&Person::first_name, "t3">) |
    to<std::vector<ParentAndChild>>;
```

This produces a query with both an inner and a left join, grouping and ordering
the results. The join conditions, `group_by`, and `order_by` all reference their
columns the same way.

## Advanced: Nested and Grouped Joins

You can nest joins and combine them with grouping and aggregation:

```cpp
const auto get_people =
    select_from<Person, "t1">(
        "last_name"_t1 | as<"last_name">,
        "first_name"_t2 | as<"first_name_child">,
        avg("age"_t1 - "age"_t2) | as<"avg_parent_age_at_birth">
    ) |
    inner_join<"t2">(get_children, "id"_t1 == "id"_t2) |
    group_by("last_name"_t1, "first_name"_t2) |
    order_by("first_name"_t2) |
    to<std::vector<ParentAndChild>>;
```

## Nullability in Joins

When performing outer joins (`left_join`, `right_join`, `full_join`), columns from the joined table may contain `NULL` values if no matching row is found. `sqlgen` handles this by wrapping the type of such columns in `std::optional`. If a field is already nullable (i.e., `std::optional<T>`), it remains `std::optional<T>`.

-   `inner_join`: Does not introduce nullability. The types of the selected columns remain unchanged.
-   `left_join`: Columns selected from the right table (the one being joined) will become `std::optional<T>` if they are not already nullable.
-   `right_join`: Columns selected from the left table (the `select_from` table) will become `std::optional<T>` if they are not already nullable.
-   `full_join`: Columns selected from both tables will become `std::optional<T>` if they are not already nullable.

### Example: Nullability in a `left_join`

Consider a `left_join` between `Person` and `Relationship` tables. If a person has no corresponding relationship, the columns from `Relationship` will be `NULL`.

```cpp
struct PersonWithOptionalRelationship {
    uint32_t id;
    std::string first_name;
    std::optional<uint32_t> child_id;
};

const auto get_people_with_optional_relationship =
    select_from<Person, "t1">(
        "id"_t1 | as<"id">,
        "first_name"_t1 | as<"first_name">,
        "child_id"_t2 | as<"child_id">
    ) |
    left_join<Relationship, "t2">("id"_t1 == "parent_id"_t2) |
    to<std::vector<PersonWithOptionalRelationship>>;
```

In this example, `child_id` is selected from the right table (`Relationship`) of a `left_join`. Therefore, its type in the result struct `PersonWithOptionalRelationship` must be `std::optional<uint32_t>` to handle cases where no matching relationship is found.

### Nullability in Multi-Table Joins

Nullability propagates through chained joins. If a table becomes nullable due to an outer join, any subsequent joins that use columns from that table will also treat them as nullable.

For example, consider a query that joins `Person` to `Relationship` (left join) and then `Relationship` to a hypothetical `Pet` table (left join).

```cpp
struct PersonWithPets {
    uint32_t person_id;
    std::optional<uint32_t> child_id;
    std::optional<std::string> pet_name;
};

// Assume Pet table has { id, name, owner_id }
const auto get_people_with_pets =
    select_from<Person, "t1">(
        "id"_t1 | as<"person_id">,
        "child_id"_t2 | as<"child_id">,
        "name"_t3 | as<"pet_name">
    ) |
    left_join<Relationship, "t2">("id"_t1 == "parent_id"_t2) |
    left_join<Pet, "t3">("child_id"_t2 == "owner_id"_t3) | // Joining on a nullable-prone column
    to<std::vector<PersonWithPets>>;
```

In this query:
1.  The first `left_join` makes columns from `Relationship` (`t2`) nullable. So, `child_id` becomes `std::optional<uint32_t>`.
2.  The second `left_join` joins `Pet` (`t3`). Since `t3` is on the right side of a `left_join`, its columns (like `pet_name`) also become nullable, resulting in `std::optional<std::string>`.

If an `inner_join` were used for the second join (`Pet`), `pet_name` would still be `std::optional<std::string>` if the `Pet` table itself has a nullable `name` column. However, the join logic itself wouldn't introduce nullability for the `Pet` table's columns. The key principle is that the nullability of a column in the final result depends on both its original type and the types of joins applied to its table.

## Notes

- Joins can be chained and nested arbitrarily
- Aliases are required for all joined tables and subqueries
- Join conditions must use the correct aliases for columns
- You can join on subqueries, enabling powerful query composition
- Joins can be combined with `group_by`, `order_by`, and aggregation functions
- The result type must match the structure of your select statement, with field names matching the aliases used in the query

## Example: Full Nested Join Query

```cpp
struct ParentAndChild {
    std::string last_name;
    std::string first_name_child;
    double avg_parent_age_at_birth;
};

const auto get_children =
    select_from<Relationship, "t1">("parent_id"_t1 | as<"id">,
                                    "first_name"_t2 | as<"first_name">,
                                    "age"_t2 | as<"age">) |
    inner_join<Person, "t2">("id"_t2 == "child_id"_t1);

const auto get_people =
    select_from<Person, "t1">(
        "last_name"_t1 | as<"last_name">,
        "first_name"_t2 | as<"first_name_child">,
        avg("age"_t1 - "age"_t2) | as<"avg_parent_age_at_birth">
    ) |
    inner_join<"t2">(get_children, "id"_t1 == "id"_t2) |
    group_by("last_name"_t1, "first_name"_t2) |
    order_by("first_name"_t2) |
    to<std::vector<ParentAndChild>>;
```

This query joins a table to a subquery, groups by two columns, and computes an aggregate over the join.


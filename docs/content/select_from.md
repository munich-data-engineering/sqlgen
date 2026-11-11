# `sqlgen::select_from`

The `sqlgen::select_from` function provides a type-safe, composable interface for building SQL SELECT queries in C++. It supports selecting from tables, subqueries, and complex nested queries with full support for joins, filtering, grouping, ordering, and result type conversion.

## Basic Usage

### Selecting from a Table

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

struct Person {
    sqlgen::PrimaryKey<uint32_t> id;
    std::string first_name;
    std::string last_name;
    int age;
};

// Select specific columns
const auto query = select_from<Person>("first_name"_c, "last_name"_c, "age"_c);

// Execute the query
const auto people = sqlite::connect()
    .and_then(query | to<std::vector<Person>>)
    .value();
```

This generates:
```sql
SELECT "first_name", "last_name", "age" FROM "Person"
```

### Selecting with Aliases

Use aliases to rename columns in the result set:

```cpp
struct Result {
    std::string name;
    int person_age;
};

const auto query = select_from<Person>(
    "first_name"_c | as<"name">,
    "age"_c | as<"person_age">
) | to<std::vector<Result>>;
```

This generates:
```sql
SELECT "first_name" AS "name", "age" AS "person_age" FROM "Person"
```

## Table Aliases

### Aliased Tables

When you need to reference the same table multiple times or want cleaner column references:

```cpp
const auto query = select_from<Person, "t1">(
    "first_name"_t1,
    "last_name"_t1,
    "age"_t1
) | where("age"_t1 > 18);
```

This generates:
```sql
SELECT t1."first_name", t1."last_name", t1."age" FROM "Person" t1 WHERE t1."age" > 18
```

## Selecting from Subqueries

You can use the result of one query as the source for another query:

```cpp
// First, create a subquery
const auto get_parents = select_from<Person, "t1">(
    "child_id"_t2 | as<"id">,
    "first_name"_t1 | as<"first_name">,
    "last_name"_t1 | as<"last_name">,
    "age"_t1 | as<"age">
) | inner_join<Relationship, "t2">("id"_t1 == "parent_id"_t2);

// Then use it as a source for another query
const auto get_people = select_from<"t1">(
    get_parents,  // Use the subquery as the source
    "last_name"_t1 | as<"last_name">,
    "first_name"_t1 | as<"first_name_parent">,
    "first_name"_t2 | as<"first_name_child">,
    ("age"_t1 - "age"_t2) | as<"parent_age_at_birth">
) | inner_join<Person, "t2">("id"_t1 == "id"_t2);
```

This generates:
```sql
SELECT t1."last_name" AS "last_name", t1."first_name" AS "first_name_parent", t2."first_name" AS "first_name_child", (t1."age") - (t2."age") AS "parent_age_at_birth" 
FROM (
    SELECT t2."child_id" AS "id", t1."first_name" AS "first_name", t1."last_name" AS "last_name", t1."age" AS "age" 
    FROM "Person" t1 
    INNER JOIN "Relationship" t2 ON t1."id" = t2."parent_id"
) t1 
INNER JOIN "Person" t2 ON t1."id" = t2."id"
```

## Expressions and Calculations

You can select calculated expressions and literal values:

```cpp
struct Result {
    std::string field;
    double avg_field2;
    std::optional<std::string> nullable_field;
    int one;
    std::string hello;
};

const auto query = select_from<TestTable>(
    "field1"_c.as<"field">(),
    avg("field2"_c).as<"avg_field2">(),
    "nullable"_c | as<"nullable_field">,
    as<"one">(1),                    // Literal value
    "hello" | as<"hello">            // String literal
) | to<Result>;
```

## Query Composition

`select_from` can be combined with various query operations:

```cpp
const auto query = select_from<Person>(
    "first_name"_c,
    "last_name"_c,
    "age"_c
)
| where("age"_c >= 18)              // Filter results
| order_by("last_name"_c, "first_name"_c)  // Order results
| limit(10)                         // Limit number of results
| offset(5)                         // Offset the result set
| to<std::vector<Person>>;          // Convert to container
```

### With Joins

```cpp
const auto query = select_from<Person, "t1">(
    "last_name"_t1 | as<"last_name">,
    "first_name"_t3 | as<"first_name_child">,
    avg("age"_t1 - "age"_t3) | as<"avg_parent_age_at_birth">
)
| inner_join<Relationship, "t2">("id"_t1 == "parent_id"_t2)
| inner_join<Person, "t3">("id"_t3 == "child_id"_t2)
| group_by("last_name"_t1, "first_name"_t3)
| order_by("last_name"_t1, "first_name"_t3)
| to<std::vector<ParentAndChild>>;
```

## Result Type Conversion

### Using `to<Container>`

Convert query results to specific container types:

```cpp
// Convert to vector
const auto people_vec = query | to<std::vector<Person>>;

// Convert to single result (query must return exactly one row)
const auto person = query | to<Person>;

// Return as Range (lazy evaluation)
const auto people_range = query;  // Returns Range<...> by default
```

### Automatic Type Deduction

If you don't specify `to<...>`, `select_from` returns a `Range` type that can be iterated:

```cpp
const auto query = select_from<Person>("first_name"_c, "age"_c);

// Execute and iterate
for (const auto& result : sqlite::connect().and_then(query).value()) {
    if (result) {
        std::cout << result->first_name << ": " << result->age << std::endl;
    }
}
```

## Syntax Variations

### Three Main Forms

1. **Table without alias:**
   ```cpp
   select_from<TableType>(fields...)
   ```

2. **Table with alias:**
   ```cpp
   select_from<TableType, "t1">(fields...)
   ```

3. **Subquery with alias:**
   ```cpp
   select_from<"t1">(subquery, fields...)
   ```

## Column References

Use the predefined column literals for clean syntax:

```cpp
using namespace sqlgen::literals;

// Use _c for unaliased tables
"column_name"_c

// Use _t1, _t2, etc. for aliased tables/joins
"column_name"_t1, "column_name"_t2

// Or use explicit col syntax
col<"column_name", "t1">
```

## Referencing columns with member pointers

As an alternative to the column literals above, every example can use a member
pointer to a struct field through the `col` template, e.g. `col<&Person::age>`.
This derives the column name from the struct instead of a string literal, so the
code compiles only if the field exists:

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

// Select specific columns via member pointers
const auto query = select_from<Person>(
    col<&Person::first_name>,
    col<&Person::last_name>,
    col<&Person::age>);
```

This generates (note that sqlgen qualifies the columns with the table name):

```sql
SELECT "Person"."first_name", "Person"."last_name", "Person"."age" FROM "Person"
```

The same applies to aliases and aliased tables:

```cpp
struct Result {
    std::string name;
    int person_age;
};

const auto query = select_from<Person>(
    col<&Person::first_name> | as<"name">,
    col<&Person::age> | as<"person_age">
) | to<std::vector<Result>>;

const auto people = sqlite::connect()
    .and_then(query)
    .value();
```

This generates:

```sql
SELECT "Person"."first_name" AS "name", "Person"."age" AS "person_age" FROM "Person"
```

For aliased tables, pass the alias as the second template argument:

```cpp
const auto query = select_from<Person, "t1">(
    col<&Person::first_name, "t1">,
    col<&Person::last_name, "t1">,
    col<&Person::age, "t1">
) | where(col<&Person::age, "t1"> > 18);
```

This generates:

```sql
SELECT t1."first_name", t1."last_name", t1."age" FROM "Person" t1 WHERE t1."age" > 18
```

And query composition works the same way:

```cpp
const auto query = select_from<Person>(
    col<&Person::first_name>,
    col<&Person::last_name>,
    col<&Person::age>
)
| where(col<&Person::age> >= 18)              // Filter results
| order_by(col<&Person::last_name>, col<&Person::first_name>)  // Order results
| limit(10)                                  // Limit number of results
| offset(5)                                  // Offset the result set
| to<std::vector<Person>>;                   // Convert to container
```

## Advanced Examples

### Complex Nested Query

```cpp
struct ParentAndChild {
    std::string last_name;
    std::string first_name_parent;
    std::string first_name_child;
    double parent_age_at_birth;
};

// Step 1: Create a subquery to get parent-child relationships
const auto get_children = select_from<Relationship, "t1">(
    "parent_id"_t1 | as<"id">,
    "first_name"_t2 | as<"first_name">,
    "age"_t2 | as<"age">
) | inner_join<Person, "t2">("id"_t2 == "child_id"_t1);

// Step 2: Use the subquery in a larger query
const auto get_people = select_from<Person, "t1">(
    "last_name"_t1 | as<"last_name">,
    "first_name"_t1 | as<"first_name_parent">,
    "first_name"_t2 | as<"first_name_child">,
    ("age"_t1 - "age"_t2) | as<"parent_age_at_birth">
) 
| inner_join<"t2">(get_children, "id"_t1 == "id"_t2)
| order_by("id"_t1, "id"_t2)
| to<std::vector<ParentAndChild>>;
```

### Aggregation with Grouping

```cpp
const auto summary = select_from<Person>(
    "last_name"_c,
    count() | as<"family_size">,
    avg("age"_c) | as<"avg_age">,
    min("age"_c) | as<"youngest">,
    max("age"_c) | as<"oldest">
) 
| group_by("last_name"_c)
| order_by("last_name"_c)
| to<std::vector<FamilySummary>>;
```

## Important Notes

- **Field Matching**: Result struct field names must exactly match the aliases used in the query
- **Type Safety**: All column references and types are checked at compile time
- **Null Handling**: Use `std::optional<T>` for nullable columns in result types
- **Execution**: Queries are executed when passed to a database connection
- **Composition**: Queries can be built incrementally and reused as subqueries
- **Aliases Required**: When using joins or subqueries, table aliases are mandatory for disambiguation and must follow the pattern `t1`, `t2`, `t3`, etc.

## Error Handling

Queries return `Result<T>` types that must be handled:

```cpp
const auto result = sqlite::connect()
    .and_then(query)
    .value();  // Throws on error

// Or handle errors explicitly
const auto result = sqlite::connect()
    .and_then(query);
    
if (result) {
    // Success
    use_data(*result);
} else {
    // Handle error
    std::cerr << "Query failed: " << result.error().what() << std::endl;
}
```


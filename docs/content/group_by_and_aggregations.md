# `sqlgen::group_by` and Aggregations

The `sqlgen::group_by` interface provides a type-safe way to group and aggregate data in SQL queries. It supports various aggregation functions like `count`, `avg`, `sum`, `min`, `max`, and `count_distinct`.

## Usage

### Basic Aggregations

Perform aggregations without grouping:

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

struct Children {
    int num_children;
    int num_last_names;
    double avg_age;
    double max_age;
    double min_age;
    double sum_age;
};

const auto get_children = select_from<Person>(
    avg("age"_c).as<"avg_age">(),
    count().as<"num_children">(),
    max("age"_c).as<"max_age">(),
    min("age"_c).as<"min_age">(),
    sum("age"_c).as<"sum_age">(),
    count_distinct("last_name"_c).as<"num_last_names">()
) | where("age"_c < 18) | to<Children>;

const auto children = sqlite::connect()
    .and_then(get_children)
    .value();
```

This generates the following SQL:

```sql
SELECT 
    AVG("age") as "avg_age",
    COUNT(*) as "num_children",
    MAX("age") as "max_age",
    MIN("age") as "min_age",
    SUM("age") as "sum_age",
    COUNT(DISTINCT "last_name") as "num_last_names"
FROM "Person"
WHERE "age" < 18;
```

### Group By with Aggregations

Group data and perform aggregations:

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

struct Children {
    std::string last_name;
    int num_children;
    int num_last_names;
    double avg_age;
    double max_age;
    double min_age;
    double sum_age;
};

const auto get_children = select_from<Person>(
    "last_name"_c,
    avg("age"_c).as<"avg_age">(),
    count().as<"num_children">(),
    max("age"_c).as<"max_age">(),
    min("age"_c).as<"min_age">(),
    sum("age"_c).as<"sum_age">(),
    count_distinct("last_name"_c).as<"num_last_names">()
) | where("age"_c < 18) | group_by("last_name"_c) | to<std::vector<Children>>;

const auto children = sqlite::connect()
    .and_then(drop<Person> | if_exists)
    .and_then(write(std::ref(people)))
    .and_then(get_children)
    .value();
```

This generates the following SQL:

```sql
SELECT 
    "last_name",
    AVG("age") as "avg_age",
    COUNT(*) as "num_children",
    MAX("age") as "max_age",
    MIN("age") as "min_age",
    SUM("age") as "sum_age",
    COUNT(DISTINCT "last_name") as "num_last_names"
FROM "Person"
WHERE "age" < 18
GROUP BY "last_name";
```

## Available Aggregation Functions

The following aggregation functions are available:

- `count()` - Counts all rows
- `count_distinct(col)` - Counts distinct values in a column
- `avg(col)` - Calculates average of a column
- `sum(col)` - Calculates sum of a column
- `max(col)` - Finds maximum value in a column
- `min(col)` - Finds minimum value in a column

Each aggregation function can be aliased using the `.as<"alias">()` syntax.

## Aggregating Nullable Values

When working with nullable columns (e.g., `std::optional<T>`), it's important to understand how aggregations handle null values:

1. The result of aggregating a nullable column must also be nullable (e.g., `std::optional<double>`)
2. Null values are excluded from aggregation calculations
3. If all values in a group are null, the aggregation result will be null

### Example with Nullable Values

```cpp
struct Person {
    sqlgen::PrimaryKey<uint32_t> id;
    std::string first_name;
    std::string last_name;
    std::optional<double> age;  // Nullable age field
};

struct Children {
    int num_children;           // Non-nullable count
    int num_last_names;        // Non-nullable count
    std::optional<double> avg_age;  // Nullable aggregation
    std::optional<double> max_age;  // Nullable aggregation
    std::optional<double> min_age;  // Nullable aggregation
    std::optional<double> sum_age;  // Nullable aggregation
};

const auto get_children = select_from<Person>(
    avg("age"_c).as<"avg_age">(),      // Results in std::optional<double>
    count().as<"num_children">(),      // Results in int (non-nullable)
    max("age"_c).as<"max_age">(),      // Results in std::optional<double>
    min("age"_c).as<"min_age">(),      // Results in std::optional<double>
    sum("age"_c).as<"sum_age">(),      // Results in std::optional<double>
    count_distinct("last_name"_c).as<"num_last_names">()
) | where("age"_c < 18 or "age"_c.is_null()) | to<Children>;
```

In this example:
- The `age` column is nullable (`std::optional<double>`)
- All aggregations of `age` result in nullable values (`std::optional<double>`)
- The `count()` and `count_distinct()` functions return non-nullable integers
- The `where` clause includes both non-null values less than 18 and null values
- If all values in a group are null, the aggregation results (avg, max, min, sum) will be null

### Important Notes

- Always use nullable types (`std::optional<T>`) for aggregated values when the source column is nullable
- Count operations (`count()`, `count_distinct()`) always return non-nullable integers
- Null values are excluded from aggregation calculations (avg, sum, max, min)
- When all values in a group are null, the aggregation result will be null
- The type system will enforce these rules at compile time

## The `.as` Operator

The `.as` operator allows you to alias columns and expressions in your SQL queries. This is particularly important when mapping query results to C++ structs, as the field names in your struct must match the aliases used in the query.

### Usage

You can use `.as` in several ways:

```cpp
// Method 1: Using the .as() member function
"field1"_c.as<"field">()

// Method 2: Using the pipe operator with as
"nullable"_c | as<"nullable_field">

// Method 3: Using as directly with values
as<"one">(1)
"hello" | as<"hello">
```

### Important Notes

- Field names in your result struct must exactly match the aliases used in the query
- Fields are matched by name, not by order
- You can alias any column, expression, or literal value
- The alias must be a string literal (using the `as<"alias">` syntax)

### Example

```cpp
struct Result {
    std::string field;           // Matches "field1"_c.as<"field">()
    double avg_field2;           // Matches avg("field2"_c).as<"avg_field2">()
    std::optional<std::string> nullable_field;  // Matches "nullable"_c | as<"nullable_field">
    int one;                     // Matches as<"one">(1)
    std::string hello;           // Matches "hello" | as<"hello">
};

const auto query = select_from<TestTable>(
    "field1"_c.as<"field">(),
    avg("field2"_c).as<"avg_field2">(),
    "nullable"_c | as<"nullable_field">,
    as<"one">(1),
    "hello" | as<"hello">
) | to<Result>;
```

In this example, each field in the `Result` struct must have a name that matches one of the aliases used in the query. The order of the fields in the struct doesn't matter, but the names must match exactly.

## Using member pointers

The aggregation and grouping examples above can also reference columns with a
member pointer to the struct field through the `col` template. The generated SQL
is the same; only the C++ reference to the column changes:

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

const auto get_children = select_from<Person>(
    avg(col<&Person::age>).as<"avg_age">(),
    count().as<"num_children">(),
    max(col<&Person::age>).as<"max_age">(),
    min(col<&Person::age>).as<"min_age">(),
    sum(col<&Person::age>).as<"sum_age">(),
    count_distinct(col<&Person::last_name>).as<"num_last_names">()
) | where(col<&Person::age> < 18) | group_by(col<&Person::last_name>) | to<std::vector<Children>>;
```

The generated SQL is identical to the examples above, except that the referenced
columns are qualified with the table name (e.g. `AVG("Person"."age")`,
`GROUP BY "Person"."last_name"`), because sqlgen knows the referenced type at
compile time.

## Notes

- The `group_by` clause must be used before `order_by` or `limit` and `offset` clauses
- You cannot use `group_by` multiple times in the same query
- Aggregation functions can be used with or without `group_by`
- The result type must match the structure of your select statement. Note that fields are matched by name, not order.
- All aggregations are type-safe and will map to appropriate SQL types
- The `Result<...>` type provides error handling; use `.value()` to extract the result (will throw an exception if there's an error) or handle errors as needed

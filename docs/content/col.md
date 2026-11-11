# `sqlgen::col`

`sqlgen::col` provides a type-safe way to reference and manipulate database columns in SQL queries. It enables building complex SQL conditions, ordering, and updates through a fluent C++ interface.

## Usage

### Basic Definition

Reference a column using the string literal operator:

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

// Using string literal operator
const auto age_col = "age"_c;

// Using col template function
const auto name_col = col<"first_name">;
// Using a member pointer (resolved at compile time)
const auto age_col_mp = col<&Person::age>;
```

### Column Operations

#### Comparison Operations

Compare columns with values or other columns:

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

// Compare with value
const auto query1 = read<std::vector<Person>> | where("age"_c > 18);

// Compare with another column
const auto query2 = read<std::vector<Person>> |
                   where("age"_c > "id"_c);
```

This generates SQL like:

```sql
-- For query1
SELECT "id", "first_name", "last_name", "age"
FROM "Person"
WHERE "age" > 18;

-- For query2
SELECT "id", "first_name", "last_name", "age"
FROM "Person"
WHERE "age" > "id";
```

#### NULL Operations

Check for NULL or NOT NULL values:

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

// Find records where age is NULL
const auto query1 = read<std::vector<Person>> |
                   where("age"_c.is_null());

// Find records where age is NOT NULL
const auto query2 = read<std::vector<Person>> |
                   where("age"_c.is_not_null());
```

This generates SQL like:

```sql
-- For query1
SELECT "id", "first_name", "last_name", "age"
FROM "Person"
WHERE "age" IS NULL;

-- For query2
SELECT "id", "first_name", "last_name", "age"
FROM "Person"
WHERE "age" IS NOT NULL;
```

#### Pattern Matching

Use LIKE and NOT LIKE for pattern matching:

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

// Find names starting with 'H'
const auto query1 = read<std::vector<Person>> |
                   where("first_name"_c.like("H%"));

// Find names not starting with 'H'
const auto query2 = read<std::vector<Person>> |
                   where("first_name"_c.not_like("H%"));
```

This generates SQL like:

```sql
-- For query1
SELECT "id", "first_name", "last_name", "age"
FROM "Person"
WHERE "first_name" LIKE 'H%';

-- For query2
SELECT "id", "first_name", "last_name", "age"
FROM "Person"
WHERE "first_name" NOT LIKE 'H%';
```

#### IN and NOT IN Operations

Use IN and NOT IN to check if a column value matches any value in a list:

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

// Find people with specific first names (variadic arguments)
const auto query1 = read<std::vector<Person>> |
                   where("first_name"_c.in("Bart", "Lisa", "Maggie"));

// Find people NOT with specific first names (variadic arguments)
const auto query2 = read<std::vector<Person>> |
                   where("first_name"_c.not_in("Homer", "Hugo"));

// Find people with specific first names (using vector)
const auto names = std::vector<std::string>({"Bart", "Lisa", "Maggie"});
const auto query3 = read<std::vector<Person>> |
                   where("first_name"_c.in(names));

// Find people NOT with specific first names (using vector)
const auto excluded_names = std::vector<std::string>({"Homer", "Hugo"});
const auto query4 = read<std::vector<Person>> |
                   where("first_name"_c.not_in(excluded_names));
```

This generates SQL like:

```sql
-- For query1
SELECT "id", "first_name", "last_name", "age"
FROM "Person"
WHERE "first_name" IN ('Bart', 'Lisa', 'Maggie');

-- For query2
SELECT "id", "first_name", "last_name", "age"
FROM "Person"
WHERE "first_name" NOT IN ('Homer', 'Hugo');

-- For query3
SELECT "id", "first_name", "last_name", "age"
FROM "Person"
WHERE "first_name" IN ('Bart', 'Lisa', 'Maggie');

-- For query4
SELECT "id", "first_name", "last_name", "age"
FROM "Person"
WHERE "first_name" NOT IN ('Homer', 'Hugo');
```

#### Ordering

Specify column ordering in queries:

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

// Order by age ascending
const auto query1 = read<std::vector<Person>> |
                    order_by("age"_c);

// Order by age descending
const auto query2 = read<std::vector<Person>> |
                    order_by("age"_c.desc());

// Multiple columns with mixed ordering
const auto query3 = read<std::vector<Person>> |
                    order_by("last_name"_c, "first_name"_c.desc());
```

This generates SQL like:

```sql
-- For query1
SELECT "id", "first_name", "last_name", "age"
FROM "Person"
ORDER BY "age";

-- For query2
SELECT "id", "first_name", "last_name", "age"
FROM "Person"
ORDER BY "age" DESC;

-- For query3
SELECT "id", "first_name", "last_name", "age"
FROM "Person"
ORDER BY "last_name", "first_name" DESC;
```

#### Updates

Set column values in UPDATE statements:

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

// Update a single column
const auto query1 = update<Person>("age"_c.set(46));

// Update multiple columns
const auto query2 = update<Person>(
    "first_name"_c.set("last_name"_c),
    "age"_c.set(100)
) | where("first_name"_c == "Hugo");
```

This generates SQL like:

```sql
-- For query1
UPDATE "Person" SET "age" = 46;

-- For query2
UPDATE "Person"
SET "first_name" = "last_name", "age" = 100
WHERE "first_name" = 'Hugo';
```

## Referencing columns with member pointers

As an alternative to the string literals above, columns can be referenced with a
member pointer to the struct field through the `col` template:

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

// References the "age" column of the Person struct
const auto age_col = col<&Person::age>;

const auto query = read<std::vector<Person>> |
                   where(col<&Person::age> > 18) |
                   order_by(col<&Person::age>.desc());
```

Because the referenced type is known at compile time,
sqlgen qualifies the generated SQL with the table name:

```sql
SELECT ... FROM "Person" WHERE "Person"."age" > 18 ORDER BY "Person"."age" DESC
```

#### Column member pointers in joins

When a table is given an alias (for example in a join), pass the alias as the
second template argument. This also works for self-joins, where the same table
appears more than once:

```cpp
const auto get_people =
    select_from<Person, "t1">(
        col<&Person::first_name, "t1">,
        col<&Person::age, "t1"> - col<&Person::age, "t3">) |
    inner_join<Person, "t3">(col<&Person::id, "t1"> == col<&Person::id, "t3">);
```

This produces SQL like:

```sql
SELECT t1."first_name", (t1."age") - (t3."age")
FROM "Person" t1
INNER JOIN "Person" t3 ON t1."id" = t3."id"
```

#### Notes

- In joins, use `col<&Table::column, "alias">` to disambiguate columns coming from
  different tables or aliases. See [joins](joins.md) for details.
- The `col<&Table::column>` and `col<&Table::column, "alias">` forms are fully
  composable with every operation described above (`where`, `order_by`,
  `group_by`, `in`, `like`, `is_null`, `set`, aggregations, and more).

## Type Safety

`sqlgen::col` class provides compile-time type safety:

- Column names are validated at compile time using string literals
- SQL injection is prevented through proper escaping and parameterization

## Notes

- The class supports all standard SQL comparison operators: `==`, `!=`, `<`, `<=`, `>`, `>=`
- Column names are automatically quoted in generated SQL
- The class is designed to be used with the query builder interface
- All operations are composable and can be chained together
- The class supports both value and column-to-column comparisons
- String literals are automatically converted to the appropriate SQL type
- The class is thread-safe and has no mutable state

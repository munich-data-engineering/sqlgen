# `sqlgen::create_as` 

The `sqlgen::create_as` interface provides a type-safe way to create tables and views from SELECT queries in a SQL database. It supports both `CREATE TABLE AS` and `CREATE VIEW AS` syntax, with options for `if_not_exists` and `or_replace` clauses.

## Usage

### Basic Create Table As

Create a table from a SELECT query:

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

const auto names_query = select_from<Person>("first_name"_c, "last_name"_c);
const auto get_names = create_as<Name>(names_query);

get_names(conn).value();
```

This generates the following SQL:

```sql
CREATE TABLE "Name" AS SELECT "first_name", "last_name" FROM "Person";
```

### With `if_not_exists` clause

Create a table only if it doesn't exist:

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

const auto names_query = select_from<Person>("first_name"_c, "last_name"_c);
const auto get_names = create_as<Name>(names_query) | if_not_exists;

get_names(conn).value();
```

This generates the following SQL:

```sql
CREATE TABLE IF NOT EXISTS "Name" AS SELECT "first_name", "last_name" FROM "Person";
```

### Create View As

Create a view from a SELECT query:

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

const auto names_query = select_from<Person>("first_name"_c, "last_name"_c);
const auto get_names = create_or_replace_view_as<Name>(names_query);

get_names(conn).value();
```

This generates the following SQL:

```sql
CREATE OR REPLACE VIEW "NAMES" AS SELECT "first_name", "last_name" FROM "PEOPLE";
```

Note: Views require the struct to have `static constexpr bool is_view = true;`. Optionally, you can specify a custom name using `static constexpr const char* viewname = "VIEW_NAME";` or `static constexpr const char* tablename = "VIEW_NAME";` (both work the same way for views).

## Example: Full Query Composition

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

// Define the source data structure
struct Person {
  sqlgen::PrimaryKey<uint32_t> id;
  std::string first_name;
  std::string last_name;
  int age;
};

// Define the target structure for the new table/view
struct Name {
  std::string first_name;
  std::string last_name;
};

// Create a SELECT query
const auto names_query = select_from<Person>("first_name"_c, "last_name"_c);

// Create table as query
const auto create_table_query = create_as<Name>(names_query) | if_not_exists;

// Execute the query
const auto result = create_table_query(conn);

if (!result) {
    // Error handling
}
```

This generates the following SQL:

```sql
CREATE TABLE IF NOT EXISTS "Name" AS SELECT "first_name", "last_name" FROM "Person";
```

## View Creation

To create views, your struct must be marked as a view:

```cpp
struct Name {
  static constexpr bool is_view = true;
  // Optionally specify a custom name
  static constexpr const char* viewname = "NAMES";  // or tablename = "NAMES"
  
  std::string first_name;
  std::string last_name;
};
```

Then use `create_or_replace_view_as`:

```cpp
const auto names_query = select_from<Person>("first_name"_c, "last_name"_c);
const auto get_names = create_or_replace_view_as<Name>(names_query);

get_names(conn).value();
```

## Using member pointers

The `select_from` queries passed to `create_as` can reference columns with a
member pointer to a struct field through the `col` template instead of the
`"..."_c` column literals. For example:

```cpp
const auto names_query = select_from<Person>(
    col<&Person::first_name>,
    col<&Person::last_name>);
const auto get_names = create_as<Name>(names_query);
```

This generates the following SQL:

```sql
CREATE TABLE "Name" AS SELECT "Person"."first_name", "Person"."last_name" FROM "Person";
```

The `if_not_exists`, view creation, and monadic error handling examples all work
with member pointers in the same way.

## Database-Specific Behavior

### MySQL

MySQL supports both `CREATE TABLE AS` and `CREATE OR REPLACE VIEW AS`:

```sql
-- Table creation
CREATE TABLE IF NOT EXISTS `Name` AS SELECT `first_name`, `last_name` FROM `Person`;

-- View creation  
CREATE OR REPLACE VIEW `NAMES` AS SELECT `first_name`, `last_name` FROM `PEOPLE`;
```

### PostgreSQL

PostgreSQL supports both `CREATE TABLE AS` and `CREATE OR REPLACE VIEW AS`:

```sql
-- Table creation
CREATE TABLE IF NOT EXISTS "Name" AS SELECT "first_name", "last_name" FROM "Person";

-- View creation
CREATE OR REPLACE VIEW "NAMES" AS SELECT "first_name", "last_name" FROM "PEOPLE";
```

### SQLite

SQLite has different view creation behavior:

```sql
-- Table creation
CREATE TABLE IF NOT EXISTS "Name" AS SELECT "first_name", "last_name" FROM "Person";

-- View creation - SQLite does NOT support CREATE OR REPLACE VIEW
-- Instead, it uses CREATE VIEW IF NOT EXISTS
CREATE VIEW IF NOT EXISTS "NAMES" AS SELECT "first_name", "last_name" FROM "PEOPLE";
```

**Important**: SQLite does not support `CREATE OR REPLACE VIEW`. Do not use `create_or_replace_view_as` with SQLite.

## Monadic Error Handling

Like other sqlgen operations, `create_as` supports monadic error handling:

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

const auto names_query = select_from<Person>("first_name"_c, "last_name"_c);
const auto get_names = create_as<Name>(names_query);

// sqlgen::Result<std::vector<Name>>
const auto result = sqlite::connect("database.db")
                       .and_then(drop<Name> | if_exists)
                       .and_then(drop<Person> | if_exists)
                       .and_then(write(std::ref(people)))
                       .and_then(get_names)
                       .and_then(sqlgen::read<std::vector<Name>>);
```

## Notes

- The `if_not_exists` clause is optional - if omitted, the query will fail if the table already exists
- Views require `static constexpr bool is_view = true;` in the struct definition
- Custom names can be specified using either `viewname` or `tablename` - they work identically for tables and views
- The `Result<Ref<Connection>>` type provides error handling; use `.value()` to extract the result (will throw an exception if there's an error) or handle errors as needed
- The table/view name is derived from the struct name (e.g., `Name` becomes `"Name"` in SQL) unless overridden by `tablename` or `viewname`
- The SELECT query must be convertible to the output struct type
- All columns from the SELECT query must have corresponding fields in the target struct
- **SQLite Limitation**: SQLite does not support `CREATE OR REPLACE VIEW` - use `create_as<ViewType>(query) | if_not_exists` for SQLite compatibility
- **MySQL/PostgreSQL**: Both support `CREATE OR REPLACE VIEW` and work with `create_or_replace_view_as`


# `sqlgen::exec`

The `sqlgen::exec` interface provides a way to execute raw SQL statements on a database connection. It's useful when you need to perform database operations that aren't covered by the type-safe interfaces, such as setting database-specific parameters or executing custom SQL statements.

{{< callout type="info" >}}
The examples below use the optional `sqlgen::postgres` backend, which is disabled by default. Enable it by configuring CMake with `-DSQLGEN_POSTGRES=ON` before building.
{{< /callout >}}

## Usage

### Basic Execution

Execute a raw SQL statement:

```cpp
const auto conn = sqlgen::postgres::connect(credentials);

sqlgen::exec(conn, "SET TIME ZONE 'UTC';").value();
```

### Curried Execution

You can also use a curried version of `exec` that takes the SQL statement first and returns a function that accepts the connection. This is useful for chaining operations:

```cpp
using namespace sqlgen;

const auto query = exec("SET TIME ZONE 'UTC';");

// Using with a connection reference
const auto conn = postgres::connect(credentials);
query(conn).value();

// Or in a chain with other operations
postgres::connect(credentials)
    .and_then(exec("SET TIME ZONE 'UTC';"))
    .value();
```

## Example: Full Query Composition

Here's a complete example showing how to use `exec` within a chain of operations:

```cpp
using namespace sqlgen;

const auto result = postgres::connect(credentials)
    .and_then(exec("SET TIME ZONE 'UTC';"))
    .and_then(drop<Person> | if_exists)
    .and_then(write(people))
    .and_then(read<std::vector<Person>>)
    .value();
```

This executes the following SQL statements in sequence:

```sql
SET TIME ZONE 'UTC';
DROP TABLE IF EXISTS "Person";
-- Table creation and data insertion via write
-- Data retrieval via read
```

## Notes

- The `Result<Ref<Connection>>` type provides error handling; use `.value()` to extract the result (will throw an exception if there's an error) or handle errors as needed
- The function has two overloads:
  1. Takes a connection reference and SQL string
  2. Takes a `Result<Ref<Connection>>` and SQL string
- The curried version is particularly useful when you want to:
  - Chain multiple database operations together
  - Pass the execution operation as a function to other operations
- Use `exec` when you need to:
  - Execute database-specific commands
  - Set database parameters
  - Run custom SQL statements not covered by type-safe interfaces
  - Perform administrative tasks
- Exercise caution when using `exec` with user-provided input to prevent SQL injection attacks
- The SQL statement is executed as-is, so ensure it's compatible with your database dialect


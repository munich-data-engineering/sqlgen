# `sqlgen::...::to_sql`

The `to_sql` functions in each dialect namespace (e.g., `sqlgen::postgres::to_sql`, `sqlgen::sqlite::to_sql`) provide a way to transpile C++ SQL operations into dialect-specific SQL strings. This is particularly useful for testing, debugging, logging, and understanding the exact SQL that will be executed by sqlgen.

{{< callout type="info" >}}
The examples below use the optional `sqlgen::postgres` backend, which is disabled by default. Enable it by configuring CMake with `-DSQLGEN_POSTGRES=ON` before building.
{{< /callout >}}

## Usage

### Basic Usage

Transpile any SQL operation to a dialect-specific SQL string:

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

// Define a query
const auto query = sqlgen::read<std::vector<Person>> |
                   where("age"_c < 18 and "first_name"_c != "Hugo") |
                   order_by("age"_c);

// Transpile to PostgreSQL dialect
const auto postgres_sql = postgres::to_sql(query);
```

This generates the following PostgreSQL SQL:

```sql
SELECT "id", "first_name", "last_name", "age" 
FROM "Person" 
WHERE ("age" < 18) AND ("first_name" != 'Hugo') 
ORDER BY "age";
```

Or for SQLite:

```cpp
// Transpile to SQLite dialect
const auto sqlite_sql = sqlite::to_sql(query);
```

### Supported Operations

`to_sql` supports transpilation of all sqlgen operations:

#### SELECT Queries

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

// Basic select
const auto select_query = sqlgen::read<std::vector<Person>>;
const auto sql = postgres::to_sql(select_query);
```

This generates the following SQL:

```sql
SELECT "id", "first_name", "last_name", "age" 
FROM "Person";
```

With a where clause:

```cpp
// With where clause
const auto filtered_query = sqlgen::read<std::vector<Person>> | 
                           where("age"_c >= 18);
const auto sql = postgres::to_sql(filtered_query);
```

This generates the following SQL:

```sql
SELECT "id", "first_name", "last_name", "age" 
FROM "Person" 
WHERE "age" >= 18;
```

#### CREATE TABLE Operations

Note that every call to `sqlgen::write` calls a `CREATE TABLE` operation
before actually inserting the data.

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

// Define a table structure
struct TestTable {
    std::string field1;
    int32_t field2;
    sqlgen::PrimaryKey<uint32_t> id;
    std::optional<std::string> nullable;
};

// Create table query
const auto create_query = create_table<TestTable> | if_not_exists;
const auto sql = postgres::to_sql(create_query);
```

This generates the following PostgreSQL SQL:

```sql
CREATE TABLE IF NOT EXISTS "TestTable" (
    "field1" TEXT NOT NULL,
    "field2" INTEGER NOT NULL,
    "id" INTEGER NOT NULL,
    "nullable" TEXT,
    PRIMARY KEY ("id")
);
```

#### INSERT Operations

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

const auto insert_query = Insert<TestTable>{};
const auto sql = postgres::to_sql(insert_query);
```

This generates the following PostgreSQL SQL:

```sql
COPY "public"."TestTable"("field1", "field2", "id", "nullable") 
FROM STDIN WITH DELIMITER '\t' NULL '\e' QUOTE '\a';
```

For SQLite, the same operation generates:

```sql
INSERT INTO "TestTable"("field1", "field2", "id", "nullable") 
VALUES (?, ?, ?, ?);
```

#### DELETE Operations

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

const auto delete_query = delete_from<TestTable> | 
                         where("field2"_c > 0);
const auto sql = postgres::to_sql(delete_query);
```

This generates the following SQL:

```sql
DELETE FROM "TestTable" 
WHERE "field2" > 0;
```


### Dialect-Specific Features

Each database dialect implements its own version of `to_sql` with dialect-specific features:

#### PostgreSQL (`sqlgen::postgres::to_sql`)

- Uses `COPY` for bulk inserts
- Supports schema names (e.g., "public")
- Uses PostgreSQL-specific escaping rules

#### SQLite (`sqlgen::sqlite::to_sql`)

- Uses `INSERT INTO` for inserts
- Uses SQLite-specific escaping rules
- Simpler schema (no schema names)

## Notes

- Each dialect's `to_sql` is a template function that works with any sqlgen operation
- The functions are `noexcept` and return a `std::string`
- The generated SQL is properly escaped and safe to use
- The functions are particularly useful for:
  - Debugging complex queries
  - Logging SQL operations
  - Understanding the exact SQL that will be executed
  - Testing and validation
- Each dialect's implementation ensures the generated SQL is compatible with that specific database
- The functions preserve all SQL operations' semantics while adapting to dialect-specific syntax


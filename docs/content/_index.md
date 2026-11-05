# sqlgen Documentation

Welcome to the sqlgen documentation. This guide provides detailed information about sqlgen's features and APIs.

![image](sqlgen.png)

## Core Concepts

{{< cards >}}
  {{< card link="install" title="Installation" icon="book-open" subtitle="How to install sqlgen.">}}
  {{< card link="defining_tables" title="Defining Tables" icon="book-open" subtitle="How to define tables using C++ structs.">}}
  {{< card link="col" title="sqlgen::col" icon="book-open" subtitle="How to represent columns in queries.">}}
  {{< card link="literals" title="sqlgen::literals" icon="book-open" subtitle="How to use column and table alias literals in queries.">}}
  {{< card link="flatten" title="sqlgen::Flatten" icon="book-open" subtitle="How to &#34;inherit&#34; fields from other structs.">}}
  {{< card link="primary_key" title="sqlgen::PrimaryKey" icon="book-open" subtitle="How to define primary keys in sqlgen.">}}
  {{< card link="result" title="sqlgen::Result" icon="book-open" subtitle="How sqlgen handles errors and results.">}}
  {{< card link="to_sql" title="sqlgen::to_sql" icon="book-open" subtitle="How to transpile C++ operations to dialect-specific SQL.">}}
{{</ cards >}}

## Database I/O

{{< cards >}}
  {{< card link="reading" title="sqlgen::read" icon="book-open" subtitle="How to read data from a database.">}}
  {{< card link="writing" title="sqlgen::write" icon="book-open" subtitle="How to write data to a database.">}}
{{</ cards >}}

## Database Operations

{{< cards >}}
  {{< card link="create_as" title="sqlgen::create_as" icon="book-open" subtitle="How to create tables and views from SELECT queries.">}}
  {{< card link="create_index" title="sqlgen::create_index" icon="book-open" subtitle="How to create an index on a table.">}}
  {{< card link="create_table" title="sqlgen::create_table" icon="book-open" subtitle="How to create a new table.">}}
  {{< card link="delete_from" title="sqlgen::delete_from" icon="book-open" subtitle="How to delete data from a table.">}}
  {{< card link="drop" title="sqlgen::drop" icon="book-open" subtitle="How to drop a table.">}}
  {{< card link="exec" title="sqlgen::exec" icon="book-open" subtitle="How to execute raw SQL statements.">}}
  {{< card link="group_by_and_aggregations" title="sqlgen::group_by and Aggregations" icon="book-open" subtitle="How to generate GROUP BY queries and aggregate data.">}}
  {{< card link="joins" title="sqlgen::inner_join, left_join, right_join, full_join" icon="book-open" subtitle="How to join different tables.">}}
  {{< card link="insert" title="sqlgen::insert, insert_or_replace" icon="book-open" subtitle="How to insert data within transactions.">}}
  {{< card link="select_from" title="sqlgen::select_from" icon="book-open" subtitle="How to read data from a database using more complex queries.">}}
  {{< card link="unite" title="sqlgen::unite and unite_all" icon="book-open" subtitle="How to combine results from multiple SELECT statements.">}}
  {{< card link="update" title="sqlgen::update" icon="book-open" subtitle="How to update data in a table.">}}
{{</ cards >}}

## Other Operations

{{< cards >}}
  {{< card link="cache" title="Cache" icon="book-open" subtitle="How to improve performance with caching.">}}
  {{< card link="mathematical_operations" title="Mathematical Operations" icon="book-open" subtitle="How to use mathematical functions in queries (e.g., abs, ceil, floor, exp, trigonometric functions, round).">}}
  {{< card link="string_operations" title="String Operations" icon="book-open" subtitle="How to manipulate and transform strings in queries (e.g., length, lower, upper, trim, replace, concat).">}}
  {{< card link="type_conversion_operations" title="Type Conversion Operations" icon="book-open" subtitle="How to convert between types safely in queries (e.g., cast int to double).">}}
  {{< card link="null_handling_operations" title="Null Handling Operations" icon="book-open" subtitle="How to handle nullable values and propagate nullability correctly (e.g., with coalesce and nullability rules).">}}
  {{< card link="timestamp_operations" title="Timestamp and Date/Time Functions" icon="book-open" subtitle="How to work with timestamps, dates, and times (e.g., extract parts, perform arithmetic, convert formats).">}}
  {{< card link="enum" title="Enums" icon="book-open" subtitle="How to work with enums in sqlgen.">}}
{{</ cards >}}

## Data Types and Validation

{{< cards >}}
  {{< card link="dynamic" title="sqlgen::Dynamic" icon="book-open" subtitle="How to define custom SQL types not natively supported by sqlgen.">}}
  {{< card link="foreign_key" title="sqlgen::ForeignKey" icon="book-open" subtitle="How to establish referential integrity between tables.">}}
  {{< card link="json" title="sqlgen::JSON" icon="book-open" subtitle="How to store and work with JSON fields.">}}
  {{< card link="list" title="std::vector" icon="book-open" subtitle="How to store std::vector fields as native lists and arrays (DuckDB, PostgreSQL).">}}
  {{< card link="pattern" title="sqlgen::Pattern" icon="book-open" subtitle="How to add regex pattern validation to avoid SQL injection.">}}
  {{< card link="timestamp" title="sqlgen::Timestamp" icon="book-open" subtitle="How timestamps work in sqlgen.">}}
  {{< card link="unique" title="sqlgen::Unique" icon="book-open" subtitle="How to enforce uniqueness constraints on table columns.">}}
  {{< card link="varchar" title="sqlgen::Varchar" icon="book-open" subtitle="How varchars work in sqlgen.">}}
{{</ cards >}}

## Other concepts

{{< cards >}}
  {{< card link="connection_pool" title="Connection Pool" icon="book-open" subtitle="How to manage database connections efficiently.">}}
  {{< card link="transactions" title="Transactions" icon="book-open" subtitle="How to use transactions for atomic operations.">}}
  {{< card link="views" title="Views" icon="book-open" subtitle="How to create and manage database views.">}}
{{</ cards >}}

## Supported Databases

{{< cards >}}
  {{< card link="duckdb" title="DuckDB" icon="book-open" subtitle="How to interact with DuckDB.">}}
  {{< card link="mysql" title="MySQL" icon="book-open" subtitle="How to interact with MariaDB and MySQL.">}}
  {{< card link="postgres" title="PostgreSQL" icon="book-open" subtitle="How to interact with PostgreSQL and compatible databases (Redshift, Aurora, Greenplum, CockroachDB, ...).">}}
  {{< card link="sqlite" title="SQLite" icon="book-open" subtitle="How to interact with SQLite3.">}}
{{</ cards >}}

For installation instructions, quick start guide, and usage examples, please refer to the [main README](../README.md).

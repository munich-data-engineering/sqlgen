# `sqlgen::duckdb`

The `sqlgen::duckdb` module provides a type-safe and efficient interface for interacting with DuckDB databases. It implements the core database operations through a connection-based API with support for prepared statements, transactions, and efficient data iteration.

## Usage

### Basic Connection

Create a connection to a DuckDB database:

```cpp
// Connect to an in-memory database
const auto conn = sqlgen::duckdb::connect();

// Connect to a file-based database
const auto conn = sqlgen::duckdb::connect("database.db");
```

The type of `conn` is `sqlgen::Result<sqlgen::Ref<sqlgen::duckdb::Connection>>`, which is useful for error handling:

```cpp
// Handle connection errors
const auto conn = sqlgen::duckdb::connect("database.db");
if (!conn) {
    // Handle error...
    return;
}

using namespace sqlgen;
using namespace sqlgen::literals;

const auto query = sqlgen::read<std::vector<Person>> |
                   where("age"_c < 18 and "first_name"_c != "Hugo");

// Use the connection
const auto minors = query(conn);
```

### Basic Operations

Write data to the database:

```cpp
struct Person {
    sqlgen::PrimaryKey<uint32_t> id;
    std::string first_name;
    std::string last_name;
    int age;
};

const auto people = std::vector<Person>{
    Person{.id = 0, .first_name = "Homer", .last_name = "Simpson", .age = 45},
    Person{.id = 1, .first_name = "Bart", .last_name = "Simpson", .age = 10},
    Person{.id = 2, .first_name = "Lisa", .last_name = "Simpson", .age = 8}
};

// Write data to database
const auto result = sqlgen::write(conn, people);
```

Read data with filtering and ordering:

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

// Read all people ordered by age
const auto all_people = sqlgen::read<std::vector<Person>> | 
                       order_by("age"_c);

// Read minors only
const auto minors = sqlgen::read<std::vector<Person>> |
                   where("age"_c < 18) |
                   order_by("age"_c);

// Use the queries
const auto result1 = all_people(conn);
const auto result2 = minors(conn);
```

### Transactions

Perform operations within transactions:

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

// Delete a person and update another in a transaction
const auto delete_hugo = delete_from<Person> | 
                        where("first_name"_c == "Hugo");

const auto update_homer = update<Person>("age"_c.set(46)) | 
                         where("first_name"_c == "Homer");

const auto result = begin_transaction(conn)
                       .and_then(delete_hugo)
                       .and_then(update_homer)
                       .and_then(commit)
                       .value();
```

### Update Operations

Update data in a table:

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

// Update multiple columns
const auto query = update<Person>("first_name"_c.set("last_name"_c), "age"_c.set(100)) |
                  where("first_name"_c == "Hugo");

query(conn).value();
```

## Notes

- The module provides a type-safe interface for DuckDB operations
- All operations return `sqlgen::Result<T>` for error handling
- Prepared statements are used for efficient query execution
- The iterator interface supports batch processing of results
- SQL generation adapts to DuckDB's dialect
- The module supports:
  - In-memory and file-based databases
  - Transactions (begin, commit, rollback)
  - Efficient batch operations
  - Type-safe SQL generation
  - Error handling through `Result<T>`
  - Resource management through `Ref<T>`
  - Auto-incrementing primary keys
  - Various data types including VARCHAR, TIMESTAMP, DATE
  - Complex queries with WHERE clauses, ORDER BY, LIMIT, OFFSET, JOINs
  - LIKE and pattern matching operations
  - Mathematical operations and string functions
  - JSON data types
  - Foreign keys and referential integrity
  - Unique constraints
  - Views and materialized views
  - Indexes
```
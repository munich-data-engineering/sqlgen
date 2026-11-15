# `sqlgen::mysql`

The `sqlgen::mysql` module provides a type-safe and efficient interface for interacting with MySQL and MariaDB databases. It implements the core database operations through a connection-based API with support for prepared statements, transactions, and efficient data iteration.

{{< callout type="info" >}}
The `sqlgen::mysql` backend is optional and disabled by default. Enable it by configuring CMake with `-DSQLGEN_MYSQL=ON` before building.
{{< /callout >}}

## Usage

### Basic Connection

Create a connection to a MySQL/MariaDB database using credentials:

```cpp
// Create credentials for the database connection
const auto creds = sqlgen::mysql::Credentials{
                        .host = "localhost",
                        .user = "myuser",
                        .password = "mypassword",
                        .dbname = "mydatabase",
                        .port = 3306,  // Optional, defaults to 3306
                        .unix_socket = "/var/run/mysqld/mysqld.sock"  // Optional, defaults to "/var/run/mysqld/mysqld.sock"
                    };

// Connect to the database
const auto conn = sqlgen::mysql::connect(creds);
```

The type of `conn` is `sqlgen::Result<sqlgen::Ref<sqlgen::mysql::Connection>>`, which is useful for error handling:

```cpp
// Handle connection errors
const auto conn = sqlgen::mysql::connect(creds);
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

### Advanced Queries

Use complex queries with joins and projections:

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

// Select specific columns with aliases
struct FirstName {
    std::string first_name;
};

const auto names = select_from<Person>("first_name"_c) | 
                  order_by("id"_c) |
                  to<std::vector<FirstName>>;

// Join operations
const auto joined_data = select_from<Person, "t1">(
    "id"_t1 | as<"id">, 
    "first_name"_t1 | as<"first_name">,
    "last_name"_t2 | as<"last_name">, 
    "age"_t2 | as<"age">) |
    inner_join<Person, "t2">("id"_t1 == "id"_t2) |
    order_by("id"_t1) |
    to<std::vector<Person>>;
```

### Connection Pools

Use connection pools for efficient resource management:

```cpp
const auto pool_config = sqlgen::ConnectionPoolConfig{.size = 2};

const auto pool = sqlgen::make_connection_pool<sqlgen::mysql::Connection>(
    pool_config, credentials);

// Use the pool in a session
const auto result = session(pool)
                       .and_then(write(std::ref(people)))
                       .and_then(sqlgen::read<std::vector<Person>>)
                       .value();
```

## Notes

- The module provides a type-safe interface for MySQL/MariaDB operations
- All operations return `sqlgen::Result<T>` for error handling
- Prepared statements are used for efficient query execution
- The iterator interface supports batch processing of results
- SQL generation adapts to MySQL's dialect
- The module supports:
  - Connection management with credentials (host, port, database name, unix socket)
  - Transactions (begin, commit, rollback)
  - Prepared statements for efficient execution
  - Efficient batch operations
  - Type-safe SQL generation
  - Error handling through `Result<T>`
  - Resource management through `Ref<T>`
  - Connection pooling for high-performance applications
  - Auto-incrementing primary keys
  - Various data types including VARCHAR, TIMESTAMP, DATE
  - Complex queries with WHERE clauses, ORDER BY, LIMIT, OFFSET, JOINs
  - LIKE and pattern matching operations
  - Mathematical operations and string functions

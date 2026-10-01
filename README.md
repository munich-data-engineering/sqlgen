# ![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white) sqlgen

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Maintenance](https://img.shields.io/badge/Maintained%3F-yes-green.svg)](https://github.com/getml/reflect-cpp/graphs/commit-activity)
[![Generic badge](https://img.shields.io/badge/C++-20-blue.svg)](https://shields.io/)
[![Generic badge](https://img.shields.io/badge/gcc-11+-blue.svg)](https://shields.io/)
[![Generic badge](https://img.shields.io/badge/clang-14+-blue.svg)](https://shields.io/)
[![Generic badge](https://img.shields.io/badge/MSVC-17+-blue.svg)](https://shields.io/)
[![Conan Center](https://img.shields.io/conan/v/sqlgen)](https://conan.io/center/recipes/sqlgen)

**📖 Documentation**: [Click here](https://munich-data-engineering.github.io/sqlgen/)

![image](docs/content/sqlgen.png)

**sqlgen** is a modern, type-safe ORM and SQL query generator for C++20, inspired by Python's [SQLAlchemy](https://github.com/sqlalchemy/sqlalchemy)/[SQLModel](https://github.com/fastapi/sqlmodel) and Rust's [Diesel](https://github.com/diesel-rs/diesel). It provides a fluent, composable interface for database operations with compile-time type checking and SQL injection protection.

sqlgen is based on and tightly integrated with [reflect-cpp](https://github.com/getml/reflect-cpp), a C++-20 library for fast serialization, deserialization and validation using reflection, similar to pydantic in Python, serde in Rust, encoding in Go or aeson in Haskell.

Together, reflect-cpp and sqlgen enable reliable and efficient ETL pipelines.

## Features

- 🔒 **Type Safety**: Compile-time validation of table schemas and queries
- 🛡️ **SQL Injection Protection**: Built-in input validation and parameterized queries
- 🔄 **Composable Queries**: Fluent interface for building complex queries
- 🚀 **High Performance**: Efficient batch operations and prepared statements
- 📦 **Modern C++**: Leverages C++20 features for a clean, expressive API
- 🔌 **Multiple Backends**: Support for PostgreSQL and SQLite
- 🔍 **Reflection Integration**: Seamless integration with [reflect-cpp](https://github.com/getml/reflect-cpp)

## Supported databases

The following table lists the databases currently supported by sqlgen and the underlying libraries used:

| Database      | Library                                                                  | Version      | License       | Remarks                                              |
|---------------|--------------------------------------------------------------------------|--------------|---------------| -----------------------------------------------------|
| DuckDB        | [duckdb](https://github.com/duckdb/duckdb)                               | >= 1.4.2     | MIT           |                                                      |
| MySQL/MariaDB | [libmariadb](https://github.com/mariadb-corporation/mariadb-connector-c) | >= 3.4.5     | LGPL          |                                                      |
| PostgreSQL    | [libpq](https://github.com/postgres/postgres)                            | >= 16.4      | PostgreSQL    | Will work for all libpq-compatible databases         |
| sqlite        | [sqlite](https://sqlite.org/index.html)                                  | >= 3.49.1    | Public Domain |                                                      |

## Quick Start

### Install using vcpkg or Conan

You can install the latest release of sqlgen
using either [vcpkg](https://vcpkg.io/en/package/sqlgen) or [Conan](https://conan.io/center/recipes/sqlgen).

### Build using vcpkg

Alternatively, you can build sqlgen from source using vcpkg:

1. Make sure you have the required dependencies installed (skip this step on Windows):
```bash
sudo apt-get install autoconf bison flex # Linux (Ubuntu, Debian, ...)
brew install autoconf bison flex # macOS
```

2. Set up vcpkg:
```bash
git submodule update --init
./vcpkg/bootstrap-vcpkg.sh  # Linux, macOS
./vcpkg/bootstrap-vcpkg.bat # Windows
```

3. Build the library:
```bash
cmake -S . -B build -DCMAKE_CXX_STANDARD=20 -DCMAKE_BUILD_TYPE=Release
cmake --build build -j 4  # gcc, clang
cmake --build build --config Release -j 4  # MSVC
```

This will build the static library. To build the shared library
add `-DBUILD_SHARED_LIBS=ON -DVCPKG_TARGET_TRIPLET=...` to the first line.
Run `./vcpkg/vcpkg help triplets` to view all supported triplets.
Common triplets for shared libraries are `x64-linux-dynamic`,
`arm64-osx-dynamic` or `x64-osx-dynamic`.

Add `-DSQLGEN_MYSQL=ON` to support MySQL/MariaDB. Add `-DSQLGEN_DUCKDB=ON` to support DuckDB.

4. Include in your CMake project:
```cmake
find_package(sqlgen REQUIRED)
target_link_libraries(your_target PRIVATE sqlgen::sqlgen)
```

### Build using Conan

You can also build sqlgen from source using Conan:

1. Install Conan (assuming you have Python and pipx installed):

```bash
pipx install conan
conan profile detect
```

For older versions of pip, you can also use `pip` instead of `pipx`.

2. Build the library:

```bash
conan build . --build=missing -s compiler.cppstd=gnu20
```

This will build the static library. To build the shared library,
add `-o */*:shared=True`.

Add `-o sqlgen/*:with_mysql=True` to support MySQL/MariaDB.

3. Include in your CMake project:
```cmake
find_package(sqlgen REQUIRED)
target_link_libraries(your_target PRIVATE sqlgen::sqlgen)
```

You can call `conan inspect .` to get an overview of the supported options.

## Usage Examples

### Hello World

```cpp
#include <sqlgen/sqlite.hpp>
#include <iostream>

struct User {
    std::string name;
    int age;
};

int main() {
    // Connect to SQLite database
    const auto conn = sqlgen::sqlite::connect("test.db");

    // Create and insert a user
    const auto user = User{.name = "John", .age = 30};
    sqlgen::write(conn, user);

    // Read all users
    const auto users = sqlgen::read<std::vector<User>>(conn).value();

    for (const auto& u : users) {
        std::cout << u.name << " is " << u.age << " years old\n";
    }
}
```

### Connecting to a Database

```cpp
#include <sqlgen/postgres.hpp>

// PostgreSQL connection
const auto credentials = sqlgen::postgres::Credentials{
    .user = "username",
    .password = "password",
    .host = "localhost",
    .dbname = "mydb",
    .port = 5432
};

const auto conn = sqlgen::postgres::connect(credentials);

// SQLite connection
const auto sqlite_conn = sqlgen::sqlite::connect("database.db");
```

### Defining Models

```cpp
struct Person {
    std::string first_name;
    std::string last_name;
    uint32_t age;
    std::optional<std::string> email;  // Nullable field
};
```

### Inserting Data

```cpp
const auto people = std::vector<Person>({
    Person{.first_name = "Homer", .last_name = "Simpson", .age = 45},
    Person{.first_name = "Marge", .last_name = "Simpson", .age = 42}
});

// Automatically creates table if it doesn't exist
// (recall that the table is fully defined at compile time)
const auto result = sqlgen::write(conn, people);

if (!result) {
    std::cerr << "Error: " << result.error().what() << std::endl;
}
```

Or:

```cpp
...

// write(...) abstracts these steps away, but
// if you prefer more granular control, you
// can use sqlgen::insert.
const auto result = begin_transaction(conn)
                        .and_then(create_table<Person> | if_not_exists)
                        .and_then(insert(std::ref(people)))
                        .and_then(commit);

...
```

Generated SQL:
```sql
BEGIN TRANSACTION;
CREATE TABLE IF NOT EXISTS "Person" (
    "first_name" TEXT NOT NULL,
    "last_name" TEXT NOT NULL,
    "age" INTEGER NOT NULL,
    "email" TEXT
);

INSERT INTO "Person" ("first_name", "last_name", "age", "email")
VALUES (?, ?, ?, ?);
COMMIT;
```

### Querying Data

```cpp
#include <rfl/json.hpp>
#include <sqlgen/postgres.hpp>

using namespace sqlgen;
using namespace sqlgen::literals;

// Build a query for adults, ordered by age
const auto query = read<std::vector<Person>> |
                   where("age"_c >= 18) |
                   order_by("age"_c.desc(), "last_name"_c) |
                   limit(10);

// Execute the query
const auto result = query(conn);

if (result) {
    // Print results as JSON
    std::cout << rfl::json::write(*result) << std::endl;
} else {
    std::cerr << "Error: " << result.error().what() << std::endl;
}
```

Generated SQL:
```sql
SELECT "first_name", "last_name", "age", "email"
FROM "Person"
WHERE "age" >= 18
ORDER BY "age" DESC, "last_name"
LIMIT 10;
```

### Grouping and Aggregating Data

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

struct Children {
    std::string last_name;
    int num_children;
    int max_age;
    int min_age;
    int sum_age;
};

const auto get_children = select_from<Person>(
    "last_name"_c,
    count().as<"num_children">(),
    max("age"_c).as<"max_age">(),
    min("age"_c).as<"min_age">(),
    sum("age"_c).as<"sum_age">(),
) | where("age"_c < 18) | group_by("last_name"_c) | to<std::vector<Children>>;

const std::vector<Children> children = get_children(conn).value();
```

Generated SQL:
```sql
SELECT
    "last_name",
    COUNT(*) as "num_children",
    MAX("age") as "max_age",
    MIN("age") as "min_age",
    SUM("age") as "sum_age"
FROM "Person"
WHERE "age" < 18
GROUP BY "last_name";
```

### Joining data

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

struct ParentAndChild {
    std::string last_name;
    std::string first_name_parent;
    std::string first_name_child;
    double parent_age_at_birth;
};

const auto get_people =
  select_from<Person, "t1">(
      "last_name"_t1 | as<"last_name">,
      "first_name"_t1 | as<"first_name_parent">,
      "first_name"_t3 | as<"first_name_child">,
      ("age"_t1 - "age"_t3) | as<"parent_age_at_birth">) |
  inner_join<Relationship, "t2">("id"_t1 == "parent_id"_t2) |
  inner_join<Person, "t3">("id"_t3 == "child_id"_t2) |
  order_by("id"_t1, "id"_t3) | to<std::vector<ParentAndChild>>;
```

Generated SQL:
```sql
SELECT t1."last_name" AS "last_name",
       t1."first_name" AS "first_name_parent",
       t3."first_name" AS "first_name_child",
       t1."age" - t3."age" AS "parent_age_at_birth"
FROM "Person" t1
INNER JOIN "Relationship" t2
ON t1."id" = t2."parent_id"
INNER JOIN "Person" t3
ON t3."id" = t2."child_id"
ORDER BY t1."id", t3."id"
```

### Nested joins

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

struct ParentAndChild {
    std::string last_name;
    std::string first_name_parent;
    std::string first_name_child;
    double parent_age_at_birth;
};

// First, create a subquery
const auto get_children =
  select_from<Relationship, "t1">("parent_id"_t1 | as<"id">,
                                  "first_name"_t2 | as<"first_name">,
                                  "age"_t2 | as<"age">) |
  inner_join<Person, "t2">("id"_t2 == "child_id"_t1);

// Then use it as a source for another query
const auto get_people =
  select_from<Person, "t1">(
      "last_name"_t1 | as<"last_name">,
      "first_name"_t1 | as<"first_name_parent">,
      "first_name"_t2 | as<"first_name_child">,
      ("age"_t1 - "age"_t2) | as<"parent_age_at_birth">) |
  inner_join<"t2">(
    get_children, // Use the subquery as the source
    "id"_t1 == "id"_t2) |
  order_by("id"_t1, "id"_t2) | to<std::vector<ParentAndChild>>;
```

Generated SQL:
```sql
SELECT t1."last_name" AS "last_name",
    t1."first_name" AS "first_name_parent",
    t2."first_name" AS "first_name_child",
    t1."age" - t2."age" AS "parent_age_at_birth"
FROM "Person" t1
INNER JOIN (
    SELECT t1."parent_id" AS "id",
           t2."first_name" AS "first_name",
           t2."age" AS "age"
    FROM "Relationship" t1
    INNER JOIN "Person" t2
    ON t2."id" = t1."child_id"
) t2
ON t1."id" = t2."id"
ORDER BY t1."id", t2."id"
```

Or:
```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

struct ParentAndChild {
    std::string last_name;
    std::string first_name_parent;
    std::string first_name_child;
    double parent_age_at_birth;
};

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
    ("age"_t1 - "age"_t2) | as<"parent_age_at_birth">) |
 inner_join<Person, "t2">("id"_t1 == "id"_t2) |
 order_by("id"_t1, "id"_t2) | to<std::vector<ParentAndChild>>;
```

Generated SQL:
```sql
SELECT t1."last_name" AS "last_name",
       t1."first_name" AS "first_name_parent",
       t2."first_name" AS "first_name_child",
       (t1."age") - (t2."age") AS "parent_age_at_birth"
FROM (
    SELECT t2."child_id" AS "id",
           t1."first_name" AS "first_name",
           t1."last_name" AS "last_name",
           t1."age" AS "age"
    FROM "Person" t1
    INNER JOIN "Relationship" t2
    ON t1."id" = t2."parent_id"
) t1
INNER JOIN "Person" t2
ON t1."id" = t2."id"
ORDER BY t1."id", t2."id"
```

### Type Safety and SQL Injection Protection

sqlgen provides comprehensive compile-time checks and runtime protection:

```cpp
// Compile-time error: No such column "color"
const auto query = read<std::vector<Person>> |
                   where("color"_c == "blue");

// Compile-time error: Cannot compare column "age" to a string
const auto query = read<std::vector<Person>> |
                   where("age"_c == "Homer");

// Compile-time error: "age" must be aggregated or included in GROUP BY
const auto query = select_from<Person>(
    "last_name"_c,
    "age"_c
) | group_by("last_name"_c);

// Compile-time error: Cannot add string and int
const auto query = select_from<Person>(
    "last_name"_c + "age"_c
);

// Runtime protection against SQL injection
std::vector<Person> get_people(const auto& conn,
                              const sqlgen::AlphaNumeric& first_name) {
    using namespace sqlgen;
    return (read<std::vector<Person>> |
            where("first_name"_c == first_name))(conn).value();
}

// This will be rejected
get_people(conn, "Homer' OR '1'='1");  // SQL injection attempt
```

## Documentation

For detailed documentation, visit our [documentation page](https://munich-data-engineering.github.io/sqlgen/).

## Contributing

We welcome constructive criticism, feature requests and contributions! Please open an issue or a pull request.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

# `sqlgen::postgres`

The `sqlgen::postgres` module provides a type-safe and efficient interface for interacting with PostgreSQL databases. It implements the core database operations through a connection-based API with support for prepared statements, transactions, and efficient data iteration.

{{< callout type="info" >}}
The `sqlgen::postgres` backend is optional and disabled by default. Enable it by configuring CMake with `-DSQLGEN_POSTGRES=ON` before building.
{{< /callout >}}

## Basic Usage

This section describes the key aspects needed in order to use the module.

### Connection

Create a connection to a PostgreSQL database using credentials:

```cpp
// Create credentials for the database connection
const auto creds = sqlgen::postgres::Credentials{
                        .user = "myuser",
                        .password = "mypassword",
                        .host = "localhost",
                        .dbname = "mydatabase",
                        .port = 5432  // Optional, defaults to 5432
                    };

// Connect to the database
const auto conn = sqlgen::postgres::connect(creds);
```

The type of `conn` is `sqlgen::Result<sqlgen::Ref<sqlgen::postgres::Connection>>`, which is useful for error handling:

```cpp
// Handle connection errors
const auto conn = sqlgen::postgres::connect(creds);
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

### Notes

- The module provides a type-safe interface for PostgreSQL operations
- All operations return `sqlgen::Result<T>` for error handling
- The COPY operator is used under-the-hood for efficient data insertion
- The iterator interface supports batch processing of results
- SQL generation adapts to PostgreSQL's dialect
- The module supports:
  - Connection management with credentials
  - Transactions
  - Prepared statements
  - Efficient batch operations
  - Type-safe SQL generation
  - Error handling through `Result<T>`
  - Resource management through `Ref<T>`
  - Customizable connection parameters (host, port, database name, etc.)
  - LISTEN/NOTIFY for real-time event notifications

# Features

This section describes more advanced aspects of the `sqlgen::postgres` module, which may not be necessary for a typical user.

## LISTEN/NOTIFY

PostgreSQL provides a simple publish-subscribe mechanism through `LISTEN` and `NOTIFY` commands. This allows database clients to receive real-time notifications when events occur, without polling. Any client can send a notification to a channel, and all clients listening on that channel will receive it asynchronously.

> **Note:** You should use a dedicated connection for LISTEN/NOTIFY, separate from your main database activity and outside any connection pool. This is because the listening connection must remain open and persistent to receive notifications, and connection pools typically recycle connections which would lose the LISTEN state.

### API

The `Connection` class provides the following methods for listen/notify:

```cpp
// Subscribe to a notification channel
rfl::Result<Nothing> listen(const std::string& channel) noexcept;

// Unsubscribe from a notification channel
rfl::Result<Nothing> unlisten(const std::string& channel) noexcept;

// Send a notification to a channel with an optional payload
rfl::Result<Nothing> notify(const std::string& channel,
                            const std::string& payload = "") noexcept;

// Consume input from the server (must be called before get_notifications)
bool consume_input() noexcept;

// Retrieve all pending notifications
std::list<Notification> get_notifications() noexcept;
```

The `Notification` struct contains:

```cpp
struct Notification {
    std::string channel;   // The channel name
    std::string payload;   // The notification payload (may be empty)
    int backend_pid;       // The PID of the notifying backend
};
```

### Subscribing to Channels

```cpp
auto conn = sqlgen::postgres::connect(creds);
if (!conn) {
    // Handle error...
    return;
}

// Subscribe to a channel
auto result = (*conn)->listen("my_channel");
if (!result) {
    // Handle error...
}
```

### Receiving Notifications

To receive notifications, you must periodically call `consume_input()` to read data from the server, then `get_notifications()` to retrieve any pending notifications:

```cpp
while (running) {
    // Consume any available input from the server
    if (!(*conn)->consume_input()) {
        // Connection error
        break;
    }

    // Process any pending notifications
    auto notifications = (*conn)->get_notifications();
    for (const auto& notification : notifications) {
        // Handle the notification
        std::cout << "Channel: " << notification.channel
                  << " Payload: " << notification.payload << std::endl;
    }

    // Sleep briefly before checking again
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
```

### Sending Notifications

```cpp
// Send a notification with a payload
auto result = (*conn)->notify("my_channel", "event data here");
if (!result) {
    // Handle error...
}
```
## Notice Processor

PostgreSQL functions can emit NOTICE messages using `RAISE NOTICE` in PL/pgSQL. By default, libpq prints these to stderr. sqlgen allows you to capture these messages by providing a custom notice handler in the connection credentials.

### Setting Up a Notice Handler

```cpp
#include <sqlgen/postgres.hpp>
#include <iostream>

// Create credentials with a notice handler
const auto creds = sqlgen::postgres::Credentials{
    .user = "myuser",
    .password = "mypassword",
    .host = "localhost",
    .dbname = "mydatabase",
    .notice_handler = [](const char* msg) {
        std::cout << "[PostgreSQL Notice] " << msg;
    }
};

auto conn = sqlgen::postgres::connect(creds);
```

### Integration with Logging Frameworks

The notice handler can forward messages to your preferred logging framework:

```cpp
#include <spdlog/spdlog.h>

const auto creds = sqlgen::postgres::Credentials{
    .user = "myuser",
    .password = "mypassword",
    .host = "localhost",
    .dbname = "mydatabase",
    .notice_handler = [](const char* msg) {
        // Remove trailing newline if present
        std::string_view sv(msg);
        if (!sv.empty() && sv.back() == '\n') {
            sv.remove_suffix(1);
        }
        spdlog::info("PostgreSQL: {}", sv);
    }
};
```

### Using with Connection Pools

The notice handler is set per-connection, so all connections in a pool will use the same handler:

```cpp
const auto creds = sqlgen::postgres::Credentials{
    .user = "myuser",
    .password = "mypassword",
    .host = "localhost",
    .dbname = "mydatabase",
    .notice_handler = [](const char* msg) {
        my_logger::log(msg);
    }
};

auto pool = sqlgen::make_connection_pool<sqlgen::postgres::Connection>(
    sqlgen::ConnectionPoolConfig{.size = 4},
    creds);
```

### Notes

- If no `notice_handler` is provided, libpq's default behavior (printing to stderr) is used
- The handler receives the full message including any trailing newline
- The handler should be thread-safe when used with connection pools, as multiple connections may invoke it concurrently

## Parameterized Queries

The `execute` method supports parameterized queries using PostgreSQL's `$1, $2, ...` placeholder syntax. This prevents SQL injection and allows safe execution of dynamic queries without needing to define custom types.

*Note*: using parameterized queries in this manner is highly discouraged within `sqlgen`, and should be used only as a last resort. You should consider first using the type-safe API. However, there are cases where this is useful such as when calling stored procedures that do not return results.

### Basic Usage

```cpp
auto conn = sqlgen::postgres::connect(creds);
if (!conn) {
    // Handle error...
    return;
}

// Call a stored function with parameters
auto result = (*conn)->execute(
    "SELECT provision_tenant($1, $2)",
    tenant_id,
    user_email
);
```

### Supported Parameter Types

The following types are automatically converted to SQL parameters:

- `std::string` - passed as-is
- `const char*` / `char*` - converted to string (nullptr becomes NULL)
- Numeric types (`int`, `long`, `double`, etc.) - converted via `std::to_string`
- `bool` - converted to `"true"` or `"false"`
- `std::optional<T>` - value or NULL if `std::nullopt`
- `std::nullopt` / `nullptr` - NULL value

### Handling NULL Values

Use `std::optional` or `std::nullopt` to pass NULL values:

```cpp
std::optional<std::string> maybe_value = std::nullopt;
auto result = (*conn)->execute(
    "INSERT INTO data (nullable_field) VALUES ($1)",
    maybe_value
);
```

### Notes

- Parameters are sent in text format and type inference is handled by PostgreSQL
- This feature uses `PQexecParams` internally for safe parameter binding
- The original `execute(sql)` overload without parameters remains available

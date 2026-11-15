# Connection Pool

sqlgen provides a thread-safe connection pool implementation that efficiently manages database connections. The pool automatically handles connection lifecycle, ensures thread safety, and provides RAII-based session management.

{{< callout type="info" >}}
The examples below use the optional `sqlgen::postgres` backend, which is disabled by default. Enable it by configuring CMake with `-DSQLGEN_POSTGRES=ON` before building.
{{< /callout >}}

## Configuration

The connection pool can be configured using `ConnectionPoolConfig`:

```cpp
using namespace sqlgen;

ConnectionPoolConfig config{
    .size = 4,                    // Number of connections in the pool
    .num_attempts = 10,          // Number of retry attempts when acquiring a connection
    .wait_time_in_seconds = 1    // Wait time between retry attempts
};

// Create a pool with the specified configuration
const auto pool = make_connection_pool<postgres::Connection>(
    config,
    postgres::Credentials{
        .user = "postgres",
        .password = "password",
        .host = "localhost",
        .dbname = "postgres"
    }
);
```

### Configuration Parameters

- `size`: The number of connections to maintain in the pool
- `num_attempts`: Maximum number of attempts to acquire a connection before giving up
- `wait_time_in_seconds`: Time to wait between retry attempts when no connections are available

## Basic Usage

### Creating a Connection Pool

Create a connection pool with a specified number of connections:

```cpp
using namespace sqlgen;

const auto pool = make_connection_pool<postgres::Connection>(
    config,  // ConnectionPoolConfig
    credentials // Variables necessary to create the connections
);

if (!pool) {
    // Handle error
    std::cerr << "Failed to create pool: " << pool.error() << std::endl;
    return;
}
```

### Acquiring Sessions

Sessions are acquired from the pool using the `session()` function:

```cpp
using namespace sqlgen;

// Acquire a session from the pool
const auto session_result = session(pool);

if (!session_result) {
    // Handle error (e.g., no available connections)
    std::cerr << "Failed to acquire session: " << session_result.error() << std::endl;
    return;
}
```

### Chaining Operations

Sessions can be used to chain database operations:

```cpp
using namespace sqlgen;

const auto result = session(pool)
    .and_then(drop<Person> | if_exists)
    .and_then(write(std::ref(people)))
    .and_then(read<std::vector<Person>>);

if (!result) {
    // Handle error
    std::cerr << "Operation failed: " << result.error() << std::endl;
    return;
}

const auto& people = result.value();
```

## Thread Safety

The connection pool is designed to be thread-safe:

- Each connection is protected by an atomic flag
- Sessions are automatically released when they go out of scope
- The pool uses atomic operations for connection management
- Multiple threads can safely acquire and release sessions

Example of thread-safe usage with monadic style:

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

// Create pool
const auto pool = make_connection_pool<postgres::Connection>(config, credentials);


std::vector<std::thread> threads;
for (int i = 0; i < 4; ++i) {
    threads.emplace_back([&]() {
        session(pool)
            .and_then(update<Person>("age"_c.set(46) | where("id"_c == i)));
    });
}

for (auto& thread : threads) {
    thread.join();
}
```

## Session Management

Sessions are managed through RAII (Resource Acquisition Is Initialization) and support monadic operations:

- Sessions automatically release their connections when destroyed
- Connections are returned to the pool when sessions go out of scope
- No explicit connection release is required

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

// Using monadic style for session management with exec
session(pool)
    .and_then(begin_transaction)
    .and_then(update<Person>("age"_c.set(46)))
    .and_then(commit);

```

## Pool Statistics

The connection pool provides methods to monitor its state:

```cpp
using namespace sqlgen;

const auto pool = make_connection_pool<postgres::Connection>(config, credentials);

// Get total number of connections
const size_t total_connections = pool.value().size();  // Returns 4

// Get number of available connections
const size_t available_connections = pool.value().available();  // Returns 4 initially
```

## Connection Acquisition

The pool implements a retry mechanism when acquiring connections:

- If no connection is available, the pool will retry up to `num_attempts` times
- Between each attempt, it waits for `wait_time_in_seconds`
- If all attempts fail, an error is returned
- This helps handle temporary connection unavailability

Example of handling connection acquisition with retries:

```cpp
using namespace sqlgen;

// Configure pool with retry behavior
ConnectionPoolConfig config{
    .size = 4,
    .num_attempts = 5,
    .wait_time_in_seconds = 2
};

const auto pool = make_connection_pool<postgres::Connection>(config, credentials);

// Acquire a session - will retry if no connections are available
const auto session_result = session(pool);
```

## Best Practices

1. **Pool Size**: Choose an appropriate pool size based on your application's needs:
   - Too small: May cause connection wait times
   - Too large: May waste resources
   - Rule of thumb: Start with (2 * number of CPU cores)

2. **Retry Configuration**: Configure retry behavior based on your use case:
   - For high-availability systems: Use more retry attempts with shorter wait times
   - For resource-constrained systems: Use fewer retries with longer wait times
   - Consider your application's latency requirements when setting wait times

3. **Session Lifetime**: Keep sessions as short as possible:
   ```cpp
   // Good: Session is released immediately after use
   session(pool).and_then(execute_query).value();

   // Bad: Session is held longer than necessary
   const auto sess = session(pool).value();
   // ... other operations ...
   sess.execute_query();
   ```

4. **Transaction Management**: Use transactions within sessions for atomic operations:
   ```cpp
   session(pool)
       .and_then(begin_transaction)
       .and_then(update<Person>("age"_c.set(46)))
       .and_then(commit)
       .value();
   ```

## Notes

- The connection pool is template-based and works with any sqlgen connection type
- Sessions are move-only types (cannot be copied)
- The pool automatically cleans up connections when destroyed
- All operations return `Result` types for error handling
- The pool is designed to be efficient and minimize contention
- Connection acquisition is non-blocking (returns error if no connections available) 

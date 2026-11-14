# Timestamp and Date/Time Functions

The `sqlgen` library provides a comprehensive set of operations for working with timestamps, dates, and times. These functions allow you to extract date/time parts, perform arithmetic, and convert between types in a type-safe and composable way. All timestamp operations work with the `sqlgen::Timestamp<...>` type (including the provided aliases `Date` and `DateTime`).

### Timestamp Types

- `Timestamp<format>`: Represents a timestamp with a custom format (see `sqlgen::Timestamp`).
- `Date`: Alias for `Timestamp<"%Y-%m-%d">` (date only).
- `DateTime`: Alias for `Timestamp<"%Y-%m-%d %H:%M:%S">` (date and time).

---

### Date/Time Extraction Functions

These functions extract parts of a timestamp or date:

- `year(ts)`: Extracts the year as an integer.
- `month(ts)`: Extracts the month (1-12).
- `day(ts)`: Extracts the day of the month (1-31).
- `hour(ts)`: Extracts the hour (0-23).
- `minute(ts)`: Extracts the minute (0-59).
- `second(ts)`: Extracts the second (0-59).
- `weekday(ts)`: Extracts the day of the week (0 = Sunday for SQLite, 0 = Monday for Postgres).

```cpp
year("birthday"_c) | as<"year">
month("birthday"_c) | as<"month">
day("birthday"_c) | as<"day">
hour("birthday"_c) | as<"hour">
minute("birthday"_c) | as<"minute">
second("birthday"_c) | as<"second">
weekday("birthday"_c) | as<"weekday">
```

---

### Timestamp Arithmetic

You can add or subtract durations (e.g., days, years, weeks, milliseconds) to/from timestamp columns using standard C++ chrono types:

```cpp
("birthday"_c + std::chrono::days(10)) | as<"birthday_plus_10">
("birthday"_c + std::chrono::years(1) - std::chrono::weeks(2)) | as<"birthday_shifted">
```

This is translated to the appropriate SQL for the backend (e.g., `datetime(..., '+10 days')` for SQLite, `+ INTERVAL '10 days'` for Postgres).

---

### Days Between

Calculates the number of days between two timestamps or dates:

```cpp
days_between("birthday"_c, Date("2011-01-01")) | as<"age_in_days">
```

---

### Unix Epoch Conversion

Converts a timestamp to the number of seconds since the Unix epoch:

```cpp
unixepoch("birthday"_c) | as<"birthday_unixepoch">
unixepoch("birthday"_c + std::chrono::days(10)) | as<"birthday_unixepoch_plus_10">
```

---

### Timestamp Construction and Conversion

You can construct or cast timestamps from strings or from extracted parts:

```cpp
cast<Date>(concat(cast<std::string>(year("birthday"_c)), "-", cast<std::string>(month("birthday"_c)), "-", cast<std::string>(day("birthday"_c)))) | as<"birthday_recreated">
```

---

### Example: Working with Timestamps

```cpp
struct Birthday {
    Date birthday;
    Date birthday_recreated;
    time_t birthday_unixepoch;
    double age_in_days;
    int hour;
    int minute;
    int second;
    int weekday;
};

const auto get_birthdays = select_from<Person>(
    ("birthday"_c + std::chrono::days(10)) | as<"birthday">,
    (cast<Date>(concat(cast<std::string>(year("birthday"_c)), "-",
                        cast<std::string>(month("birthday"_c)), "-",
                        cast<std::string>(day("birthday"_c))))) | as<"birthday_recreated">,
    days_between("birthday"_c, Date("2011-01-01")) | as<"age_in_days">,
    unixepoch("birthday"_c + std::chrono::days(10)) | as<"birthday_unixepoch">,
    hour("birthday"_c) | as<"hour">,
    minute("birthday"_c) | as<"minute">,
    second("birthday"_c) | as<"second">,
    weekday("birthday"_c) | as<"weekday">
) | order_by("id"_c) | to<std::vector<Birthday>>;
```

This generates SQL like:

**SQLite:**
```sql
SELECT datetime("birthday", '+10 days') AS "birthday", ...
```
**Postgres:**
```sql
SELECT "birthday" + INTERVAL '10 days' AS "birthday", ...
```

### Using member pointers

Every column literal above (`"birthday"_c`, `"id"_c`, …) can be replaced with a
member pointer to the corresponding struct field through the `col` template, e.g.
`col<&Person::birthday>`. The example above rewritten with member pointers:

```cpp
const auto get_birthdays = select_from<Person>(
    (col<&Person::birthday> + std::chrono::days(10)) | as<"birthday">,
    (cast<Date>(concat(cast<std::string>(year(col<&Person::birthday>)), "-",
                        cast<std::string>(month(col<&Person::birthday>)), "-",
                        cast<std::string>(day(col<&Person::birthday>))))) | as<"birthday_recreated">,
    days_between(col<&Person::birthday>, Date("2011-01-01")) | as<"age_in_days">,
    unixepoch(col<&Person::birthday> + std::chrono::days(10)) | as<"birthday_unixepoch">,
    hour(col<&Person::birthday>) | as<"hour">,
    minute(col<&Person::birthday>) | as<"minute">,
    second(col<&Person::birthday>) | as<"second">,
    weekday(col<&Person::birthday>) | as<"weekday">
) | order_by(col<&Person::id>) | to<std::vector<Birthday>>;
```

### Notes

- All timestamp operations are type-safe and propagate nullability as described above.
- You can chain and nest timestamp operations with other operations.
- Duration arithmetic supports `std::chrono::days`, `std::chrono::years`, `std::chrono::weeks`, `std::chrono::milliseconds`, etc.
- The result types and SQL translation may differ slightly between SQLite and Postgres, but the C++ interface is unified. 
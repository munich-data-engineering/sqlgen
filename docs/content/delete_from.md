# `sqlgen::delete_from` 

The `sqlgen::delete_from` interface provides a type-safe way to delete records from a SQL database. It supports composable query building with `where` clauses to specify which records should be deleted.

## Usage

### Basic Delete

Delete all records from a table:

```cpp
const auto conn = sqlgen::sqlite::connect("database.db");

sqlgen::delete_from<Person>(conn).value();
```

This generates the following SQL:

```sql
DELETE FROM "Person";
```

Note that `conn` is actually a connection wrapped into an `sqlgen::Result<...>`.
This means you can use monadic error handling and fit this into a single line:

```cpp
// sqlgen::Result<Ref<Connection>>
const auto result = sqlgen::sqlite::connect("database.db").and_then(
                        sqlgen::delete_from<Person>);
```

Please refer to the documentation on `sqlgen::Result<...>` for more information on error handling.

### With `where` clause

Delete specific records using a `where` clause:

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

const auto query = delete_from<Person> |
                   where("first_name"_c == "Hugo");

query(conn).value();
```

This generates the following SQL:

```sql
DELETE FROM "Person"
WHERE "first_name" = 'Hugo';
```

#### Using column member pointers

The same query can reference `Person`'s fields with member pointers. The generated
SQL qualifies the referenced column with the table name:

```cpp
const auto query = delete_from<Person> |
                   where(col<&Person::first_name> == "Hugo");
```

This generates the following SQL:

```sql
DELETE FROM "Person"
WHERE "Person"."first_name" = 'Hugo';
```

Note that `"..."_c` refers to the name of the column. If such a field does not
exist on the struct `Person`, the code will fail to compile. `"..."_c` is
defined in the namespace `sqlgen::literals`.

You can also use monadic error handling here:

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

const auto query = delete_from<Person> |
                   where("first_name"_c == "Hugo");

// sqlgen::Result<Ref<Connection>>
const auto result = sqlite::connect("database.db").and_then(query);
```

## Example: Full Query Composition

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

const auto query = delete_from<Person> |
                   where("age"_c >= 18 and "last_name"_c == "Simpson");

const auto result = query(conn);

if (!result) {
    // Error handling
}
```

This generates the following SQL:

```sql
DELETE FROM "Person"
WHERE ("age" >= 18) AND ("last_name" = 'Simpson');
```

#### Using column member pointers

```cpp
const auto query = delete_from<Person> |
                   where(col<&Person::age> >= 18 and col<&Person::last_name> == "Simpson");
```

This generates the following SQL:

```sql
DELETE FROM "Person"
WHERE ("Person"."age" >= 18) AND ("Person"."last_name" = 'Simpson');
```

It is strongly recommended that you use `using namespace sqlgen` and `using namespace sqlgen::literals`. However,
if you do not want to do that, you can rewrite the example above as follows:

```cpp
const auto query = sqlgen::delete_from<Person> |
                   sqlgen::where(sqlgen::col<"age"> >= 18 and sqlgen::col<"last_name"> == "Simpson");

const auto result = query(conn);
```

## Notes

- The `where` clause is optional - if omitted, all records will be deleted
- The `Result<Ref<Connection>>` type provides error handling; use `.value()` to extract the result (will throw an exception if there's an error) or handle errors as needed or refer to the documentation on `sqlgen::Result<...>` for other forms of error handling.
- `"..."_c` refers to the name of the column


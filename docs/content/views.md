# Views

sqlgen supports creating and managing database views, which are virtual tables based on the result set of a SQL statement. Views provide a way to simplify complex queries, present data in a specific format, or restrict access to certain columns.

## Defining Views

To define a view in sqlgen, create a struct with the `is_view` flag:

```cpp
struct EmployeeView {
  static constexpr bool is_view = true;
  
  std::string first_name;
  std::string last_name;
  std::string department;
  int salary;
};
```

### Custom View Names

You can optionally specify a custom name for your view using either `viewname` or `tablename`:

```cpp
struct EmployeeView {
  static constexpr const char* viewname = "EMPLOYEE_SUMMARY";  // or tablename = "EMPLOYEE_SUMMARY"
  static constexpr bool is_view = true;
  
  std::string first_name;
  std::string last_name;
  std::string department;
  int salary;
};
```

Both `viewname` and `tablename` work identically for views - they are just syntactic sugar.

## Creating Views

### Basic View Creation

Create a view from a SELECT query:

```cpp
using namespace sqlgen;
using namespace sqlgen::literals;

const auto employee_query = select_from<Employee>("first_name"_c, "last_name"_c, "department"_c, "salary"_c);
const auto create_view = create_as<EmployeeView>(employee_query);

create_view(conn).value();
```

This generates:

```sql
CREATE VIEW "EmployeeView" AS SELECT "first_name", "last_name", "department", "salary" FROM "Employee";
```

### Create or Replace View

Use `create_or_replace_view_as` to create a view or replace it if it already exists:

```cpp
const auto employee_query = select_from<Employee>("first_name"_c, "last_name"_c, "department"_c, "salary"_c);
const auto create_view = create_or_replace_view_as<EmployeeView>(employee_query);

create_view(conn).value();
```

This generates different SQL depending on the database:

- **MySQL/PostgreSQL**: `CREATE OR REPLACE VIEW "EMPLOYEE_SUMMARY" AS SELECT ...`
- **SQLite**: `CREATE VIEW "EmployeeView" AS SELECT ...`

**Important**: SQLite does not support `CREATE OR REPLACE VIEW`. When you use `create_or_replace_view_as` with SQLite, sqlgen does not automatically translate this and will result in an error if the view already exists. The recommended way to achieve this is to manually drop the view before creating it:

```cpp
(drop<EmployeeView> | if_exists)(conn).value();
const auto create_view = create_as<EmployeeView>(employee_query);
create_view(conn).value();
```

### View Creation with if_not_exists

For SQLite compatibility, you can also use the `if_not_exists` modifier:

```cpp
const auto employee_query = select_from<Employee>("first_name"_c, "last_name"_c, "department"_c, "salary"_c);
const auto create_view = create_as<EmployeeView>(employee_query) | if_not_exists;

create_view(conn).value();
```

This generates different SQL depending on the database:

- **SQLite**: `CREATE VIEW IF NOT EXISTS "EmployeeView" AS SELECT ...`
- **MySQL/PostgreSQL**: `CREATE VIEW "EmployeeView" AS SELECT ...` (if_not_exists ignored for views)

## Complex View Examples

### Filtered Views

Create views with WHERE clauses:

```cpp
struct HighSalaryEmployees {
  static constexpr bool is_view = true;
  
  std::string first_name;
  std::string last_name;
  int salary;
};

const auto high_salary_query = select_from<Employee>("first_name"_c, "last_name"_c, "salary"_c) |
                               where("salary"_c > 50000);

const auto create_high_salary_view = create_or_replace_view_as<HighSalaryEmployees>(high_salary_query);
```

### Joined Views

Create views from joined tables:

```cpp
struct EmployeeDepartment {
  static constexpr bool is_view = true;
  
  std::string first_name;
  std::string last_name;
  std::string department_name;
  std::string location;
};

const auto joined_query = select_from<Employee, "t1">(
    "first_name"_t1 | as<"first_name">,
    "last_name"_t1 | as<"last_name">,
    "department_name"_t2 | as<"department_name">,
    "location"_t2 | as<"location">
) |
inner_join<Department, "t2">("department_id"_t1 == "id"_t2);

const auto create_joined_view = create_or_replace_view_as<EmployeeDepartment>(joined_query);
```

### Aggregated Views

Create views with aggregations:

```cpp
struct DepartmentStats {
  static constexpr bool is_view = true;
  
  std::string department;
  int employee_count;
  double avg_salary;
  int total_salary;
};

const auto stats_query = select_from<Employee>(
                            "department"_c,
                            count("id"_c) | as<"employee_count">,
                            avg("salary"_c) | as<"avg_salary">,
                            sum("salary"_c) | as<"total_salary">
                        ) | group_by("department"_c);

const auto create_stats_view = create_or_replace_view_as<DepartmentStats>(stats_query);
```

## Working with Views

### Reading from Views

Views can be used just like regular tables:

```cpp
// Read all data from the view
const auto employees = sqlgen::read<std::vector<EmployeeView>>(conn).value();

// Query the view with conditions
const auto high_earners_query = sqlgen::read<std::vector<EmployeeView>> | where("salary"_c > 75000);
const auto high_earners = high_earners_query(conn).value();
```

### Dropping Views

Drop views using the same `drop` interface:

```cpp
using namespace sqlgen;

const auto result = drop<EmployeeView>(conn);
// or with if_exists
const auto result = (drop<EmployeeView> | if_exists)(conn);
```

## Using member pointers

The `select_from` columns in the view examples above can instead reference the
struct fields with member pointers through the `col` template. For instance, the
filtered view and joined view examples become:

```cpp
const auto high_salary_query = select_from<Employee>(
                                   col<&Employee::first_name>,
                                   col<&Employee::last_name>,
                                   col<&Employee::salary>) |
                               where(col<&Employee::salary> > 50000);
```

```cpp
const auto joined_query = select_from<Employee, "t1">(
    col<&Employee::first_name, "t1"> | as<"first_name">,
    col<&Employee::last_name, "t1"> | as<"last_name">,
    col<&Department::name, "t2"> | as<"department_name">,
    col<&Department::location, "t2"> | as<"location">
) |
inner_join<Department, "t2">(col<&Employee::department_id, "t1"> == col<&Department::id, "t2">);
```

## Database-Specific Behavior

### MySQL and PostgreSQL

Both MySQL and PostgreSQL support `CREATE OR REPLACE VIEW`:

```sql
CREATE OR REPLACE VIEW "EMPLOYEE_SUMMARY" AS SELECT ...;
```

### SQLite

SQLite has different view creation behavior. It does not support `CREATE OR REPLACE VIEW`. Instead, you should manually drop the view if it exists before creating it. See the example in the "Create or Replace View" section.

## Best Practices

1. **Naming Convention**: Use descriptive names for views that indicate their purpose
2. **Column Selection**: Only include columns that are actually needed in the view
3. **Performance**: Views with complex joins or aggregations may impact query performance
4. **Maintenance**: Keep views simple and focused on a single purpose
5. **Documentation**: Document the purpose and expected data of each view
6. **Database Compatibility**: Be aware of database-specific differences in view creation syntax
7. **SQLite Users**: Use `create_as<ViewType>(query) | if_not_exists` for better compatibility

## Notes

- Views are read-only by default in most databases
- Views can be used in other queries just like regular tables
- The `is_view = true` flag is required to distinguish views from tables
- Custom view names using `viewname` or `tablename` are optional but recommended for clarity
- Views can be created from any valid SELECT query, including complex joins, aggregations, and subqueries
- **SQLite Limitation**: Use `create_as<ViewType>(query) | if_not_exists` instead of `create_or_replace_view_as` for better compatibility
- **MySQL/PostgreSQL**: Both work well with `create_or_replace_view_as` 
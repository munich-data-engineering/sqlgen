# `unite` and `unite_all`

The `unite` and `unite_all` functions allow you to combine the results of multiple `SELECT` statements into a single result set.

## `unite`

The `unite` function corresponds to the SQL `UNION` operator. It combines the result sets of two or more `SELECT` statements and removes duplicate rows.

### Example

```cpp
struct User1 {
  std::string name;
  int age;
};

struct User2 {
  std::string name;
  int age;
};

const auto s1 = sqlgen::select_from<User1>("name"_c, "age"_c);
const auto s2 = sqlgen::select_from<User2>("name"_c, "age"_c);

const auto united = sqlgen::unite<std::vector<User1>>(s1, s2);
```

## `unite_all`

The `unite_all` function corresponds to the SQL `UNION ALL` operator. It combines the result sets of two or more `SELECT` statements, including all duplicate rows.

### Example

```cpp
struct User1 {
  std::string name;
  int age;
};

struct User2 {
  std::string name;
  int age;
};

const auto s1 = sqlgen::select_from<User1>("name"_c, "age"_c);
const auto s2 = sqlgen::select_from<User2>("name"_c, "age"_c);

const auto united = sqlgen::unite_all<std::vector<User1>>(s1, s2);
```

## Using member pointers

The `select_from` columns in the examples above can instead reference the struct
fields with member pointers through the `col` template. For `unite`:

```cpp
const auto s1 = sqlgen::select_from<User1>(col<&User1::name>, col<&User1::age>);
const auto s2 = sqlgen::select_from<User2>(col<&User2::name>, col<&User2::age>);
```

And for the join nesting example, pass the alias as the second template argument:

```cpp
const auto sel = select_from<Login, "t1">(col<&Login::id, "t1">, col<&Login::username, "t1">) |
                 inner_join<"t2">(united, col<&Login::username, "t1"> == col<&User1::name, "t2">) |
                 where(col<&Login::id, "t1"> == 1) | to<std::vector<Login>>;
```

## Nesting in `SELECT` statements

You can use the result of a `unite` or `unite_all` operation as a subquery in a `SELECT` statement.

### Example

```cpp
const auto united = sqlgen::unite<std::vector<User1>>(s1, s2);

const auto sel = sqlgen::select_from(united, "name"_c, "age"_c);
```

## Nesting in `JOIN` statements

You can also use the result of a `unite` or `unite_all` operation as a subquery in a `JOIN` statement.

### Example

```cpp
struct Login {
  int id;
  std::string username;
};

const auto united = sqlgen::unite<std::vector<User1>>(s1, s2);

const auto sel = select_from<Login, "t1">("id"_t1, "username"_t1) |
                 inner_join<"t2">(united, "username"_t1 == "name"_t2) |
                 where("id"_t1 == 1) | to<std::vector<Login>>;
```

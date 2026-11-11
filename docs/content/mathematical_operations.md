# Mathematical Operations

The `sqlgen` library provides a set of mathematical functions for use in queries. These functions are type-safe and map to the appropriate SQL operations for the target database.

## Motivating Example
Suppose you want to analyze the ages of children in a family, grouped by their last name, and you want to compute the average, minimum, and maximum age (with some mathematical operations applied):

```cpp
struct Children {
  std::string last_name;
  double avg_age;
  double max_age_plus_one;
  double min_age_plus_one;
};

const auto get_children =
    select_from<Person>(
        "last_name"_c,
        round(avg(cast<double>("age"_c))).as<"avg_age">(),
        max(cast<double>("age"_c) + 1.0).as<"max_age_plus_one">(),
        (min(cast<double>("age"_c)) + 1.0).as<"min_age_plus_one">()
    )
    | where("age"_c < 18)
    | group_by("last_name"_c)
    | to<std::vector<Children>>;
```

This query groups people by last name, filters for those under 18, and computes the average age (rounded), the maximum age plus one, and the minimum age plus one.

## Using member pointers

Every column literal above (`"age"_c`, `"last_name"_c`, …) can be replaced with a
member pointer to the corresponding struct field through the `col` template, e.g.
`col<&Person::age>`. The motivating example rewritten with member pointers:

```cpp
const auto get_children =
    select_from<Person>(
        col<&Person::last_name>,
        round(avg(cast<double>(col<&Person::age>))).as<"avg_age">(),
        max(cast<double>(col<&Person::age>) + 1.0).as<"max_age_plus_one">(),
        (min(cast<double>(col<&Person::age>)) + 1.0).as<"min_age_plus_one">()
    )
    | where(col<&Person::age> < 18)
    | group_by(col<&Person::last_name>)
    | to<std::vector<Children>>;
```

## Mathematical Functions

### `abs`
Returns the absolute value of a numeric expression.

```cpp
abs("age"_c * (-1)) | as<"abs_age">
```

### `ceil` / `floor`
Rounds a numeric value up (`ceil`) or down (`floor`) to the nearest integer.

```cpp
ceil("salary"_c) | as<"salary_ceiled">
floor("salary"_c) | as<"salary_floored">
```

### `exp`, `ln`, `log2`, `sqrt`
- `exp(x)`: Exponential function (e^x)
- `ln(x)`: Natural logarithm
- `log2(x)`: Base-2 logarithm
- `sqrt(x)`: Square root

```cpp
round(exp(cast<double>("age"_c)), 2) | as<"exp_age">
round(sqrt(cast<double>("age"_c)), 2) | as<"sqrt_age">
```

### `sin`, `cos`, `tan`
Trigonometric functions.

```cpp
sin("angle"_c) | as<"sin_angle">
cos("angle"_c) | as<"cos_angle">
tan("angle"_c) | as<"tan_angle">
```

### `round`
Rounds a numeric value to a specified number of decimal places.

```cpp
round("price"_c, 2) | as<"rounded_price">
``` 
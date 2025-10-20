#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include "fstring.hpp"

// A simple testing framework
#define RUN_TEST(name) \
    std::cout << "Running test: " << #name << "... "; \
    test_##name(); \
    std::cout << "PASSED" << std::endl

// A simple testing framework
#define TEST(name) void test_##name()

// Test functions
TEST(basic_string_substitution);
TEST(multiple_substitutions);
TEST(different_types);
TEST(nested_braces);
TEST(format_specifiers);
TEST(missing_argument);
TEST(escape_braces);

int main() {

    // Example usage
    std::cout << "This code:" << std::endl <<
R"delimeter(
    std::cout << f(
R"(My name is {name}.
My surname is {surname}.
I am {age} years old.
My body weight is {weight} kg.
And all that I'm saying is {status}.
)",
    "name"_a="John",
    "surname"_a="Doe",
    "weight"_a=70.5,
    "age"_a=30,
    "status"_a=true
    ) << std::endl;
)delimeter" << std::endl << "Outputs:" << std::endl << std::endl;

    // Example usage
    std::cout << f(
R"(My name is {name}.
My surname is {surname}.
I am {age} years old.
My body weight is {weight} kg.
And all that I'm saying is {status}.
)",
    "name"_a="John",
    "surname"_a="Doe",
    "weight"_a=70.5,
    "age"_a=30,
    "status"_a=true
    ) << std::endl << std::endl;

    // Run all tests
    std::cout << "Running fstring tests..." << std::endl << std::endl;

    // Run all tests
    RUN_TEST(basic_string_substitution);
    RUN_TEST(multiple_substitutions);
    RUN_TEST(different_types);
    RUN_TEST(nested_braces);
    RUN_TEST(format_specifiers);
    RUN_TEST(missing_argument);
    RUN_TEST(escape_braces);

    std::cout << std::endl << "All tests passed!" << std::endl;

    return 0;
}

// Test basic string substitution
TEST(basic_string_substitution) {
    std::string name = "John";
    std::string result = f("{name}", "name"_a=name);
    assert(result == "John");
}

// Test multiple substitutions
TEST(multiple_substitutions) {
    std::string name = "Alice";
    int age = 30;
    std::string result = f("{name} is {age} years old", "name"_a=name, "age"_a=age);
    assert(result == "Alice is 30 years old");
}

// Test different types
TEST(different_types) {
    std::string name = "Bob";
    int count = 5;
    double price = 10.5;
    bool is_active = true;
    
    std::string result = f("{name} bought {count} items at ${price} each. Active: {active}", 
                          "name"_a=name, 
                          "count"_a=count, 
                          "price"_a=price, 
                          "active"_a=is_active);
    
    assert(result == "Bob bought 5 items at $10.5 each. Active: true");
}

// Test with nested braces
TEST(nested_braces) {
    std::vector<int> numbers = {1, 2, 3};
    std::string result = f("Vector: {nums}", "nums"_a="[" + std::to_string(numbers[0]) + ", " + 
                                                  std::to_string(numbers[1]) + ", " + 
                                                  std::to_string(numbers[2]) + "]");
    assert(result == "Vector: [1, 2, 3]");
}

// Test format specifiers
TEST(format_specifiers) {
    // String formatting
    assert(apply_format_spec("Hello", "s") == "Hello");
    assert(apply_format_spec("Hello", "<10") == "Hello     ");
    assert(apply_format_spec("Hello", ">10") == "     Hello");
    assert(apply_format_spec("Hello", "^10") == "  Hello   ");
    assert(apply_format_spec("Hello", "_>10") == "_____Hello");
    
    // Integer formatting
    assert(apply_format_spec("42", "d") == "42");
    assert(apply_format_spec("42", "+d") == "+42");
    assert(apply_format_spec("42", " d") == " 42");
    assert(apply_format_spec("42", "#b") == "0b101010");
    assert(apply_format_spec("42", "#o") == "052");
    assert(apply_format_spec("42", "#x") == "0x2a");
    assert(apply_format_spec("42", "#X") == "0X2A");
    
    // Integer formatting with width and alignment
    assert(apply_format_spec("42", "5d") == "   42");
    assert(apply_format_spec("42", "<5d") == "42   ");
    assert(apply_format_spec("42", "^5d") == " 42  ");
    assert(apply_format_spec("42", "05d") == "00042");
    
    // Float formatting
    assert(apply_format_spec("3.14159", "f") == "3.141590"); // Default precision
    assert(apply_format_spec("3.14159", ".2f") == "3.14");
    assert(apply_format_spec("3.14159", "08.2f") == "00003.14");
    assert(apply_format_spec("3.14159", "+.2f") == "+3.14");
    assert(apply_format_spec("3.14159", " .2f") == " 3.14");
    assert(apply_format_spec("3.14159", "e") == "3.141590e+00");
    assert(apply_format_spec("3.14159", "E") == "3.141590E+00");
    assert(apply_format_spec("0.75", "%") == "75.000000%");
    assert(apply_format_spec("0.75", ".1%") == "75.0%");

    // Float formatting with width and alignment
    assert(apply_format_spec("3.14159", "10f") == "  3.141590");
    assert(apply_format_spec("3.14159", "<10f") == "3.141590  ");
    assert(apply_format_spec("3.14159", "^10f") == " 3.141590 ");
    assert(apply_format_spec("3.14159", "010f") == "003.141590");
    
    // Percentage formatting with width and alignment
    assert(apply_format_spec("0.75", "11%") == " 75.000000%");
    assert(apply_format_spec("0.75", "<11%") == "75.000000% ");
    assert(apply_format_spec("0.75", "^11%") == "75.000000% ");
    assert(apply_format_spec("0.75", "011%") == "075.000000%");
    
    // Scientific notation formatting with width and alignment
    assert(apply_format_spec("3.14159", "10e") == "3.141590e+00");
    assert(apply_format_spec("3.14159", "<10e") == "3.141590e+00");
    assert(apply_format_spec("3.14159", "^10e") == "3.141590e+00");
    assert(apply_format_spec("3.14159", "010e") == "3.141590e+00");

    // Testing format specifiers within f-string syntax
    int number = 42;
    std::string result = f("Number: {num:d}", "num"_a=number);
    assert(result == "Number: 42");
    
    double value = 3.14159;
    result = f("Pi: {pi:f}", "pi"_a=value);
    assert(result == "Pi: 3.141590");

    result = f("Pi: {pi:.2f}", "pi"_a=value);
    assert(result == "Pi: 3.14");

    result = f("Pi: {pi:08.2f}", "pi"_a=value);
    assert(result == "Pi: 00003.14");

    result = f("Pi: {pi:+.2f}", "pi"_a=value);
    assert(result == "Pi: +3.14");

    result = f("Pi: {pi: .2f}", "pi"_a=value);
    assert(result == "Pi:  3.14");

    result = f("Pi: {pi:e}", "pi"_a=value);
    assert(result == "Pi: 3.141590e+00");

    result = f("Pi: {pi:E}", "pi"_a=value);
    assert(result == "Pi: 3.141590E+00");

    result = f("Percentage: {val:%}", "val"_a=0.75);
    assert(result == "Percentage: 75.000000%");

    result = f("Percentage: {val:.1%}", "val"_a=0.75);
    assert(result == "Percentage: 75.0%");
}

// Test what happens when an argument is missing
TEST(missing_argument) {
    bool exception_caught = false;
    try {
        std::string result = f("{name} is here", "wrong_name"_a="John");
    } catch (const std::runtime_error& e) {
        exception_caught = true;
        std::string error_msg = e.what();
        assert(error_msg.find("No value provided for placeholder: name") != std::string::npos);
    }
    assert(exception_caught);
}

// Test escaping braces
TEST(escape_braces) {
    std::string name = "Charlie";
    std::string result = f("{{name}} is not replaced, but {name} is", "name"_a=name);
    assert(result == "{name} is not replaced, but Charlie is");
}


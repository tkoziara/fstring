#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <functional>
#include <regex>
#include <stdexcept>
#include <type_traits>
#include <experimental/type_traits>

namespace fstring {

namespace detail {
    // Helper to check if a type can be streamed
    template<typename T>
    using is_streamable = decltype(std::declval<std::ostream&>() << std::declval<T>());

    template<typename T>
    constexpr bool is_streamable_v = std::experimental::is_detected_v<is_streamable, T>;

    // Convert any streamable type to string
    template<typename T>
    std::string to_string(const T& value) {
        if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, const char*>) {
            return value;
        } else if constexpr (is_streamable_v<T>) {
            std::ostringstream ss;
            ss << value;
            return ss.str();
        } else {
            static_assert(is_streamable_v<T>, "Type must be streamable to std::ostream");
            return ""; // unreachable
        }
    }
}

// Apply format specifiers (basic implementation)
inline std::string apply_format_spec(const std::string& value, const std::string& format_spec) {
    // This is a simplified implementation that handles a few common Python format specifiers
    if (format_spec.empty()) {
        return value;
    }

    // Handle some basic format specifiers
    char type = format_spec.back();
    
    switch (type) {
        case 's': // String
            return value;
        case 'd': // Decimal integer
        case 'f': // Float
            // For numbers, we would need to parse and format, this is simplified
            return value;
        default:
            // We could implement more specifiers like alignment, fill, width, etc.
            return value;
    }
}

/**
 * f-string implementation for C++
 * Usage: f("{name} is {age} years old", "name"_a=name, "age"_a=age)
 */
template<typename... Args>
std::string f(const std::string& format, Args&&... args) {
    return f_impl(format, std::forward<Args>(args)...);
}

// Implementation details for f-string
template<typename... Args>
std::string f_impl(const std::string& format, Args&&... args) {
    std::regex placeholder_pattern(R"(\{([^{}:]+)(?::([^{}]*))?\})");
    std::string result = format;
    
    // Pack the arguments into a tuple of pairs (name, value)
    std::tuple<Args...> arg_tuple{std::forward<Args>(args)...};
    
    // Find placeholders
    std::smatch match;
    std::string temp = format;
    std::map<std::string, std::string> replacements;
    
    // Collect replacements from tuple
    collect_replacements(replacements, arg_tuple, std::index_sequence_for<Args...>{});
    
    // Apply all replacements
    while (std::regex_search(temp, match, placeholder_pattern)) {
        std::string placeholder_name = match[1].str();
        std::string format_spec = match[2].str();
        
        auto it = replacements.find(placeholder_name);
        if (it == replacements.end()) {
            throw std::runtime_error("No value provided for placeholder: " + placeholder_name);
        }
        
        std::string replacement = it->second;
        
        // Apply format specifier if present (basic implementation)
        if (!format_spec.empty()) {
            replacement = apply_format_spec(replacement, format_spec);
        }
        
        // Replace the placeholder with the value
        result = std::regex_replace(result, 
                                   std::regex("\\{" + placeholder_name + 
                                             (format_spec.empty() ? "\\}" : ":" + format_spec + "\\}")),
                                   replacement, 
                                   std::regex_constants::format_first_only);
        
        // Move to next match
        temp = match.suffix().str();
    }
    
    return result;
}

// Helper to collect replacements from the argument tuple
template<typename Tuple, std::size_t... Is>
void collect_replacements(std::map<std::string, std::string>& replacements,
                         const Tuple& tuple,
                         std::index_sequence<Is...>) {
    (void)(collect_replacement(replacements, std::get<Is>(tuple)), ...);
}

// Process a single name=value pair
template<typename T>
void collect_replacement(std::map<std::string, std::string>& replacements, const T& arg) {
    replacements[arg.name] = detail::to_string(arg.value);
}

// Name-value pair for named arguments
struct NamedArgBase {
    std::string name;
};

// Name-value pair for named arguments
template<typename T>
struct NamedArg : NamedArgBase {
    T value;

    NamedArg(const std::string& name, const T& value) : NamedArgBase{name}, value(value) {}

    // Specialize value to bool
    NamedArg<std::string> operator=(bool value) const {
        return {name, value ? std::string("true") : std::string("false")};
    }

    // Specialize value to const char*
    NamedArg<std::string> operator=(const char* value) const {
        return {name, std::string(value)};
    }

    // Assign value to named argument
    template<typename U>
    NamedArg<U> operator=(const U& value) const {
        return {name, value};
    }

    // Support for rvalue references
    template<typename U>
    NamedArg<U> operator=(U&& value) const {
        return {name, std::forward<U>(value)};
    }
};

// User-defined literal for named arguments
NamedArg<std::string> operator""_a(const char* name, std::size_t len) {
    return {std::string(name, len), std::string{}};
}

} // namespace fstring

// Convenience using directive
using namespace fstring;

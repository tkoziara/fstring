#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <functional>
#include <regex>
#include <stdexcept>
#include <type_traits>

namespace fstring {

namespace detail {
    // Helper to check if a type can be streamed
    template <typename, typename = std::void_t<>>
    struct is_streamable : std::false_type {};

    template <typename T>
    struct is_streamable<T, std::void_t<decltype(std::declval<std::ostream&>() << std::declval<T>())>> 
        : std::true_type {};

    template <typename T>
    inline constexpr bool is_streamable_v = is_streamable<T>::value;

    // Convert any streamable type to string
    template<typename T>
    std::string to_string(const T& value) {
        if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, const char*>) {
            return value;
        } else if constexpr (std::is_convertible_v<T, std::string>) {
            return std::string(value);
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

// Apply format specifiers
inline std::string apply_format_spec(const std::string& value, const std::string& format_spec) {
    if (format_spec.empty()) {
        return value; // No formatting needed
    }
    
    // Parsing format specifier
    char fill = ' '; // Default fill character
    char align = '>'; // Default right alignment
    char sign = '\0';  // Null character (ASCII 0)
    bool alternate_form = false; // '#' flag
    bool zero_padding = false; // '0' flag
    int width = 0;
    int precision = -1;
    char type = '\0'; // Format type (d, f, x, etc.)
    
    size_t pos = 0;
    
    // Check for fill and alignment
    if (format_spec.size() > 1 && (format_spec[1] == '<' ||
        format_spec[1] == '>' || format_spec[1] == '^' || format_spec[1] == '=')) {
        fill = format_spec[0];
        align = format_spec[1];
        pos = 2;
    } else if (format_spec[0] == '<' || format_spec[0] == '>' ||
               format_spec[0] == '^' || format_spec[0] == '=') {
        align = format_spec[0];
        pos = 1;
    }
    
    // Check for sign specifier
    if (format_spec[pos] == '+' || format_spec[pos] == '-' || format_spec[pos] == ' ') {
        sign = format_spec[pos];
        pos++;
    }
    
    // Check for alternate form ('#')
    if (format_spec[pos] == '#') {
        alternate_form = true;
        pos++;
    }
    
    // Check for zero-padding ('0')
    if (format_spec[pos] == '0') {
        zero_padding = true;
        pos++;
    }
    
    // Parse width
    while (pos < format_spec.size() && std::isdigit(format_spec[pos])) {
        width = width * 10 + (format_spec[pos] - '0');
        pos++;
    }
    
    // Parse precision
    if (pos < format_spec.size() && format_spec[pos] == '.') {
        pos++;
        precision = 0;
        while (pos < format_spec.size() && std::isdigit(format_spec[pos])) {
            precision = precision * 10 + (format_spec[pos] - '0');
            pos++;
        }
    }
    
    // Check type specifier
    if (pos < format_spec.size()) {
        type = format_spec[pos];
    }
    
    std::ostringstream ss;
    
    // Apply sign handling
    if (sign == '+' && value[0] != '-') {
        ss << '+';
    } else if (sign == ' ' && value[0] != '-') {
        ss << ' ';
    }
    
    // Handle different type specifiers
    switch (type) {
        case 's': // String
            ss << value;
            break;
        case 'd': // Decimal integer
            ss << std::stoi(value);
            break;
        case 'b': // Binary
            {
                int num = std::stoi(value);
                std::string binary = std::bitset<32>(num).to_string();
                binary.erase(0, binary.find_first_not_of('0')); // Remove leading zeros
                if (binary.empty()) binary = "0"; // Ensure at least one digit
                if (alternate_form) {
                    ss << "0b";
                }
                ss << binary;
            }
            break;
        case 'o': // Octal
            if (alternate_form) ss << "0";
            ss << std::oct << std::stoi(value);
            break;
        case 'x': // Hexadecimal (lowercase)
            if (alternate_form) ss << "0x";
            ss << std::hex << std::stoi(value);
            break;
        case 'X': // Hexadecimal (uppercase)
            if (alternate_form) ss << "0X";
            ss << std::uppercase << std::hex << std::stoi(value);
            break;
        case 'f': // Fixed-point float
        case 'e': // Scientific notation (lowercase)
        case 'E': // Scientific notation (uppercase)
        case '%': // Percentage
            {
                double num = std::stod(value);
                if (precision < 0) {
                    precision = 6; // Default precision for floating-point numbers
                }
                ss.precision(precision);
                if (type == 'f') {
                    ss.setf(std::ios::fixed);
                } else if (type == 'e') {
                    ss.setf(std::ios::scientific);
                } else if (type == 'E') {
                    ss.setf(std::ios::scientific);
                    ss << std::uppercase;
                } else if (type == '%') {
                    ss.setf(std::ios::fixed);
                    num *= 100;
                }
                ss << num;
                if (type == '%') {
                    ss << '%';
                }
            }
            break;
        default:
            ss << value;
    }
    
    std::string formatted = ss.str();
    
    // Apply width and alignment with zero-padding support
    if (formatted.size() < static_cast<size_t>(width)) {
        size_t padding = width - formatted.size();
        if (zero_padding && align == '>') {
            if (!formatted.empty() && (formatted[0] == '+' || formatted[0] == '-')) {
                char sign_char = formatted[0];
                formatted = formatted.substr(1);
                formatted.insert(0, padding, '0');
                formatted.insert(0, 1, sign_char);
            } else {
                formatted.insert(0, padding, '0');
            }
        } else {
            if (align == '<') {
                formatted.append(padding, fill);
            } else if (align == '>') {
                formatted.insert(0, padding, fill);
            } else if (align == '^') {
                size_t left_pad = padding / 2;
                size_t right_pad = padding - left_pad;
                formatted.insert(0, left_pad, fill);
                formatted.append(right_pad, fill);
            }
        }
    }
    
    return formatted;
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

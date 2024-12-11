# range3::byte_span

byte_span is a non-owning view class for byte sequences, similar to std::span.
It provides unified handling of byte-like types (`std::byte`, `char`, `unsigned char`)
and allows viewing any trivially copyable data as a byte sequence.

## Features

- Non-owning view type with zero overhead
- Unified handling of byte-like types (`std::byte`, `char`, `unsigned char`)
- View any trivially copyable type as byte sequences
- Compatible with any contiguous range (vectors, arrays, C-style arrays)
- Type-safe design using C++20 concepts

## Basic Usage

### Creating from Byte-like Types

The most common use case is creating a byte view from containers of byte-like types:

```cpp
std::vector<char> chars = {'a', 'b', 'c'};
byte_span view1{chars};  // Deduced as byte_span<std::byte>

unsigned char raw[4] = {0x00, 0x01, 0x02, 0x03};
byte_span view2{raw};   // Deduced as byte_span<std::byte, 4>

std::array<std::byte, 3> bytes = {
    std::byte{1}, std::byte{2}, std::byte{3}
};
byte_span view3{bytes}; // Deduced as byte_span<std::byte, 3>
```

### Creating from Any Trivially Copyable Type
View any trivially copyable data as bytes:

```cpp
// From vector of integers
std::vector<int> ints = {1, 2, 3};
byte_span view1{ints};  // Deduced as byte_span<std::byte>
assert(view1.size() == sizeof(int) * 3);

// From custom struct
struct Point { 
    float x, y; 
}; 
Point p{1.0f, 2.0f};
byte_span view2{&p, 1};  // Deduced as byte_span<std::byte>
assert(view2.size() == sizeof(Point)); // sizeof(Point) = 8 bytes (typically)
```

### Dynamic and Static Extents
```cpp
// Dynamic extent (size known at runtime)
std::vector<std::byte> vec(100);
byte_span view1{vec};  // Deduced as byte_span<std::byte, dynamic_extent>

// Static extent (size known at compile time)
std::array<char, 4> arr = {'a', 'b', 'c', 'd'};
byte_span view2{arr};  // Deduced as byte_span<std::byte, 4>

// Converting from static to dynamic extent is always possible
byte_view view3 = view2;  // OK: byte_view has dynamic extent

// Converting from dynamic to static extent requires explicit extent
std::vector<char> vec2(4);
byte_span<std::byte, 4> view4{vec2};  // OK if vec2.size() == 4
```

## Advanced Usage

### Using byte_view and cbyte_view

Two convenient aliases are provided:
- `byte_view`: Alias for `byte_span<std::byte>` (mutable, dynamic_extent)
- `cbyte_view`: Alias for `byte_span<const std::byte>` (immutable, dynamic_extent)

Example:

```cpp
void process_bytes(cbyte_view data) {  // Accept any byte-like input
    for (auto b : data) {
        std::cout << std::format("{:02x} ", std::to_integer<int>(b));
    }
}

// Works with any byte-like container
std::vector<char> chars = {'a', 'b', 'c'};
process_bytes(chars);

std::array<unsigned char, 4> raw = {0x00, 0x01, 0x02, 0x03};
process_bytes(raw);
```

```cpp
// Function accepting read-only byte view
void print_hex_dump(cbyte_view data) {
    for (auto b : data) {
        std::cout << std::format("{:02x} ", std::to_integer<int>(b));
    }
}

// Function accepting mutable byte view
void fill_pattern(byte_view data) {
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = std::byte{static_cast<unsigned char>(i & 0xFF)};
    }
}

// Usage
std::vector<std::byte> buffer(100);
fill_pattern(buffer);      // Modify buffer
print_hex_dump(buffer);    // Read buffer
```

### Error Handling
`byte_span` provides safety guarantees at both compile-time and runtime:

```cpp
// Compile-time checks
struct non_trivially_copyable {
    std::string str;  // non-trivially copyable member
};
non_trivially_copyable obj;
byte_span view{obj};  // Compilation error: type requirements not met

// Extent mismatch
std::vector<char> vec(3);
byte_span<std::byte, 4> view{vec};  // Runtime assertion failure: size mismatch

std::array<char, 8> arr;
byte_span<std::byte, 4> view{arr};  // Compilation error: static extent mismatch
```

### String Handling
Work seamlessly with string types:

```cpp
std::string str = "Hello";
byte_span view1{str};  // View string data as bytes

std::string_view sv = "World";
byte_span view2{sv};   // View string_view data as bytes
// Note: does not include null terminator

// Convert back to string_view
auto sv2 = as_sv(view1);  // Get std::string_view
```

### Type Conversions

Safe conversions between types:

```cpp
std::vector<int> numbers = {1, 2, 3, 4};
byte_span view{numbers};

// Convert to span of ints
std::span<const int> cint_span = as_span<int>(view);  
std::span<int> int_span = as_writable_span<int>(view);

// Get reference to first value
const int& first = as_value<int>(view);  

// Convert to std::span of bytes
std::span<const std::byte> cbyte_std_span = as_bytes(view);
std::span<std::byte> byte_std_span = as_writable_bytes(view);
```

### Sub-views
Create views of specific ranges:

```cpp
std::vector<std::byte> data(100);
byte_span view{data};

auto first_10 = view.first(10);     // First 10 bytes
auto last_10 = view.last(10);       // Last 10 bytes
auto sub_view = view.subspan(5, 20); // 20 bytes starting at offset 5
```

## Requirements

- C++20 or later
- Compiler with concepts support

# Building and installing

See the [BUILDING](BUILDING.md) document.

# Contributing

See the [CONTRIBUTING](CONTRIBUTING.md) document.

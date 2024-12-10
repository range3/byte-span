# byte_span

`byte_span` is a lightweight view class for efficient handling of byte data. Based on the concept of `std::span`, it provides specialized functionality for byte data manipulation.

## Features

- Non-owning view type
- Unified handling of byte-like types (`std::byte`, `char`, `unsigned char`)
- View trivially copyable data types as byte sequences
- Type-safe design using C++20 concepts
- Zero-overhead abstraction

## Basic Usage

### 1. Creating from Byte Arrays

The most basic usage is creating a `byte_span` from a byte array:

```cpp
std::byte data[4] = {
    std::byte{0x00}, std::byte{0x01}, 
    std::byte{0x02}, std::byte{0x03}
};
byte_span view{data};  // Deduced as byte_span<std::byte, 4>
```

### 2. Creating from std::array

Easy creation from `std::array`:

```cpp
std::array<std::byte, 4> arr = {
    std::byte{0x00}, std::byte{0x01}, 
    std::byte{0x02}, std::byte{0x03}
};
byte_span view{arr};  // Deduced as byte_span<std::byte, 4>
```

### 3. Creating from std::vector

You can create views from any contiguous container, including `std::vector`:

```cpp
// Integer data
std::vector<int> numbers = {1, 2, 3, 4};
byte_span view{numbers};  // View integers as bytes

// Access underlying bytes
auto first_byte = view[0];
auto size_in_bytes = view.size();  // size = sizeof(int) * numbers.size()
```

### 4. Dynamic Size Views

For runtime-sized views, use `dynamic_extent`:

```cpp
std::vector<std::byte> vec(100);
byte_span view{vec};  // Deduced as byte_span<std::byte, dynamic_extent>
```

## Advanced Usage

### 1. Using byte_view and cbyte_view

The library provides two convenient type aliases:
- `byte_view`: Mutable byte span (alias for `byte_span<std::byte>`)
- `cbyte_view`: Immutable byte span (alias for `byte_span<const std::byte>`)

Example usage in functions:

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

### 2. Trivially Copyable Types as Byte Views

Any trivially copyable type can be viewed as bytes:

```cpp
struct Point {
    float x;
    float y;
};

Point p{1.0f, 2.0f};
byte_span view{std::span{&p, 1}};  // View Point's bytes
```

### 3. String Data Handling

String data can be easily handled:

```cpp
std::string str = "Hello, World!";
byte_span view{str};  // View string data as bytes

// Convert to string_view
auto sv = as_sv(view);  // Get as std::string_view
```

### 4. Implicit Conversion in Function Arguments

Functions can accept various container types through implicit conversion:

```cpp
void process_bytes(byte_span view) {
    // Process byte data
}

// Various ways to call
std::vector<std::byte> vec(100);
process_bytes(vec);  // OK

std::array<char, 10> arr;
process_bytes(arr);  // OK

unsigned char raw_data[20];
process_bytes(raw_data);  // OK
```

### 5. Data Type Conversions

Safe data type conversions:

```cpp
std::vector<int> numbers = {1, 2, 3, 4};
byte_span view{numbers};

// Read as span of ints
auto int_span = as_span<int>(view);
// Get first value
auto& first_int = as_value<int>(view);
```

### 6. Creating Sub-views

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

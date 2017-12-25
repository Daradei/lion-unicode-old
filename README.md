# Small Unicode library for C++17

This is a small Unicode library for C++17 which supports handling of UTF-32, UTF-16 and UTF-8 sequences. That is, it supports reading and writing from and to files, encoding and decoding for all encoding schemes, as well as converting between them.

Below are laid out summaries of the different parts of the library. After each one follows an explanation of the functions/types, as well as a simple example of how to use them.

## An `std::fstream` wrapper to handle Unicode files

This is a simple wrapper around an `std::fstream` object. This is necessary to achieve higher security, as a Unicode file requires some restrictions - the stream must always be opened in binary mode and no formatted input/output functions should be done on it. For this purpose the wrapper types described below provide only a few of the functions `std::fstream` provides.

```c++
namespace blue::unicode
{
    class ustream_base
    {
    protected:
        ustream_base();
        explicit ustream_base(const char* file, std::ios::openmode mode = std::ios::binary);
        explicit ustream_base(const std::string& file, std::ios::openmode mode = std::ios::binary);
		
        void swap(ustream_base& rhs);

    public:
        using char_type   = std::fstream::char_type;
        using traits_type = std::fstream::traits_type;
        using int_type    = std::fstream::int_type;
        using pos_type    = std::fstream::pos_type;
        using off_type    = std::fstream::off_type;

        ustream_base(const ustream_base&) = delete;
        ustream_base& operator=(const ustream_base&) = delete;

        virtual ~ustream_base() = default;

        bool good() const;
        bool eof() const;
        bool fail() const;
        bool bad() const;

        bool operator!() const;
        explicit operator bool() const;

        std::ios::iostate rdstate() const;
        void setstate(std::ios::iostate state);
        void clear(std::ios::iostate state = std::ios::goodbit);
    };
    
}
```

* The class `ustream_base` holds the actual `std::fstream` and provides some of the function that `std::ios` does. It is used for error checking. 
  + The constructors open the stream as if by `std::fstream stream(filename, mode | std::ios::binary);` where `mode` is the one supplied by the user. 
  + The member types are equivalent to those in `std::fstream`.
  + The member functions call the corresponding `std::fstream` function: `ustream_base::rdstate()` returns `stream.rdstate()`, etc.

```c++
namespace blue::unicode
{
    class uistream : public ustream_base
    {
    public:
        using char_type   = ustream_base::char_type;
        using traits_type = ustream_base::traits_type;
        using int_type    = ustream_base::int_type;
        using pos_type    = ustream_base::pos_type;
        using off_type    = ustream_base::off_type;

        using sentry = std::ifstream::sentry;

    public:
        uistream();
        explicit uistream(const char* file, std::ios::openmode mode = std::ios::in);
        explicit uistream(const std::string& file, std::ios::openmode mode = std::ios::in);

        uistream(const uistream&) = delete;
        uistream& operator=(const uistream&) = delete;

        uistream(uistream&&) = default;
        uistream& operator=(uistream&&) = default;

        void swap(uistream& rhs);

        bool is_open() const;

        void open(const char* file, std::ios::openmode mode = std::ios::in | std::ios::binary);
        void open(const std::string& file, std::ios::openmode mode = std::ios::in | std::ios::binary);

        void close() ;

        uistream& read(char_type* str, std::streamsize count);

        std::streamsize gcount() const;

        pos_type tellg();
        uistream& seekg(pos_type pos) ;
        uistream& seekg(off_type off, std::ios_base::seekdir dir);
    };

    void swap(uistream& lhs, uistream& rhs);
}
```

* The `uistream` class is the equivalent of `std::ifstream`. It is designed to be passed to other parts of this library and not to be used by the user except for error checking.

```c++
namespace blue::unicode
{
    class uostream : public ustream_base
    {
    public:
        using char_type   = ustream_base::char_type;
        using traits_type = ustream_base::traits_type;
        using int_type    = ustream_base::int_type;
        using pos_type    = ustream_base::pos_type;
        using off_type    = ustream_base::off_type;

        using sentry = std::ofstream::sentry;

    public:
        uostream();
        explicit uostream(const char* file, std::ios::openmode mode = std::ios::out);
        explicit uostream(const std::string& file, std::ios::openmode mode = std::ios::out);

        uostream(const uostream&) = delete;
        uostream& operator=(const uostream&) = delete;

        uostream(uostream&&) = default;
        uostream& operator=(uostream&&) = default;

        void swap(uostream& rhs);

        bool is_open() const;

        void open(const char* file, std::ios::openmode mode = std::ios::out | std::ios::binary);
        void open(const std::string& file, std::ios::openmode mode = std::ios::out | std::ios::binary);

        void close();

        uostream& write(const char_type* str, std::streamsize count);

        pos_type tellp() ;
        uostream& seekp(pos_type pos);
        uostream& seekp(off_type off, std::ios_base::seekdir dir);

        uostream& flush() ;
    };

    void swap(uostream& lhs, uostream& rhs);
}
```

* The `uostream` class is the equivalent of `std::ofstream`. It is designed to be passed to other parts of this library and not to be used by the user except for error checking.

## The ```codepoint``` type

This part of the library defines the `codepoint` type as a 32-bit unsigned integer, as well as some helper functions.

```c++
namespace blue::unicode
{
    using codepoint = char32_t;

    constexpr codepoint replacement_character() noexcept;

    constexpr codepoint codepoint_min() noexcept;
    constexpr codepoint codepoint_max() noexcept;
	
    constexpr bool is_high_surrogate(codepoint cp) noexcept;
    constexpr bool is_low_surrogate(codepoint cp) noexcept;
    constexpr bool is_surrogate(codepoint cp) noexcept;
    constexpr bool is_valid(codepoint cp) noexcept;
    constexpr bool is_BOM(codepoint cp) noexcept;
}
```

* `replacement_character` returns `0xFFFD`.
* `codepoint_min` returns `0`.
* `codepoint_max` returns `0x0010FFFF`.
* `is_high_surrogate` returns `true` if the given codepoint is in the interval `[0xD800, 0xDBFF]`, otherwise it returns `false`.
* `is_low_surrogate` returns `true` if the given codepoint is in the interval `[0xDC00, 0xDFFF]`, otherwise it returns `false`.
* `is_surrogate` returns `true` if the given codepoint is neither a high surrogate, nor a low surrogate, otherwise it returns `false`.
* `is_valid` returns `true` if the given codepoint is not a surrogate and is less than or equal to the `codepoint_max()`, otherwise it returns `false`.
* `is_BOM` returns `true` if the given codepoint is equal to `0xFEFF`.

## Analyzing Unicode files

The following describes functions and types which allow the recognition of the encoding scheme that a Unicode file uses.

```c++
namespace blue::unicode
{
    namespace constants
    {
        constexpr std::string_view UTF32_LE_BOM("\xFF\xFE\x00\x00", 4);
        constexpr std::string_view UTF32_BE_BOM("\x00\x00\xFE\xFF", 4);
        constexpr std::string_view UTF16_LE_BOM("\xFF\xFE", 2);
        constexpr std::string_view UTF16_BE_BOM("\xFE\xFF", 2);
        constexpr std::string_view UTF8_BOM("\xEF\xBB\xBF", 3);
    }

    enum class conversion
    {
        lenient,
        strict
    };

    enum class write_bom
    {
        yes,
        no
    };

    enum class format : char16_t
    {
        utf32,
        utf16,
        utf8,
        unknown
    };

    enum class byte_order : char16_t
    {
        little,
        big,
        none
    };

    struct encoding
    {
        format format = format::unknown;
        byte_order order = byte_order::none;

        static encoding get(uistream& in);
    };
}
```

* The `std::string_view` constants inside of the `constants` namespace provide the BOMs (Byte Order Marks) that may or may not be present in a Unicode file.
* The enum `conversion` provides two constants: `lenient` and `strict`, where:
  + `lenient` is used as a flag to indicate that the conversion should be without checking for errors;
  + `strict` is used as a flag to indicate that the conversion should be done with error checking and taking the appropriate measures.
* The enum `write_bom` provides the constants `yes` and `no`. It is used to indicate whether or not the library should prepend the file with a BOM when writing a UTF sequence to it.
* The enum `format` defines the constants `utf32`, `utf16`, `utf8` and `unknown` to specify the different Unicode encoding formats.
* The enum `byte_order` defines the constants `little`, `big` and `none` to specify the byte order of a file. (`none` is used for UTF-8.)

* The type `encoding` holds two public member variables: `format` and `order` of types `format` and `byte_order` respectively. 
* The static member function `get` takes a `uistream` and figures out the encoding, that is the format and byte_order, of the file. This is done in two steps.
  + Firstly, it checks if the file includes a BOM. If it does, it returns the appropriate encoding regardless of whether or not the file is actually of that encoding.
  + Secondly, it reads some amount of bytes from the stream and it uses a statistical procedure to determine the encoding. As such, this procedure will not succeed 100% of the times, but it will give a pretty good guess.

#### Example usage of `encoding`

```c++
#include <blue/unicode/unicode.hpp> // always use this, don't rely on the small headers

namespace uni = blue::unicode; // convenience namespace alias

int main()
{
    uni::uistream in("utf8file.txt");
    if (!in.is_open()) {
        // failed to open file
    }
    uni::encoding encoding = uni::encoding::get(in);
    switch (encoding.format) 
    {
    uni::encoding::utf32:
        // handle if the encoding is UTF-32
    uni::encoding::utf16:
        // handle if the encoding is UTF-16
    uni::encoding::utf8:
        // handle if the encoding is UTF-8
    uni::encoding::unknown:
        // handle if the encoding is unknown
    }
}

```

## Handling UTF-32, UTF-16 and UTF-8 sequences

The classes `utf32`, `utf16` and `utf8` provide utilities for:
* decoding and encoding
* getting the length of a sequence
* converting between encoding schemes
* reading and writing from and to a file

The three classes provide equivalent interfaces. Because of that there will be one explanation for all three of them after their summaries.

### The `utf32` class
```c++
namespace blue::unicode
{
    class utf32
    {
    public:
        using string_type      = std::u32string;
        using string_view_type = std::u32string_view;
        using char_type        = std::u32string::value_type;

        class iterator;

        template<conversion conv = conversion::strict, typename ForwardIterator>
        static ForwardIterator decode(ForwardIterator first, ForwardIterator last, codepoint& cp);

        template<typename OutputIterator>
        static OutputIterator encode(codepoint cp, OutputIterator output);

        template<typename ForwardIterator>
        static std::size_t length(ForwardIterator first, ForwardIterator last);

        template<conversion = conversion::strict, typename ForwardIterator, typename OutputIterator>
        static OutputIterator to_utf8(ForwardIterator first, ForwardIterator last, OutputIterator output);

        template<conversion = conversion::strict, typename ForwardIterator, typename OutputIterator>
        static OutputIterator to_utf16(ForwardIterator first, ForwardIterator last, OutputIterator output);

        template<conversion = conversion::strict, typename ForwardIterator, typename OutputIterator>
        static OutputIterator to_utf32(ForwardIterator first, ForwardIterator last, OutputIterator output);

        template<typename OutputIterator>
        static OutputIterator read(uistream& in, OutputIterator output, byte_order order);

        template<write_bom wrbom = write_bom::yes, typename ForwardIterator>
        static ForwardIterator write(uostream& out, ForwardIterator first, ForwardIterator last, byte_order order);
    };

    class utf32::iterator
    {
    public:
        using value_type        = codepoint;
        using difference_type   = std::ptrdiff_t;
        using pointer           = void;
        using reference         = codepoint;
        using iterator_category = std::bidirectional_iterator_tag;
        using iterator_type     = utf32::string_type::const_iterator;

   	protected:
  		iterator_type current;

   	public:
        iterator() noexcept;
        explicit iterator(iterator_type it) noexcept;
        iterator(const iterator& rhs) noexcept;

        iterator& operator=(const iterator& rhs) noexcept;

        reference operator*() const noexcept;

        iterator& operator++() noexcept;
        iterator operator++(int) noexcept;

        iterator& operator--() noexcept;
        iterator operator--(int) noexcept;

        iterator_type base() const noexcept;
    };
    
    bool operator==(const utf32::iterator& lhs, const utf32::iterator& rhs) noexcept;
    bool operator!=(const utf32::iterator& lhs, const utf32::iterator& rhs) noexcept;
}
```

### The `utf16` class
```c++
namespace blue::unicode
{
    class utf16
    {
    public:
        using string_type      = std::u16string;
        using string_view_type = std::u16string_view;
        using char_type        = std::u16string::value_type;

        class iterator;

        template<conversion conv = conversion::strict, typename ForwardIterator>
        static ForwardIterator decode(ForwardIterator first, ForwardIterator last, codepoint& cp);

        template<typename OutputIterator>
        static OutputIterator encode(codepoint cp, OutputIterator output);

        template<typename ForwardIterator>
        static std::size_t length(ForwardIterator first, ForwardIterator last);

        template<conversion conv = conversion::strict, typename ForwardIterator, typename OutputIterator>
        static OutputIterator to_utf8(ForwardIterator first, ForwardIterator last, OutputIterator output);

        template<conversion = conversion::strict, typename ForwardIterator, typename OutputIterator>
        static OutputIterator to_utf16(ForwardIterator first, ForwardIterator last, OutputIterator output);

        template<conversion conv = conversion::strict, typename ForwardIterator, typename OutputIterator>
        static OutputIterator to_utf32(ForwardIterator first, ForwardIterator last, OutputIterator output);

        template<typename OutputIterator>
        static OutputIterator read(uistream& in, OutputIterator output, byte_order order);

        template<write_bom wrbom = write_bom::yes, typename ForwardIterator>
        static ForwardIterator write(uostream& out, ForwardIterator first, ForwardIterator last, byte_order order);
    };

    class utf16::iterator
    {
    public:
        using value_type        = codepoint;
        using difference_type   = std::ptrdiff_t;
        using pointer           = void;
        using reference         = codepoint;
        using iterator_category = std::bidirectional_iterator_tag;
        using iterator_type     = utf16::string_type::const_iterator;

    protected:
        iterator_type current;

    public:
        iterator() noexcept;
        explicit iterator(iterator_type it) noexcept;
        iterator(const iterator& rhs) noexcept;

        iterator& operator=(const iterator& rhs) noexcept;

        reference operator*() const noexcept;

        iterator& operator++() noexcept;
        iterator operator++(int) noexcept;

        iterator& operator--() noexcept;
        iterator operator--(int) noexcept;

        iterator_type base() const noexcept;
    };

    bool operator==(const utf16::iterator& lhs, const utf16::iterator& rhs) noexcept;
    bool operator!=(const utf16::iterator& lhs, const utf16::iterator& rhs) noexcept;
}
```

### The `utf8` class
```c++
namespace blue::unicode
{
    class utf8
    {
    public:
        using string_type      = std::string;
        using string_view_type = std::string_view;
        using char_type        = unsigned char;

        class iterator;

        template<conversion conv = conversion::strict, typename ForwardIterator>
        static ForwardIterator decode(ForwardIterator first, ForwardIterator last, codepoint& cp);

        template<typename OutputIterator>
        static OutputIterator encode(codepoint codepoint, OutputIterator output);

        template<typename ForwardIterator>
        static std::size_t length(ForwardIterator first, ForwardIterator last);

        template<conversion = conversion::strict, typename ForwardIterator, typename OutputIterator>
        static OutputIterator to_utf8(ForwardIterator first, ForwardIterator last, OutputIterator output);

        template<conversion conv = conversion::strict, typename ForwardIterator, typename OutputIterator>
        static OutputIterator to_utf16(ForwardIterator first, ForwardIterator last, OutputIterator output);

        template<conversion conv = conversion::strict, typename ForwardIterator, typename OutputIterator>
        static OutputIterator to_utf32(ForwardIterator first, ForwardIterator last, OutputIterator output);

        template<typename OutputIterator>
        static OutputIterator read(uistream& in, OutputIterator output, byte_order order = byte_order::none);
		
        template<write_bom wrbom = write_bom::yes, typename ForwardIterator>
        static ForwardIterator write(uostream& out, ForwardIterator first, ForwardIterator last, byte_order order = byte_order::none);
    };

    class utf8::iterator
    {
    public:
        using value_type        = codepoint;
        using difference_type   = std::ptrdiff_t;
        using pointer           = void;
        using reference         = codepoint;
        using iterator_category = std::bidirectional_iterator_tag;
        using iterator_type     = utf8::string_type::const_iterator;

    protected:
        iterator_type current;

    public:
        iterator() noexcept;
        explicit iterator(iterator_type it) noexcept;

        iterator(const iterator& rhs) noexcept;
		
        iterator& operator=(const iterator& rhs) noexcept;

        reference operator*() const noexcept;

        iterator& operator++() noexcept;
        iterator operator++(int) noexcept;

        iterator& operator--() noexcept;
        iterator operator--(int) noexcept;

        iterator_type base() const noexcept;
    };

    bool operator==(const utf8::iterator& lhs, const utf8::iterator& rhs) noexcept;
    bool operator!=(const utf8::iterator& lhs, const utf8::iterator& rhs) noexcept;
}
```
* \[Note\] The following restrictions apply to all the member functions and types of `utf32`, `utf16` and `utf8`:
  + The member types (`string_type`, `string_view_type`, `char_type`) as defines as (`std::u32string`, `std::u32string_view`, `std::u32string::value_type`) for `utf32`, (`std::u16string`, `std::u16string_view`, `std::u16string::value_type`) for `utf16` and as (`std::string`, `std::string_view`, `unsigned char`) for `utf8`.
  + `ForwardIterator` is an iterator, which points to a range of unsigned integer of size at least 32 bits for `utf32`, 16 bits for `utf16` and 8 bits for `utf8`.
  + The function `decode` takes a range of unsigned integers and a reference to a codepoint in which the decoded codepoint is stored. It returns an iterator to a new range, yet to be decoded. The provided conversion template parameter specifies whether error handling is done (yes if `conversion::strict`, no if `conversion::lenient`).
  + `OutputIterator` is an iterator, which points to unsigned integers big enough to hold the value, given by the respective function. 
  + The function `encode` encodes the given codepoint into the given `OutputIterator`.
  + The functions `to_utf32`, `to_utf16` and `to_utf8` convert a given range into the respective encoding format, writing the new range into the given `OutputIterator` taking into account the provided `conversion` specifier.
  + The function `read` reads a Unicode file and stores it into the `OutputIterator` taking into account the given `byte_order`. For `utf8` the byte order must be `byte_order::none`, while for `utf16` and `utf32` - either `byte_order::little` or `byte_order::big`. If the wrong byte order is given, nothing is done.
  + The function `write` writes a UTF range into a file, taking into account the given byte order. If the provided `write_bom` template parameter is equal to `write_bom::yes`, then a BOM is written first into the file. If the wrong byte order is given, nothing is done. The function returns an iterator to the last successfully written element.

### An example usage of `utf32`, `utf16` and `utf8`

```c++
#include <blue/unicode/unicode.hpp>

#include <iterator> // for std::back_inserter

namespace uni = blue::unicode;

int main()
{
    // I know the file is UTF-16 BE
    uni::uistream in("utf16.txt");
    if (!in.is_open()) {
        // failed to open the file
    }
    
    uni::utf16::string_type str;
    uni::utf16::read(in, std::back_inserter(str), uni::byte_order::big); // read the file into str
    
    // decoding the utf-16 sequence stored in str
    for (auto i = str.begin(); i != str.end(); ) 
    {
        uni::codepoint cp;
        i = uni::utf16::decode(i, str.end(), cp);
        // work with cp...
    }
    
    // the above decoding can be simplified, as decoding is the same as converting to utf-32
    uni::utf32::string_type u32;
    uni::utf16::to_utf32(str.begin(), str.end(), std::back_inserter(u32));
    // or do a lenient conversion - no error checking
    uni::utf16::to_utf32<uni::conversion::lenient>(str.begin(), str.end(), std::back_inserter(u32));
    
    // write the utf32 sequence u32 to a file
    uni::uostream out("u32.txt");
    if (!out.is_open()) {
        // cannot open file
    }
    // write with little endian byte ordering and don't write BOM
    uni::utf32::write<uni::write_bom::no>(out, u32.begin(), u32.end(), uni::byte_order::little);
    // overwrite with a BOM and big endian
    uni::utf32::write(out, u32.begin(), u32.end(), uni::byte_order::big);
}
```

### Example usage of UTF iterators

The `utf32`, `utf16` and `utf8` classes also provide a member type `iterator`. It acts as a `BidirectionalIterator` over a Unicode sequence. The sequence must be valid, as the iterators don't do any error checking. 

```c++
#include <blue/unicode/unicode.hpp>

namespace uni = blue::unicode;

int main()
{
    uni::utf8::string str = u8"Hello world"; // can use a C++17 u8 literal
    
    for (auto i = uni::utf8::iterator(str.begin()); i != uni::utf8::iterator(str.end()); ++i) 
    {
        uni::codepoint cp = *i;
        // work with the codepoint
    }
}
```

## Other utilities and helper functions

The library also includes a few helper functions that combine some of the aforementioned.

```c++
namespace blue::unicode
{
    using default_utf = utf16;
    constexpr byte_order default_byte_order = byte_order::big;

    template<typename UTF = default_utf, conversion conv = conversion::strict>
    typename UTF::string_type read_file(uistream& in);

    template<typename UTFO = default_utf, write_bom wrbom = write_bom::yes>
    void write_file(uostream& file, const utf8::string_type& text, byte_order order = byte_order::none);

    template<typename UTFO = default_utf, write_bom wrbom = write_bom::yes>
    void write_file(uostream& file, const utf16::string_type& text, byte_order order = default_byte_order);

    template<typename UTFO = default_utf, write_bom wrbom = write_bom::yes>
    void write_file(uostream& file, const utf32::string_type& text, byte_order order = default_byte_order);

    utf8::iterator make_iterator(const utf8::string_type::const_iterator& it);
    utf16::iterator make_iterator(const utf16::string_type::const_iterator& it);
    utf32::iterator make_iterator(const utf32::string_type::const_iterator& it);
}
```

* The function `read_file` takes two template parameters `UTF` and `conv`. It determines the encoding of the provided file. Reads the entire file, converts it to `UTF` with the given conversion `conv` and returns it as the result. `UTF` must be one of `utf32`, `utf16`, `utf8`. 
* The function `write_file` takes an input Unicode string, converts it to `UTFO` and writes it to the file, taking into account the given `byte_order` and `write_bom`.
* The function `make_iterator` is a shorthand and is best described by the below code snippet.

### Example usage of the utilities

```c++
#include <blue/unicode/unicode.hpp>

namespace uni = blue::unicode;

int main()
{
    uni::uistream in("file.txt"); // unknown encoding
    auto str = uni::read_file(in); // str will be of type default_utf::string_type, which is utf16::string_type
    
    // or 
    auto str2 = uni::read_file<uni::utf8>(in); // str will be utf8::string_type
    
    // iterate the sequences. uni::make_iterator automatically determines the type of the iterator
    for (auto i = uni::make_iterator(str.begin()); i != uni::make_iterator(str.end()); ++i) {
        uni::codepoint cp = *i;
    }
    for (auto i = uni::make_iterator(str2.begin()); i != uni::make_iterator(str2.end()); ++i) {
        uni::codepoint cp = *i;
    }
    
    // write to a file as utf-32
    uni::uostream out("output.txt");
    uni::write_file(out, str); // will be written as UTF-16 big endian which is 'default_byte_order'
}
```

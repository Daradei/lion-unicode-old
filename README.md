# Small Unicode library for C++17

## An ```std::fstream``` wrapped to handle Unicode files

```
namespace blue::unicode
{
	class ustream_base
	{
	protected:
		ustream_base() ;
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

		void close() ;

		uostream& write(const char_type* str, std::streamsize count);

		pos_type tellp() ;
		uostream& seekp(pos_type pos);
		uostream& seekp(off_type off, std::ios_base::seekdir dir);

		uostream& flush() ;
	};

	void swap(uostream& lhs, uostream& rhs);
}
```

## The ```codepoint``` type

```
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

## Unicode encoding forms

```
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

## Handling UTF-32, UTF-16 and UTF-8 

```
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
		using value_type		= codepoint;
		using difference_type	= std::ptrdiff_t;
		using pointer	        = void;
		using reference		    = codepoint;
		using iterator_category = std::bidirectional_iterator_tag;
		using iterator_type		= utf32::string_type::const_iterator;

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

	bool operator==(const utf32::iterator& lhs, const utf32::iterator& rhs) noexcept'
	bool operator!=(const utf32::iterator& lhs, const utf32::iterator& rhs) noexcept;

	class utf16
	{
	public:
		using string_type	   = std::u16string;
		using string_view_type = std::u16string_view;
		using char_type		   = std::u16string::value_type;

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
		using value_type	    = codepoint;
		using difference_type   = std::ptrdiff_t;
		using pointer		    = void;
		using reference	        = codepoint;
		using iterator_category = std::bidirectional_iterator_tag;
		using iterator_type		= utf16::string_type::const_iterator;

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
	
	class utf8
	{
	public:
		using string_type	   = std::string;
		using string_view_type = std::string_view;
		using char_type		   = unsigned char;

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
		using value_type	    = codepoint;
		using difference_type   = std::ptrdiff_t;
		using pointer	        = void;
		using reference		    = codepoint;
		using iterator_category = std::bidirectional_iterator_tag;
		using iterator_type		= utf8::string_type::const_iterator;

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

## Other utilities and helper functions

```
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

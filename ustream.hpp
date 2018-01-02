#ifndef BLUE_UNICODE_USTREAM_HPP
#define BLUE_UNICODE_USTREAM_HPP

#include <fstream>
#include <ios>
#include <string>
#include <utility>

namespace blue::unicode
{
	class ustream_base
	{
	protected:
		std::fstream stream;

		ustream_base() 
			: stream()
		{}

		explicit ustream_base(const char* file, std::ios::openmode mode = std::ios::binary)
			: stream(file, mode | std::ios_base::binary)
		{}

		explicit ustream_base(const std::string& file, std::ios::openmode mode = std::ios::binary)
			: ustream_base(file.c_str(), mode)
		{}

		void swap(ustream_base& rhs) {
			stream.swap(rhs.stream);
		}

	public:
		using char_type   = std::fstream::char_type;
		using traits_type = std::fstream::traits_type;
		using int_type    = std::fstream::int_type;
		using pos_type    = std::fstream::pos_type;
		using off_type    = std::fstream::off_type;

		ustream_base(const ustream_base&) = delete;
		ustream_base& operator=(const ustream_base&) = delete;

		virtual ~ustream_base() = default;

		bool good() const { return stream.good(); }
		bool eof() const { return stream.eof(); }
		bool fail() const { return stream.fail(); }
		bool bad() const { return stream.bad(); }

		bool operator!() const {
			return !stream;
		}

		explicit operator bool() const {
			return stream.operator bool();
		}

		std::ios::iostate rdstate() const {
			return stream.rdstate();
		}

		void setstate(std::ios::iostate state) {
			stream.setstate(state);
		}

		void clear(std::ios::iostate state = std::ios::goodbit) {
			stream.clear(state);
		}
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
		// ifstream members
		uistream()
			: ustream_base()
		{}

		explicit uistream(const char* file, std::ios::openmode mode = std::ios::in)
			: ustream_base(file, mode | std::ios::in)
		{}

		explicit uistream(const std::string& file, std::ios::openmode mode = std::ios::in)
			: uistream(file.c_str(), mode)
		{}

		uistream(const uistream&) = delete;
		uistream& operator=(const uistream&) = delete;

		uistream(uistream&&) = default;
		uistream& operator=(uistream&&) = default;

		void swap(uistream& rhs) {
			ustream_base::swap(rhs);
		}

		bool is_open() const {
			return stream.is_open();
		}

		void open(const char* file, std::ios::openmode mode = std::ios::in | std::ios::binary) {
			stream.open(file, mode | std::ios::in | std::ios::binary);
		}

		void open(const std::string& file, std::ios::openmode mode = std::ios::in | std::ios::binary) {
			stream.open(file, mode | std::ios::in | std::ios::binary);
		}

		void close() {
			stream.close();
		}

		// istream members
		uistream& read(char_type* str, std::streamsize count)
		{
			stream.read(str, count);
			return *this;
		}

		std::streamsize gcount() const {
			return stream.gcount();
		}

		pos_type tellg() {
			return stream.tellg();
		}

		uistream& seekg(pos_type pos) 
		{
			stream.seekg(pos);
			return *this;
		}

		uistream& seekg(off_type off, std::ios_base::seekdir dir)
		{
			stream.seekg(off, dir);
			return *this;
		}
	};

	inline void swap(uistream& lhs, uistream& rhs) {
		lhs.swap(rhs);
	}

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
		// ofstream members
		uostream()
			: ustream_base()
		{}

		explicit uostream(const char* file, std::ios::openmode mode = std::ios::out)
			: ustream_base(file, mode | std::ios::out)
		{}

		explicit uostream(const std::string& file, std::ios::openmode mode = std::ios::out)
			: uostream(file.c_str(), mode)
		{}

		uostream(const uostream&) = delete;
		uostream& operator=(const uostream&) = delete;

		uostream(uostream&&) = default;
		uostream& operator=(uostream&&) = default;

		void swap(uostream& rhs) {
			ustream_base::swap(rhs);
		}

		bool is_open() const {
			return stream.is_open();
		}

		void open(const char* file, std::ios::openmode mode = std::ios::out | std::ios::binary) {
			stream.open(file, mode | std::ios::out | std::ios::binary);
		}

		void open(const std::string& file, std::ios::openmode mode = std::ios::out | std::ios::binary) {
			stream.open(file, mode | std::ios::out | std::ios::binary);
		}

		void close() {
			stream.close();
		}

		// ostream members
		uostream& write(const char_type* str, std::streamsize count)
		{
			stream.write(str, count);
			return *this;
		}

		pos_type tellp() {
			return stream.tellp();
		}

		uostream& seekp(pos_type pos)
		{
			stream.seekp(pos);
			return *this;
		}

		uostream& seekp(off_type off, std::ios_base::seekdir dir)
		{
			stream.seekp(off, dir);
			return *this;
		}

		uostream& flush() 
		{
			stream.flush();
			return *this;
		}
	};

	inline void swap(uostream& lhs, uostream& rhs) {
		lhs.swap(rhs);
	}
}

#endif

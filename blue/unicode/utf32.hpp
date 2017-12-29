#ifndef BLUE_UNICODE_UTF32_HPP
#define BLUE_UNICODE_UTF32_HPP

#include "codepoint.hpp"
#include "encoding.hpp"
#include "ustream.hpp"

#include <string>
#include <string_view>
#include <cstddef>
#include <iterator>
#include <algorithm>

namespace blue::unicode
{
	class utf8;
	class utf16;

	class utf32
	{
	public:
		using string_type      = std::u32string;
		using string_view_type = std::u32string_view;
		using char_type        = std::u32string::value_type;

		class iterator;

		template<conversion conv = conversion::strict, typename ForwardIterator>
		static ForwardIterator decode(ForwardIterator first, ForwardIterator last, codepoint& cp)
		{
			if constexpr(conv == conversion::strict) {
				cp = is_valid(*first) ? *first : replacement_character();
			}
			else if constexpr(conv == conversion::lenient) {
				cp = *first;
			}
			return ++first;
		}

		template<typename OutputIterator>
		static OutputIterator encode(codepoint cp, OutputIterator output)
		{
			*output = cp;
			return ++output;
		}

		template<typename ForwardIterator>
		static std::size_t length(ForwardIterator first, ForwardIterator last) {
			return std::distance(first, last);
		}

		template<conversion = conversion::strict, typename ForwardIterator, typename OutputIterator>
		static OutputIterator to_utf8(ForwardIterator first, ForwardIterator last, OutputIterator output)
		{
			for (; first != last; ++first) {
				output = utf8::encode(*first, output);
			}
			return output;
		}

		template<conversion = conversion::strict, typename ForwardIterator, typename OutputIterator>
		static OutputIterator to_utf16(ForwardIterator first, ForwardIterator last, OutputIterator output)
		{
			for (; first != last; ++first) {
				output = utf16::encode(*first, output);
			}
			return output;
		}

		template<conversion = conversion::strict, typename ForwardIterator, typename OutputIterator>
		static OutputIterator to_utf32(ForwardIterator first, ForwardIterator last, OutputIterator output) {
			return std::copy(first, last, output);
		}

		template<typename OutputIterator>
		static OutputIterator read(uistream& in, OutputIterator output, byte_order order)
		{
			if (order == byte_order::none) {
				return output;
			}

			std::string text;
			in.seekg(0, std::ios::end);
			if (in.tellg() % 4 == 0) {
				text.resize(in.tellg());
			}
			else {
				text.resize(((in.tellg() / 4) + 1) * 4);
			}
			
			in.seekg(0, std::ios::beg);
			in.read(&text[0], text.size());

			if (order == byte_order::little)
			{
				for (std::string::size_type i = 0; i < text.size(); i += 4)
				{
					const codepoint bytes[] = {
						static_cast<unsigned char>(text[i + 3]),
						static_cast<unsigned char>(text[i + 2]),
						static_cast<unsigned char>(text[i + 1]),
						static_cast<unsigned char>(text[i])
					};
					*output++ = (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
				}
			}
			else if (order == byte_order::big)
			{
				for (std::string::size_type i = 0; i < text.size(); i += 4)
				{
					const codepoint bytes[] = {
						static_cast<unsigned char>(text[i]),
						static_cast<unsigned char>(text[i + 1]),
						static_cast<unsigned char>(text[i + 2]),
						static_cast<unsigned char>(text[i + 3])
					};
					*output++ = (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
				}
			}
			return output;
		}

		template<write_bom wrbom = write_bom::yes, typename ForwardIterator>
		static ForwardIterator write(uostream& out, ForwardIterator first, ForwardIterator last, byte_order order)
		{
			if (order == byte_order::none) {
				return first;
			}

			std::string towrite;
			if (order == byte_order::little)
			{
				for (ForwardIterator it = first; it != last; ++it)
				{
					const char bytes[] = {
						static_cast<char>(*it & 0x000000FF),
						static_cast<char>((*it & 0x0000FF00) >> 8),
						static_cast<char>((*it & 0x00FF0000) >> 16),
						static_cast<char>(*it >> 24)
					};
					towrite.append(bytes, 4);
				}
				if constexpr(wrbom == write_bom::yes)
				{
					if (towrite.compare(0, 4, constants::UTF32_LE_BOM) != 0) {
						out.write(constants::UTF32_LE_BOM.data(), 4);
					}
				}
			}
			else if (order == byte_order::big)
			{
				for (ForwardIterator it = first; it != last; ++it)
				{
					const char bytes[] = {
						static_cast<char>(*it >> 24),
						static_cast<char>((*it & 0x00FF0000) >> 16),
						static_cast<char>((*it & 0x0000FF00) >> 8),
						static_cast<char>(*it & 0x000000FF)
					};
					towrite.append(bytes, 4);
				}
				if constexpr(wrbom == write_bom::yes)
				{
					if (towrite.compare(0, 4, constants::UTF32_BE_BOM) != 0) {
						out.write(constants::UTF32_BE_BOM.data(), 4);
					}
				}
			}
			out.write(&towrite[0], towrite.size());
			return std::next(first, out.tellp() / 4 - (wrbom == write_bom::yes ? 4 : 0));
		}

		template<typename ForwardIterator>
		static ForwardIterator valid_sequence(ForwardIterator first, ForwardIterator last)
		{
			while (first != last)
			{
				ForwardIterator temp = first;

				codepoint cp;
				first = decode(first, last, cp);
				if (cp == replacement_character()) {
					return temp;
				}
			}
			return first;
		}
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
		iterator() noexcept
			: current()
		{}

		explicit iterator(iterator_type it) noexcept
			: current(it)
		{}

		iterator(const iterator& rhs) noexcept
			: current(rhs.current)
		{}

		iterator& operator=(const iterator& rhs) noexcept
		{
			current = rhs.current;
			return *this;
		}

		reference operator*() const noexcept {
			return *current;
		}

		iterator& operator++() noexcept
		{
			++current;
			return *this;
		}

		iterator operator++(int) noexcept
		{
			iterator temp(*this);
			++*this;
			return temp;
		}

		iterator& operator--() noexcept
		{
			--current;
			return *this;
		}

		iterator operator--(int) noexcept
		{
			iterator temp(*this);
			--*this;
			return temp;
		}

		iterator_type base() const noexcept {
			return current;
		}
	};

	inline bool operator==(const utf32::iterator& lhs, const utf32::iterator& rhs) noexcept {
		return lhs.base() == rhs.base();
	}
	inline bool operator!=(const utf32::iterator& lhs, const utf32::iterator& rhs) noexcept {
		return lhs.base() != rhs.base();
	}
}

#endif

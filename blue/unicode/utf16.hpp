#ifndef BLUE_UNICODE_UTF16_HPP
#define BLUE_UNICODE_UTF16_HPP

#include "codepoint.hpp"
#include "encoding.hpp"
#include "ustream.hpp"

#include <string>
#include <string_view>
#include <cstddef>
#include <algorithm>
#include <iterator>

namespace blue::unicode
{
	class utf8;

	class utf16
	{
	public:
		using string_type	   = std::u16string;
		using string_view_type = std::u16string_view;
		using char_type		   = std::u16string::value_type;

		class iterator;

		template<conversion conv = conversion::strict, typename ForwardIterator>
		static ForwardIterator decode(ForwardIterator first, ForwardIterator last, codepoint& cp)
		{
			if constexpr(conv == conversion::strict)
			{
				const char_type one = *first++;
				if (is_high_surrogate(one))
				{
					if (first != last)
					{
						const char_type two = *first;
						if (is_low_surrogate(two)) 
						{
							++first;
							cp = static_cast<codepoint>(((one - 0xD800) << 10) + (two - 0xDC00) + 0x0010000);
						}
						else {
							cp = replacement_character();
						}
					}
					else {
						cp = replacement_character();
					}
				}
				else {
					cp = is_valid(one) ? one : replacement_character();
				}
			}
			else if constexpr(conv == conversion::lenient)
			{
				if (const char_type one = *first++; is_high_surrogate(one))
				{
					const char_type two = *first++;
					cp = static_cast<codepoint>(((one - 0xD800) << 10) + (two - 0xDC00) + 0x0010000);
				}
				else {
					cp = one;
				}
			}
			return first;
		}

		template<typename OutputIterator>
		static OutputIterator encode(codepoint cp, OutputIterator output)
		{
			if (cp <= 0xFFFF) {
				*output++ = static_cast<char_type>(cp);
			}
			else
			{
				cp -= 0x0010000;
				*output++ = static_cast<char_type>((cp >> 10) + 0xD800);
				*output++ = static_cast<char_type>((cp & 0x3FF) + 0xDC00);
			}
			return output;
		}

		template<typename ForwardIterator>
		static std::size_t length(ForwardIterator first, ForwardIterator last)
		{
			std::size_t len = 0;
			while (first != last)
			{
				codepoint cp;
				first = decode<conversion::lenient>(first, last, cp);
				++len;
			}
			return len;
		}

		template<conversion conv = conversion::strict, typename ForwardIterator, typename OutputIterator>
		static OutputIterator to_utf8(ForwardIterator first, ForwardIterator last, OutputIterator output)
		{
			while (first != last)
			{
				codepoint cp;
				first = decode<conv>(first, last, cp);
				output = utf8::encode(cp, output);
			}
			return output;
		}

		template<conversion = conversion::strict, typename ForwardIterator, typename OutputIterator>
		static OutputIterator to_utf16(ForwardIterator first, ForwardIterator last, OutputIterator output) {
			return std::copy(first, last, output);
		}

		template<conversion conv = conversion::strict, typename ForwardIterator, typename OutputIterator>
		static OutputIterator to_utf32(ForwardIterator first, ForwardIterator last, OutputIterator output)
		{
			while (first != last)
			{
				codepoint cp;
				first = decode<conv>(first, last, cp);
				*output++ = cp;
			}
			return output;
		}

		template<typename OutputIterator>
		static OutputIterator read(uistream& in, OutputIterator output, byte_order order)
		{
			if (order == byte_order::none) {
				return output;
			}

			std::string text;
			in.seekg(0, std::ios::end);
			text.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&text[0], text.size());

			if (order == byte_order::little)
			{
				for (std::string::size_type i = 0; i < text.size(); i += 2)
				{
					const char_type bytes[] = {
						static_cast<unsigned char>(text[i + 1]),
						static_cast<unsigned char>(text[i])
					};
					*output++ = (bytes[0] << 8) | bytes[1];
				}
			}
			else if (order == byte_order::big)
			{
				for (std::string::size_type i = 0; i < text.size(); i += 2)
				{
					const char_type bytes[] = {
						static_cast<unsigned char>(text[i]),
						static_cast<unsigned char>(text[i + 1])
					};
					*output++ = (bytes[0] << 8) | bytes[1];
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
						static_cast<char>(*it & 0x00FF),
						static_cast<char>((*it & 0xFF00) >> 8)
					};
					towrite.append(bytes, 2);
				}
				if constexpr(wrbom == write_bom::yes)
				{
					if (towrite.compare(0, 2, constants::UTF16_LE_BOM) != 0) {
						out.write(constants::UTF16_LE_BOM.data(), 2);
					}
				}
			}
			else if (order == byte_order::big)
			{
				for (ForwardIterator it = first; it != last; ++it)
				{
					const char bytes[] = {
						static_cast<char>((*it & 0xFF00) >> 8),
						static_cast<char>(*it & 0x00FF)
					};
					towrite.append(bytes, 2);
				}
				if constexpr(wrbom == write_bom::yes)
				{
					if (towrite.compare(0, 2, constants::UTF16_BE_BOM) != 0) {
						out.write(constants::UTF16_BE_BOM.data(), 2);
					}
				}
			}
			out.write(&towrite[0], towrite.size());
			return std::next(first, out.tellp() / 2 - (wrbom == write_bom::yes ? 2 : 0));
		}

		template<typename ForwardIterator>
		static bool is_valid(ForwardIterator first, ForwardIterator last)
		{
			while (first != last)
			{
				codepoint cp;
				first = decode(first, last, cp);
				if (cp == replacement_character()) {
					return false;
				}
			}
			return true;
		}
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

		reference operator*() const noexcept
		{
			value_type cp = 0;

			const char_type one = *current;
			if (one >= 0xD800 && one <= 0xDBFF)
			{
				const char_type two = *std::next(current);
				cp = static_cast<reference>(((one - 0xD800) << 10) + (two - 0xDC00) + 0x0010000);
			}
			else {
				cp = one;
			}

			return cp;
		}

		iterator& operator++() noexcept
		{
			const char_type one = *current;
			++current;
			if (one >= 0xD800 && one <= 0xDBFF) {
				++current;
			}
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
			const char_type two = *current;
			if (two >= 0xDC00 && two <= 0xDFFF) {
				--current;
			}
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

	inline bool operator==(const utf16::iterator& lhs, const utf16::iterator& rhs) noexcept {
		return lhs.base() == rhs.base();
	}
	inline bool operator!=(const utf16::iterator& lhs, const utf16::iterator& rhs) noexcept {
		return lhs.base() != rhs.base();
	}
}

#endif

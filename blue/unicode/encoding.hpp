#ifndef BLUE_UNICODE_ENCODING_HPP
#define BLUE_UNICODE_ENCODING_HPP

#include "ustream.hpp"

#include <string_view>
#include <string>
#include <cstddef>

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

		static encoding get(uistream& in)
		{
			char bytes[4] = { 0, 0, 0, 0 };
			in.read(bytes, 4);

			// in.read() will set failbit if there are less than 4 bytes in the file
			in.clear();

			if (constants::UTF32_LE_BOM.compare(0, 4, bytes, 4) == 0) {
				return encoding{ format::utf32, byte_order::little };
			}
			else if (constants::UTF32_BE_BOM.compare(0, 4, bytes, 4) == 0) {
				return encoding{ format::utf32, byte_order::big };
			}
			else if (constants::UTF16_LE_BOM.compare(0, 2, bytes, 2) == 0) {
				return encoding{ format::utf16, byte_order::little };
			}
			else if (constants::UTF16_BE_BOM.compare(0, 2, bytes, 2) == 0) {
				return encoding{ format::utf16, byte_order::big };
			}
			else if (constants::UTF8_BOM.compare(0, 3, bytes, 3) == 0) {
				return encoding{ format::utf8 };
			}
			else
			{
				std::string segment;
				in.seekg(0, std::ios::end);
				if (in.tellg() <= 100) {
					segment.resize((in.tellg() / 4) * 4); // previous divisible by 4
				}
				else {
					segment.resize(100);
				}

				in.seekg(0, std::ios::beg);
				in.read(&segment[0], segment.size());
				in.seekg(0, std::ios::beg);

				if (segment.find('\0') == std::string::npos) {
					return encoding{ format::utf8 };
				}
				if (segment.find("\0\0", 0, 2) == std::string::npos)
				{
					bool big = true;
					for (std::string::size_type i = 0; i < segment.size(); ++i)
					{
						if (segment[i] == 0)
						{
							if (i % 2 == 0) {
								big = true;
							}
							else {
								big = false;
							}
						}
					}
					return encoding{ format::utf16, big ? byte_order::big : byte_order::little }; 
				}
				else
				{
					bool big = true;
					for (std::string::size_type i = 0; i < segment.size(); i += 4)
					{
						if ((!segment[i + 0] && !segment[i + 1] && !segment[i + 2]) ||
							(!segment[i + 0] && !segment[i + 1]) ||
							(!segment[i + 0]))
						{
							big = true;
						}
						else {
							big = false;
						}
					}
					return encoding{ format::utf32, big ? byte_order::big : byte_order::little };
				}
			}
			return encoding{};
		}
	};
}

#endif

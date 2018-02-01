#ifndef LION_UNICODE_CODEPOINT_HPP
#define LION_UNICODE_CODEPOINT_HPP

namespace lion::unicode
{
	using codepoint = char32_t;

	constexpr codepoint replacement_character() noexcept { return 0x0000FFFD; }

	constexpr codepoint codepoint_min() noexcept { return 0; }
	constexpr codepoint codepoint_max() noexcept { return 0x0010FFFF; }
	
	constexpr bool is_high_surrogate(codepoint cp) noexcept {
		return cp >= 0xD800 && cp <= 0xDBFF;
	}

	constexpr bool is_low_surrogate(codepoint cp) noexcept {
		return cp >= 0xDC00 && cp <= 0xDFFF;
	}

	constexpr bool is_surrogate(codepoint cp) noexcept {
		return (cp >= 0xD800 && cp <= 0xDFFF);
	}

	constexpr bool is_valid(codepoint cp) noexcept {
		return !is_surrogate(cp) && cp <= codepoint_max();
	}

	constexpr bool is_BOM(codepoint cp) noexcept {
		return cp == 0x0000FEFF;
	}
}

#endif

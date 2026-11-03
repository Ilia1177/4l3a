#ifndef WORD_SANITIZER_HPP
#define WORD_SANITIZER_HPP

#include <string>
#include <string_view>

[[nodiscard]] std::string sanitize_word(std::string_view raw);

#endif  // WORD_SANITIZER_HPP

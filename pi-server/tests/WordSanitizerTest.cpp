#include <gtest/gtest.h>

#include <string>

#include "WordSanitizer.hpp"

TEST(WordSanitizerTest, EmptyInputReturnsEmptyString) { EXPECT_EQ(sanitize_word(""), ""); }

TEST(WordSanitizerTest, WhitespaceOnlyInputReturnsEmptyString) {
    EXPECT_EQ(sanitize_word(" \t\r\n"), "");
}

TEST(WordSanitizerTest, ExtractsFirstWhitespaceSeparatedToken) {
    EXPECT_EQ(sanitize_word("  hello world  "), "hello");
}

TEST(WordSanitizerTest, RemovesSurroundingQuotes) {
    EXPECT_EQ(sanitize_word("\"'hello'\""), "hello");
}

TEST(WordSanitizerTest, RemovesNonPrintableCharacters) {
    EXPECT_EQ(sanitize_word(std::string("he\x01llo")), "hello");
}

TEST(WordSanitizerTest, TruncatesToTwentyFourCharacters) {
    EXPECT_EQ(sanitize_word(std::string(30, 'a')), std::string(24, 'a'));
}

TEST(WordSanitizerTest, PreservesPrintablePunctuation) {
    EXPECT_EQ(sanitize_word("a-b_c!"), "a-b_c!");
}

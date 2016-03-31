#include <limits.h>
#include "gtest/gtest.h"
#include "AnsiAwareString.h"

TEST(AnsiAwareString, canConstructStringFromLiteralString) {
  AnsiAwareString string("some string");

  EXPECT_EQ("some string", string.get());
}

TEST(AnsiAwareString, lengthOfEmptyString) {
  AnsiAwareString string;

  EXPECT_EQ(0, string.length());
  EXPECT_EQ(0, string.size());
}

TEST(AnsiAwareString, lengthOfPlainString) {
  AnsiAwareString string("123456789012345");

  EXPECT_EQ(15, string.length());
  EXPECT_EQ(15, string.size());
}

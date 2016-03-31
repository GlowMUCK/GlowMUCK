#include <string>
#include "AnsiAwareString.h"

AnsiAwareString::AnsiAwareString() {
}

AnsiAwareString::AnsiAwareString(std::string string) {
  this->string = string;
}

std::string AnsiAwareString::get() {
  return string;
}

size_t AnsiAwareString::length() {
  return string.length();
}

size_t AnsiAwareString::size() {
  return this->length();
}

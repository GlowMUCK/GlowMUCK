#ifndef ANSI_AWARE_STRING_H
#define ANSI_AWARE_STRING_H
class AnsiAwareString {
 public:
  AnsiAwareString();
  AnsiAwareString(std::string);
  std::string get();
  size_t length();
  size_t size();
 private:
  std::string string;
};
#endif

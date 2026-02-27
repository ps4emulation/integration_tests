#include "log.h"

#include <iomanip>
#include <sstream>

std::ostream& center(std::ostream& os, const std::string& s, int width) {
  int len = (int)s.size();
  if (width <= len) return os << s.substr(0, width);
  int left  = (width - len) / 2;
  int right = width - len - left;
  return os << std::string(left, ' ') << s << std::string(right, ' ');
}

std::ostream& right(std::ostream& os, const std::string& s, int width) {
  int len = (int)s.size();
  if (width <= len) return os << s.substr(0, width);
  int left = (width - len);
  return os << std::string(left, ' ') << s;
}

std::string center(const std::string& s, int width) {
  int len = (int)s.size();
  if (width <= len) return s.substr(0, width);
  int left  = (width - len) / 2;
  int right = width - len - left;
  return std::string(left, ' ') + s + std::string(right, ' ');
}

std::string right(const std::string& s, int width) {
  int len = (int)s.size();
  if (width <= len) return s.substr(0, width);
  int left = (width - len);
  return std::string(left, ' ') + s;
}

std::string to_octal(int value) {
  std::ostringstream oss;
  oss << std::oct << value;
  return oss.str();
}

std::string to_hex(int value) {
  std::ostringstream oss;
  oss << std::hex << value;
  return oss.str();
}
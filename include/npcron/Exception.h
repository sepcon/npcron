#pragma once

#include <exception>
#include <string>

namespace npcron {

class BadSyntaxException : public std::exception {
 public:
  static void raise(std::string reason) {
    throw BadSyntaxException(std::move(reason));
  }
  BadSyntaxException(std::string&& what) { _what = std::move(what); }
  const char* what() const noexcept override { return _what.c_str(); }

 private:
  std::string _what;
};

}  // namespace npcron

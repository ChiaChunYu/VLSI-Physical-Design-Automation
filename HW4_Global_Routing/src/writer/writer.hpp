#pragma once
#include <string>

#include "../design/design.hpp"

class Design;

class Writer {
 public:
  Writer() = delete;
  Writer(const Writer&) = delete;
  Writer& operator=(const Writer&) = delete;

  static void WriteOutput(const std::string& filename, const Design& design);
};

#pragma once
#include "../design/design.hpp"

class Parser {
 public:
  Parser() = delete;
  Parser(const Parser&) = delete;
  Parser& operator=(const Parser&) = delete;

  static void Parse(const std::string& filename, Design& design);
};
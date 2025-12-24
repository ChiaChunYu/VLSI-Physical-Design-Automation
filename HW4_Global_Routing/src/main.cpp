#include <iostream>
#include <stdexcept>
#include <string>
#include <chrono>

#include "design/design.hpp"
#include "global_router/global_router.hpp"
#include "parser/parser.hpp"
#include "writer/writer.hpp"

int main(int argc, char* argv[]) {
  auto start_time = std::chrono::steady_clock::now();
  if (argc < 3) {
    throw std::runtime_error("Usage: ./hw4 <input_file> <output_file>");
  }
  std::string input_file = argv[1];
  std::string output_file = argv[2];
  Design design;
  Parser::Parse(input_file, design);
  GlobalRouter global_router(design, start_time);
  global_router.Route();
  Writer::WriteOutput(output_file, design);
  auto end_time = std::chrono::steady_clock::now();
  std::chrono::duration<double> elapsed_seconds = end_time - start_time;
  std::cout << "Total Elapsed Time: " << elapsed_seconds.count() << " seconds"
            << std::endl;
  return 0;
}
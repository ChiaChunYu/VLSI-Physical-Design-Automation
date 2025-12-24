#include <iostream>
#include <omp.h>
#include <algorithm>
#include "design/design.hpp"
#include "parser/parser.hpp"
#include "result_writer/result_writer.hpp"
#include "detailed_placer/detailed_placer.hpp"
#include "debugger/debugger.hpp"

int main(int argc, char *argv[])
{
    std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <input LEF> <input DEF> <output DEF>" << std::endl;
        return 1;
    }
    int max_threads = omp_get_max_threads();
    int optimal_threads = std::min(max_threads, 16);
    omp_set_num_threads(optimal_threads);
    std::string lef_filename = argv[1];
    std::string def_filename = argv[2];
    std::string output_def_filename = argv[3];
    Design design;
    DetailedPlacer detailed_placer(design, start_time);
    Parser::parse(lef_filename, def_filename, design);
    detailed_placer.run();
    ResultWriter::write_def(output_def_filename, design );
    return 0;
}
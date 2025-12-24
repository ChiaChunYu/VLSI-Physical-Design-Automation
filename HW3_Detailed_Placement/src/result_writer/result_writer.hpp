#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include "../design/design.hpp"

class ResultWriter
{
public:
    static void write_def(const std::string &filename, const Design &design);
};
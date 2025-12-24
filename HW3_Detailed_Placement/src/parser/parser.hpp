#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include "../design/design.hpp"

enum class LefParserState
{
    IDLE,
    IN_UNITS,
    IN_SITE,
    IN_MACRO
};

enum class DefParserState
{
    IDLE,
    IN_DESIGN,
    IN_COMPONENTS,
    IN_PINS,
    IN_NETS
};

class Parser
{
    static void parse_lef(const std::string &filename, Design &design);
    static void parse_def(const std::string &filename, Design &design);

public:
    static void parse(const std::string &lef_filename, const std::string &def_filename, Design &design);
};
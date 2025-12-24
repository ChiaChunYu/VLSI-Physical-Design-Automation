#pragma once
#include "../design/design.hpp"

class Debugger
{
    static void check_overlap(const Design &design);
    static void check_row_boundary(const Design &design);
    static void check_site_alignment(const Design &design);
    static void check_orientation(const Design &design);

public:
    static void run(const Design &design);
};
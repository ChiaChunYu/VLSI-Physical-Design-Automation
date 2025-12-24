#include "debugger.hpp"
#include <iostream>

void Debugger::check_overlap(const Design &design)
{
    int total_errors = 0;
    for (const Row *row : design.rows)
    {
        if (row->components.empty())
            continue;
        int size = static_cast<int>(row->components.size());
        for (int i = 0; i < size - 1; ++i)
        {
            Component *curr = row->components[i];
            Component *next = row->components[i + 1];
            int curr_end_x = curr->x + curr->width;
            int next_start_x = next->x;
            if (curr_end_x > next_start_x)
            {
                std::cout << "[Overlap Error] Row " << row->orig_y
                          << ": " << curr->name << " (" << curr_end_x << ")"
                          << " overlaps with " << next->name << " (" << next_start_x << ")"
                          << std::endl;
                total_errors++;
            }
        }
    }
    if (total_errors > 0)
        std::cout << "Total Overlap Errors: " << total_errors << std::endl;
}

void Debugger::check_row_boundary(const Design &design)
{
    int total_errors = 0;
    for (const Row *row : design.rows)
    {
        for (const Component *comp : row->components)
        {
            if (comp->is_fixed || comp->type == "PIN")
                continue;
            if (comp->site_idx < 0 || comp->site_idx + comp->width_sites > row->num_sites)
            {
                std::cout << "[Boundary Error] " << comp->name
                          << " at site " << comp->site_idx
                          << " (Row limit: " << row->num_sites << ")"
                          << std::endl;
                total_errors++;
            }
        }
    }
    if (total_errors > 0)
        std::cout << "Total Boundary Errors: " << total_errors << std::endl;
}

void Debugger::check_site_alignment(const Design &design)
{
    int alignment_errors = 0;
    int sync_errors = 0;
    for (const Row *row : design.rows)
    {
        for (const Component *comp : row->components)
        {
            if (comp->is_fixed || comp->type == "PIN")
                continue;
            int relative_x = comp->x - row->orig_x;
            if (relative_x % row->step_x != 0)
            {
                std::cout << "[Alignment Error] " << comp->name
                          << " at x=" << comp->x
                          << " (Row orig=" << row->orig_x << ", step=" << row->step_x << ")"
                          << std::endl;
                alignment_errors++;
            }
            int calculated_x = row->orig_x + comp->site_idx * row->step_x;
            if (comp->x != calculated_x)
            {
                std::cout << "[Sync Error] " << comp->name
                          << " stored x=" << comp->x
                          << " but site_idx(" << comp->site_idx << ") -> " << calculated_x
                          << std::endl;
                sync_errors++;
            }
        }
    }
    if (alignment_errors > 0)
        std::cout << "Total Alignment Errors: " << alignment_errors << std::endl;
    if (sync_errors > 0)
        std::cout << "Total Sync Errors: " << sync_errors << std::endl;
}

void Debugger::check_orientation(const Design &design)
{
    int total_errors = 0;
    for (const Row *row : design.rows)
    {
        for (const Component *comp : row->components)
        {
            if (comp->is_fixed || comp->type == "PIN")
                continue;
            if (comp->orientation != row->orientation)
            {
                std::cout << "[Orientation Error] " << comp->name
                          << " has " << comp->orientation
                          << " but Row " << row->orig_y
                          << " requires " << row->orientation << std::endl;
                total_errors++;
            }
        }
    }
    if (total_errors > 0)
        std::cout << "Total Orientation Errors: " << total_errors << std::endl;
}

void Debugger::run(const Design &design)
{
    check_overlap(design);
    check_row_boundary(design);
    check_site_alignment(design);
    check_orientation(design);
}
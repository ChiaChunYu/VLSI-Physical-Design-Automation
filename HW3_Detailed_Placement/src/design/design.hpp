#pragma once
#include <string>
#include <vector>
#include <unordered_map>

class Net;
class Component;
class Row;

struct Unit
{
public:
    double dbu_per_micron;
};

struct Site
{
public:
    std::string name;
    std::string type;
    double width;
    double height;
};

struct Macro
{
public:
    std::string type;
    std::string name;
    std::string site_name;
    double width;
    double height;
};

struct Rect
{
    int min_x, max_x, min_y, max_y;
    bool valid; 
};

class Component
{
public:
    std::string name;
    std::string macro_name;
    std::string type; // e.g "PIN", "CORE", "BLOCK" ..
    std::string orientation;
    std::string site_name;
    int width_sites;
    int row_idx;
    int site_idx;
    int idx_in_row;
    int x;
    int y;
    int width;
    int height;
    bool is_fixed;
    std::vector<Net *> nets;

    void swap_position(Component *other, std::vector<Row *> &rows);
};

class Net
{
public:
    std::string name;
    std::vector<Component *> components;

    long long int get_HPWL() const;
    long long int get_swap_HPWL(const Component *comp_a, const Component *comp_b) const;
    long long int get_move_HPWL(const Component *target_comp, int new_x, int new_y) const;
    Rect get_boundary_excluding(const Component* excluded_comp) const;
};

class Row
{
public:
    std::string name;
    std::string site_name;
    std::string orientation;
    int num_sites; // horizontal site number
    int orig_x;
    int orig_y;
    int step_x;
    int step_y;
    std::vector<Component *> components;

    void remove_component(Component *comp);
    void insert_component(Component *comp, int target_site_idx);

    int get_left_limit(int idx_in_row) const;
    int get_right_limit(int idx_in_row) const;

    inline int get_x(int site_idx) const { return orig_x + site_idx * step_x; }
    inline int get_site_idx(int x) const { return std::max(0, std::min(num_sites - 1, (int)((x - orig_x) / step_x))); }
};

class Design
{
public:
    Unit unit;
    std::vector<std::string> def_file;
    std::vector<Row *> rows;
    std::vector<Component *> components;
    std::vector<Net *> nets;
    std::unordered_map<std::string, Site *> site_map;
    std::unordered_map<std::string, Macro *> macro_map;
    std::unordered_map<std::string, Component *> component_map;
    
    long long int get_total_HPWL() const;
};

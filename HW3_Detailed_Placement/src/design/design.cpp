#include "design.hpp"
#include <limits>
#include <algorithm>

void Component::swap_position(Component *other, std::vector<Row *> &rows)
{
    if (!other)
        return;
    if (this == other || this->is_fixed || other->is_fixed)
        return;
    Row *this_row = rows[this->row_idx];
    Row *other_row = rows[other->row_idx];
    std::swap(this_row->components[this->idx_in_row], other_row->components[other->idx_in_row]);
    std::swap(this->row_idx, other->row_idx);
    std::swap(this->idx_in_row, other->idx_in_row);
    std::swap(this->site_idx, other->site_idx);
    std::swap(this->x, other->x);
    std::swap(this->y, other->y);
    std::swap(this->orientation, other->orientation);
}

long long int Net::get_HPWL() const
{
    if (components.empty())
        return 0;
    int min_x = std::numeric_limits<int>::max();
    int max_x = std::numeric_limits<int>::min();
    int min_y = std::numeric_limits<int>::max();
    int max_y = std::numeric_limits<int>::min();
    for (const Component *comp : components)
    {
        min_x = std::min(min_x, comp->x);
        max_x = std::max(max_x, comp->x);
        min_y = std::min(min_y, comp->y);
        max_y = std::max(max_y, comp->y);
    }
    return static_cast<long long int>(max_x - min_x) + static_cast<long long int>(max_y - min_y);
}

long long int Net::get_move_HPWL(const Component *target_comp, int new_x, int new_y) const
{
    if (components.empty())
        return 0;
    int min_x = std::numeric_limits<int>::max();
    int max_x = std::numeric_limits<int>::min();
    int min_y = std::numeric_limits<int>::max();
    int max_y = std::numeric_limits<int>::min();
    for (const Component *cur_comp : components)
    {
        int x = cur_comp->x;
        int y = cur_comp->y;
        if (cur_comp == target_comp)
        {
            x = new_x;
            y = new_y;
        }
        if (x < min_x)
            min_x = x;
        if (x > max_x)
            max_x = x;
        if (y < min_y)
            min_y = y;
        if (y > max_y)
            max_y = y;
    }
    return static_cast<long long int>((max_x - min_x) + (max_y - min_y));
}

long long int Net::get_swap_HPWL(const Component *comp_a, const Component *comp_b) const
{
    if (components.empty())
        return 0;
    int min_x = std::numeric_limits<int>::max();
    int max_x = std::numeric_limits<int>::min();
    int min_y = std::numeric_limits<int>::max();
    int max_y = std::numeric_limits<int>::min();
    for (const Component *cur_comp : components)
    {
        int x = cur_comp->x;
        int y = cur_comp->y;
        if (cur_comp == comp_a)
        {
            x = comp_b->x;
            y = comp_b->y;
        }
        else if (cur_comp == comp_b)
        {
            x = comp_a->x;
            y = comp_a->y;
        }
        if (x < min_x)
            min_x = x;
        if (x > max_x)
            max_x = x;
        if (y < min_y)
            min_y = y;
        if (y > max_y)
            max_y = y;
    }
    return static_cast<long long int>((max_x - min_x) + (max_y - min_y));
}

Rect Net::get_boundary_excluding(const Component* excluded_comp) const
{
    Rect rect;
    rect.min_x = std::numeric_limits<int>::max();
    rect.max_x = std::numeric_limits<int>::min();
    rect.min_y = std::numeric_limits<int>::max();
    rect.max_y = std::numeric_limits<int>::min();
    rect.valid = false;
    int cnt = 0;
    for (const Component *cur_comp : components)
    {
        if (cur_comp == excluded_comp) continue;
        cnt++;
        if (cur_comp->x < rect.min_x) rect.min_x = cur_comp->x;
        if (cur_comp->x > rect.max_x) rect.max_x = cur_comp->x;
        if (cur_comp->y < rect.min_y) rect.min_y = cur_comp->y;
        if (cur_comp->y > rect.max_y) rect.max_y = cur_comp->y;
    }
    if (cnt > 0) rect.valid = true;
    return rect;
}

void Row::remove_component(Component *comp)
{
    if (!comp)
        return;
    int idx = comp->idx_in_row;
    if (idx >= 0 && idx < static_cast<int>(components.size()) && components[idx] == comp)
    {
        components.erase(components.begin() + idx);
        int size = static_cast<int>(components.size());
        for (int i = idx; i < size; ++i)
        {
            components[i]->idx_in_row = i;
        }
    }
}

void Row::insert_component(Component *comp, int target_site_idx)
{
    if (!comp)
        return;
    auto it = std::lower_bound(components.begin(), components.end(), target_site_idx,
                               [](Component *c, int site)
                               { return c->site_idx < site; });
    int insert_idx = std::distance(components.begin(), it);
    components.insert(it, comp);
    comp->idx_in_row = insert_idx;
    comp->site_idx = target_site_idx;
    comp->x = this->get_x(target_site_idx);
    comp->y = this->orig_y;
    comp->orientation = this->orientation;
    int size = static_cast<int>(components.size());
    for (int i = insert_idx + 1; i < size; ++i)
    {
        components[i]->idx_in_row = i;
    }
}

int Row::get_left_limit(int idx_in_row) const
{
    if (idx_in_row > 0 && idx_in_row < static_cast<int>(components.size()) + 1)
    {
        Component *left_neighbor = components[idx_in_row - 1];
        return left_neighbor->site_idx + left_neighbor->width_sites;
    }
    return 0;
}

int Row::get_right_limit(int idx_in_row) const
{
    if (idx_in_row >= -1 && idx_in_row < static_cast<int>(components.size()) - 1)
    {
        Component *right_neighbor = components[idx_in_row + 1];
        return right_neighbor->site_idx;
    }
    return num_sites;
}

long long int Design::get_total_HPWL() const
{
    long long int total_HPWL = 0;
    for (const Net *net : nets)
    {
        total_HPWL += net->get_HPWL();
    }
    return total_HPWL;
}

#include "detailed_placer.hpp"
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <limits>
#include <climits>
#include <chrono>
#include <omp.h>
#include <unordered_set>
#include <numeric>
#include <random>

HungarianSolver::HungarianSolver(int size)
    : n_(size), cost_matrix_(size + 1, std::vector<long long int>(size + 1)), row_label_(size + 1), col_label_(size + 1), match_(size + 1), slack_(size + 1), way_(size + 1)
{
}

void HungarianSolver::set_cost(int i, int j, long long int cost_val)
{
    this->cost_matrix_[i + 1][j + 1] = cost_val;
}

long long int HungarianSolver::solve()
{
    std::fill(match_.begin(), match_.end(), 0);
    std::fill(row_label_.begin(), row_label_.end(), 0);
    std::fill(col_label_.begin(), col_label_.end(), 0);
    std::fill(way_.begin(), way_.end(), 0);

    for (int row = 1; row <= n_; ++row)
    {
        match_[0] = row;
        int current_col = 0;

        std::fill(slack_.begin(), slack_.end(),
                  std::numeric_limits<long long int>::max());
        std::vector<char> in_tree(n_ + 1, false);

        do
        {
            in_tree[current_col] = true;
            int current_row = match_[current_col];

            long long int min_slack = std::numeric_limits<long long int>::max();
            int next_col = 0;

            for (int col = 1; col <= n_; ++col)
            {
                if (!in_tree[col])
                {
                    long long int reduced_cost =
                        cost_matrix_[current_row][col] - row_label_[current_row] - col_label_[col];

                    if (reduced_cost < slack_[col])
                    {
                        slack_[col] = reduced_cost;
                        way_[col] = current_col;
                    }

                    if (slack_[col] < min_slack)
                    {
                        min_slack = slack_[col];
                        next_col = col;
                    }
                }
            }

            for (int col = 0; col <= n_; ++col)
            {
                if (in_tree[col])
                {
                    row_label_[match_[col]] += min_slack;
                    col_label_[col] -= min_slack;
                }
                else
                {
                    slack_[col] -= min_slack;
                }
            }

            current_col = next_col;
        } while (match_[current_col] != 0);

        do
        {
            int parent_col = way_[current_col];
            match_[current_col] = match_[parent_col];
            current_col = parent_col;
        } while (current_col != 0);
    }

    return -col_label_[0];
}

std::vector<int> HungarianSolver::get_assignment()
{
    std::vector<int> result(n_);
    for (int i = 1; i <= n_; ++i)
    {
        if (match_[i] != 0)
        {
            result[match_[i] - 1] = i - 1;
        }
    }
    return result;
}

DetailedPlacer::DetailedPlacer(Design &design, std::chrono::steady_clock::time_point start_time, PlacerConfig config)
    : design_(design), config_(config), start_time_(start_time), rng_()
{
    if (config_.random_seed == 0)
    {
        std::random_device rd;
        config_.random_seed = rd();
    }
    rng_.seed(config_.random_seed);
    std::cout << "[Info] DetailedPlacer initialized with random seed: " << config_.random_seed << std::endl;
}

void DetailedPlacer::run()
{
    using clock = std::chrono::steady_clock;
    auto last_stage_time = clock::now();

    place_components();

    long long int prev_hpwl = design_.get_total_HPWL();
    int iteration = 0;

    std::cout << "[Info] Starting detailed placement" << std::endl;
    std::cout << "[Info] Initial total HPWL: " << prev_hpwl << std::endl;

    const double reserve_for_shift = config_.reserve_for_shift;

    while (true)
    {
        if (is_timeout(reserve_for_shift))
            break;

        local_reorder(4);
        if (is_timeout(reserve_for_shift))
            break;

        independent_set_matching();
        if (is_timeout(reserve_for_shift))
            break;

        global_swap();
        if (is_timeout(reserve_for_shift))
            break;

        local_reorder(3);
        if (is_timeout(reserve_for_shift))
            break;

        long long int cur_hpwl = design_.get_total_HPWL();
        std::cout << "[Info] Iteration " << (++iteration) << " HPWL: " << cur_hpwl
                  << " (Delta: " << prev_hpwl - cur_hpwl << ")"
                  << " Time: " << std::chrono::duration<double>(clock::now() - last_stage_time).count() << "s" << std::endl;

        if (cur_hpwl >= prev_hpwl)
            break;

        prev_hpwl = cur_hpwl;
        last_stage_time = clock::now();
    }

    shift_components();

    std::cout << "[Info] Finished. Total time: "
              << std::chrono::duration<double>(clock::now() - start_time_).count() << " s" << std::endl;
    std::cout << "[Info] Final HPWL: " << design_.get_total_HPWL() << std::endl;
}

void DetailedPlacer::place_components()
{
    std::sort(design_.rows.begin(), design_.rows.end(),
              [](Row *a, Row *b)
              {
                  if (a->orig_y != b->orig_y)
                      return a->orig_y < b->orig_y;
                  return a->orig_x < b->orig_x;
              });

    std::unordered_map<int, std::vector<Row *>> y_to_rows_map;
    std::unordered_map<Row *, int> row_to_idx_map;
    for (size_t i = 0; i < design_.rows.size(); ++i)
    {
        Row *row = design_.rows[i];
        y_to_rows_map[row->orig_y].push_back(row);
        row_to_idx_map[row] = static_cast<int>(i);
    }

    for (Component *comp : design_.components)
    {
        if (comp->type == "PIN")
        {
            continue;
        }
        else if (comp->type == "CORE")
        {
            auto it = y_to_rows_map.find(comp->y);
            if (it == y_to_rows_map.end())
                continue;

            std::vector<Row *> &candidates = it->second;
            Row *target_row = candidates[0];

            for (Row *row : candidates)
            {
                int row_end_x = row->orig_x + (row->num_sites * row->step_x);
                if (comp->x >= row->orig_x && comp->x < row_end_x)
                {
                    target_row = row;
                    break;
                }
            }

            int start_site_idx = std::floor((comp->x - target_row->orig_x) / (double)target_row->step_x);
            comp->site_idx = start_site_idx;
            comp->width_sites = static_cast<int>(std::ceil((double)comp->width / target_row->step_x));
            comp->row_idx = row_to_idx_map[target_row];
            target_row->components.push_back(comp);
        }
        else if (comp->type == "BLOCK")
        {
            comp->is_fixed = true;

            int macro_min_x = comp->x;
            int macro_max_x = comp->x + comp->width;
            int macro_min_y = comp->y;
            int macro_max_y = comp->y + comp->height;

            int n_rows = static_cast<int>(design_.rows.size());
            for (int i = 0; i < n_rows; ++i)
            {
                Row *row = design_.rows[i];

                int row_height_pixels = row->step_y;
                if (design_.site_map.count(row->site_name))
                {
                    row_height_pixels = std::round(
                        design_.site_map[row->site_name]->height *
                        design_.unit.dbu_per_micron);
                }

                int row_min_y = row->orig_y;
                int row_max_y = row_min_y + row_height_pixels;

                if (macro_min_y >= row_max_y || macro_max_y <= row_min_y)
                    continue;

                int row_min_x = row->orig_x;
                int row_max_x = row->orig_x + (row->num_sites * row->step_x);
                if (macro_max_x <= row_min_x || macro_min_x >= row_max_x)
                    continue;

                int start_site_idx = std::floor((double)(macro_min_x - row->orig_x) / row->step_x);
                int end_site_idx = std::ceil((double)(macro_max_x - row->orig_x) / row->step_x);
                start_site_idx = std::max(0, start_site_idx);
                end_site_idx = std::min(row->num_sites, end_site_idx);
                if (start_site_idx >= end_site_idx)
                    continue;

                Component *shadow = new Component(*comp);
                shadow->site_idx = start_site_idx;
                shadow->width_sites = end_site_idx - start_site_idx;
                shadow->row_idx = i;
                shadow->is_fixed = true;
                row->components.push_back(shadow);
            }
        }
    }

    for (Row *row : design_.rows)
    {
        if (row->components.empty())
            continue;
        std::sort(row->components.begin(), row->components.end(),
                  [](Component *a, Component *b)
                  { return a->site_idx < b->site_idx; });
        for (int i = 0; i < static_cast<int>(row->components.size()); ++i)
            row->components[i]->idx_in_row = i;
    }
}

void DetailedPlacer::local_reorder(int window_size)
{
    int num_rows = (int)design_.rows.size();
    if (num_rows <= 0 || window_size <= 1)
        return;

    std::vector<std::vector<int>> row_adjacency_list(num_rows);
    std::vector<int> row_indices;

    for (Net *net : design_.nets)
    {
        row_indices.clear();
        for (Component *comp : net->components)
        {
            if (comp->row_idx >= 0 && comp->row_idx < num_rows)
                row_indices.push_back(comp->row_idx);
        }
        std::sort(row_indices.begin(), row_indices.end());
        row_indices.erase(std::unique(row_indices.begin(), row_indices.end()), row_indices.end());

        for (size_t i = 0; i < row_indices.size(); ++i)
        {
            for (size_t j = i + 1; j < row_indices.size(); ++j)
            {
                row_adjacency_list[row_indices[i]].push_back(row_indices[j]);
                row_adjacency_list[row_indices[j]].push_back(row_indices[i]);
            }
        }
    }

    std::vector<int> color(num_rows, -1);
    std::vector<bool> used_color;
    int max_color = 0;

    for (int r = 0; r < num_rows; ++r)
    {
        std::sort(row_adjacency_list[r].begin(), row_adjacency_list[r].end());
        row_adjacency_list[r].erase(std::unique(row_adjacency_list[r].begin(), row_adjacency_list[r].end()), row_adjacency_list[r].end());

        used_color.assign(max_color + 2, false);
        for (int nei : row_adjacency_list[r])
        {
            if (color[nei] != -1)
                used_color[color[nei]] = true;
        }
        int c = 0;
        while (used_color[c])
            c++;
        color[r] = c;
        max_color = std::max(max_color, c);
    }

    std::vector<std::vector<int>> independent_row_groups(max_color + 1);

    for (int r = 0; r < num_rows; ++r)
        independent_row_groups[color[r]].push_back(r);

    for (const auto &group : independent_row_groups)
    {
        if (is_timeout())
            return;

#pragma omp parallel for schedule(dynamic)
        for (size_t i = 0; i < group.size(); ++i)
        {
            int row_idx = group[i];
            Row *row = design_.rows[row_idx];
            int limit = (int)row->components.size() - window_size;

            for (int j = 0; j <= limit; ++j)
            {
                bool can_reorder = true;
                for (int k = 0; k < window_size; ++k)
                {
                    Component *comp = row->components[j + k];
                    if (!comp || comp->is_fixed || comp->type == "PIN")
                    {
                        can_reorder = false;
                        break;
                    }
                }
                if (!can_reorder)
                    continue;

                std::vector<Component *> window_comps(window_size);
                std::vector<int> order(window_size);
                std::vector<int> orig_x(window_size);
                std::vector<Net *> affected_nets;
                affected_nets.reserve(window_size * 5);

                for (int k = 0; k < window_size; ++k)
                {
                    Component *comp = row->components[j + k];
                    window_comps[k] = comp;
                    order[k] = k;
                    orig_x[k] = comp->x;
                    affected_nets.insert(affected_nets.end(), comp->nets.begin(), comp->nets.end());
                }

                std::sort(affected_nets.begin(), affected_nets.end());
                affected_nets.erase(std::unique(affected_nets.begin(), affected_nets.end()), affected_nets.end());

                long long int min_HPWL = 0;
                for (Net *net : affected_nets)
                    min_HPWL += net->get_HPWL();

                int start_site = window_comps.front()->site_idx;
                std::vector<int> best_order = order;
                bool improved = false;

                do
                {
                    int current_site = start_site;
                    for (int k = 0; k < window_size; ++k)
                    {
                        Component *comp = window_comps[order[k]];
                        comp->x = row->orig_x + current_site * row->step_x;
                        current_site += comp->width_sites;
                    }

                    long long int cur_HPWL = 0;
                    for (Net *net : affected_nets)
                        cur_HPWL += net->get_HPWL();

                    if (cur_HPWL <= min_HPWL)
                    {
                        min_HPWL = cur_HPWL;
                        best_order = order;
                        improved = true;
                    }

                    for (int k = 0; k < window_size; ++k)
                        window_comps[k]->x = orig_x[k];

                } while (std::next_permutation(order.begin(), order.end()));

                if (improved)
                {
                    int current_site = start_site;
                    for (int k = 0; k < window_size; ++k)
                    {
                        Component *comp_to_place = window_comps[best_order[k]];
                        row->components[j + k] = comp_to_place;
                        comp_to_place->idx_in_row = j + k;
                        comp_to_place->site_idx = current_site;
                        comp_to_place->x = row->orig_x + current_site * row->step_x;
                        current_site += comp_to_place->width_sites;
                    }
                }
            }
        }
    }
}

void DetailedPlacer::independent_set_matching()
{
    size_t total_comps = design_.components.size();
    std::vector<int> indices(total_comps);
    std::iota(indices.begin(), indices.end(), 0);
    std::shuffle(indices.begin(), indices.end(), rng_);
    std::vector<Component *> batch_pivots;
    batch_pivots.reserve(config_.batch_size);
    std::vector<MatchingSolution> batch_solutions(config_.batch_size);
    std::unordered_set<Component *> touched_in_batch;

    int timeout_counter = 0;

    for (size_t i = 0; i < total_comps; i += config_.batch_size)
    {
        if ((++timeout_counter & 7) == 0 && is_timeout())
            return;

        batch_pivots.clear();
        for (size_t j = 0; j < config_.batch_size && (i + j) < total_comps; ++j)
        {
            int idx = indices[i + j];
            Component *comp = design_.components[idx];
            if (!comp->is_fixed && comp->type != "PIN")
                batch_pivots.push_back(comp);
        }
        if (batch_pivots.empty())
            continue;
        int cur_batch_size = (int)batch_pivots.size();

#pragma omp parallel for schedule(dynamic)
        for (int j = 0; j < cur_batch_size; ++j)
        {
            Component *pivot = batch_pivots[j];
            MatchingSolution &solution = batch_solutions[j];
            solution.HPWL_reduction = 0;
            solution.comps.clear();
            solution.states.clear();

            std::vector<Component *> candidates;
            candidates.reserve(config_.ism_max_candidates);
            candidates.push_back(pivot);

            OptimalRegion optimal_region = find_optimal_region(pivot);
            std::vector<int> rows_to_check;
            rows_to_check.push_back(pivot->row_idx);
            if (pivot->row_idx + 1 < (int)design_.rows.size())
                rows_to_check.push_back(pivot->row_idx + 1);
            if (pivot->row_idx - 1 >= 0)
                rows_to_check.push_back(pivot->row_idx - 1);
            if (optimal_region.is_valid)
                rows_to_check.push_back((optimal_region.row_min_idx + optimal_region.row_max_idx) / 2);

            std::sort(rows_to_check.begin(), rows_to_check.end());
            rows_to_check.erase(std::unique(rows_to_check.begin(), rows_to_check.end()), rows_to_check.end());

            for (int row_idx : rows_to_check)
            {
                if (candidates.size() >= config_.ism_max_candidates)
                    break;
                if (row_idx < 0 || row_idx >= (int)design_.rows.size())
                    continue;
                Row *row = design_.rows[row_idx];
                int center_x = (std::abs(row_idx - pivot->row_idx) <= 1) ? pivot->x : (optimal_region.x_min + optimal_region.x_max) / 2;
                int range = pivot->width * config_.ism_range_factor;

                for (Component *neighbor : row->components)
                {
                    if (candidates.size() >= config_.ism_max_candidates)
                        break;
                    if (std::abs(neighbor->x - center_x) > range)
                        continue;
                    if (neighbor == pivot || neighbor->is_fixed || neighbor->type == "PIN")
                        continue;
                    if (neighbor->width != pivot->width || neighbor->height != pivot->height)
                        continue;
                    candidates.push_back(neighbor);
                }
            }
            if (candidates.size() < 2)
                continue;

            int num_candidates = (int)candidates.size();
            std::vector<int> degree(num_candidates, 0);
            for (int u = 0; u < num_candidates; ++u)
            {
                for (int v = u + 1; v < num_candidates; ++v)
                {
                    if (!is_independent(candidates[u], candidates[v]))
                    {
                        degree[u]++;
                        degree[v]++;
                    }
                }
            }
            std::vector<int> order(num_candidates);
            std::iota(order.begin(), order.end(), 0);
            std::sort(order.begin() + 1, order.end(), [&](int a, int b)
                      { return degree[a] < degree[b]; });

            std::vector<Component *> independent_set;
            independent_set.reserve(config_.ism_set_size);
            for (int idx : order)
            {
                Component *cand = candidates[idx];
                bool is_compatible = true;
                for (Component *chosen : independent_set)
                {
                    if (!is_independent(cand, chosen))
                    {
                        is_compatible = false;
                        break;
                    }
                }
                if (is_compatible)
                {
                    independent_set.push_back(cand);
                    if (independent_set.size() >= config_.ism_set_size)
                        break;
                }
            }
            if (independent_set.size() < 2)
                continue;

            int set_size = (int)independent_set.size();
            HungarianSolver solver(set_size);
            std::vector<PlacementState> states;
            states.reserve(set_size);
            long long int cur_total = 0;

            for (Component *comp : independent_set)
            {
                Row *ptr = (comp->row_idx >= 0 && comp->row_idx < (int)design_.rows.size()) ? design_.rows[comp->row_idx] : nullptr;
                states.push_back({comp->x, comp->y, comp->row_idx, comp->site_idx, comp->idx_in_row, ptr});
                for (Net *net : comp->nets)
                    cur_total += net->get_HPWL();
            }

            for (int comp_idx = 0; comp_idx < set_size; ++comp_idx)
            {
                for (int slot_idx = 0; slot_idx < set_size; ++slot_idx)
                {
                    long long int cost = 0;
                    Component *comp = independent_set[comp_idx];
                    for (Net *net : comp->nets)
                        cost += net->get_move_HPWL(comp, states[slot_idx].x, states[slot_idx].y);
                    solver.set_cost(comp_idx, slot_idx, cost);
                }
            }

            long long int min_hpwl = solver.solve();
            long long int diff = cur_total - min_hpwl;

            if (diff > 0)
            {
                solution.HPWL_reduction = diff;
                solution.comps = independent_set;
                solution.states.assign(set_size, {0, 0, -1, -1, -1, nullptr});
                std::vector<int> assign = solver.get_assignment();
                for (int k = 0; k < set_size; ++k)
                    solution.states[k] = states[assign[k]];
            }
        }

        touched_in_batch.clear();
        for (int j = 0; j < cur_batch_size; ++j)
        {
            MatchingSolution &res = batch_solutions[j];
            if (res.HPWL_reduction <= 0)
                continue;

            bool conflict = false;
            for (Component *comp : res.comps)
            {
                if (touched_in_batch.count(comp))
                {
                    conflict = true;
                    break;
                }
            }
            if (conflict)
                continue;

            for (size_t k = 0; k < res.comps.size(); ++k)
            {
                Component *comp = res.comps[k];
                PlacementState &state = res.states[k];

                if (!state.row_ptr)
                    continue;

                comp->x = state.x;
                comp->y = state.y;
                comp->row_idx = state.row_idx;
                comp->site_idx = state.site_idx;
                comp->idx_in_row = state.idx_in_row;
                comp->orientation = state.row_ptr->orientation;

                if (state.idx_in_row >= 0 && state.idx_in_row < (int)state.row_ptr->components.size())
                {
                    state.row_ptr->components[state.idx_in_row] = comp;
                }
                touched_in_batch.insert(comp);
            }
        }
    }
}

void DetailedPlacer::global_swap()
{
    size_t total_comps = design_.components.size();
    if (total_comps == 0)
        return;

    std::vector<int> indices(total_comps);
    std::iota(indices.begin(), indices.end(), 0);
    std::shuffle(indices.begin(), indices.end(), rng_);

    int batch_size = config_.batch_size;
    if (batch_size <= 0)
        batch_size = 1;

    std::vector<Component *> batch_candidates;
    batch_candidates.reserve(batch_size);

    std::vector<BestMove> batch_moves;
    batch_moves.reserve(batch_size);

    std::unordered_set<Component *> touched_in_batch;
    touched_in_batch.reserve(batch_size * 2);

    int timeout_counter = 0;

    for (size_t base_idx = 0; base_idx < total_comps; base_idx += batch_size)
    {
        if ((++timeout_counter & 7) == 0 && is_timeout())
            return;

        batch_candidates.clear();

        for (size_t i = 0; i < static_cast<size_t>(batch_size) && (base_idx + i) < total_comps; ++i)
        {
            int idx = indices[base_idx + i];
            Component *comp = design_.components[idx];

            if (!comp->is_fixed && comp->type != "PIN")
                batch_candidates.push_back(comp);
        }

        if (batch_candidates.empty())
            continue;

        int cur_batch_size = static_cast<int>(batch_candidates.size());

        batch_moves.clear();
        batch_moves.resize(cur_batch_size);

#pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < cur_batch_size; ++i)
        {
            Component *comp = batch_candidates[i];
            BestMove local_best_move;
            local_best_move.source_comp = comp;
            OptimalRegion opt_reg = find_optimal_region(comp);
            if (opt_reg.is_valid)
            {
                for (int r = opt_reg.row_min_idx; r <= opt_reg.row_max_idx; ++r)
                {
                    if (design_.rows[r]->site_name != comp->site_name)
                        continue;
                    find_best_move_in_row(comp, r, opt_reg, local_best_move);
                }
            }
            batch_moves[i] = local_best_move;
        }

        std::sort(batch_moves.begin(), batch_moves.end(),
                  [](const BestMove &a, const BestMove &b)
                  {
                      return a.HPWL_reduction > b.HPWL_reduction;
                  });

        touched_in_batch.clear();

        for (int i = 0; i < cur_batch_size; ++i)
        {
            BestMove &best_move = batch_moves[i];

            if (best_move.HPWL_reduction <= 0)
                continue;

            Component *source_component = best_move.source_comp;
            if (!source_component)
                continue;

            if (touched_in_batch.count(source_component))
                continue;

            bool valid = false;
            if (best_move.is_whitespace_move)
            {
                Row *target_row = design_.rows[best_move.target_row_idx];
                if (!is_region_occupied(target_row, best_move.target_site_idx, source_component->width_sites, source_component))
                {
                    valid = true;
                }
            }
            else if (best_move.target_comp)
            {
                if (!touched_in_batch.count(best_move.target_comp))
                {
                    if (is_legal(source_component, best_move.target_comp))
                        valid = true;
                }
            }

            if (!valid)
                continue;

            long long int current_real_reduction = 0;

            if (best_move.is_whitespace_move)
            {
                Row *target_row = design_.rows[best_move.target_row_idx];
                int target_x = target_row->get_x(best_move.target_site_idx);
                int target_y = target_row->orig_y;
                current_real_reduction = calc_move_HPWL_reduction(source_component, target_x, target_y);
            }
            else if (best_move.target_comp)
            {
                current_real_reduction = calc_swap_HPWL_reduction(source_component, best_move.target_comp);
            }

            if (current_real_reduction > 0)
            {
                apply_best_move(source_component, best_move);
                touched_in_batch.insert(source_component);
                if (best_move.target_comp)
                    touched_in_batch.insert(best_move.target_comp);
            }
        }
    }
}

void DetailedPlacer::shift_components()
{
    int timeout_counter = 0;

    for (Component *comp : design_.components)
    {
        if ((timeout_counter & 31) == 0) 
        {
            if (is_timeout())
                break;
        }

        if (comp->is_fixed || comp->type == "PIN")
            continue;

        bool is_move_left = true;
        while (try_shift(comp, is_move_left))
            ;

        is_move_left = false;
        while (try_shift(comp, is_move_left))
            ;

        ++timeout_counter;
    }
}

OptimalRegion DetailedPlacer::find_optimal_region(Component *comp)
{
    OptimalRegion optimal_region;
    if (!comp || comp->nets.empty())
        return optimal_region;

    std::vector<int> x_series;
    std::vector<int> y_series;
    x_series.reserve(comp->nets.size() * 2);
    y_series.reserve(comp->nets.size() * 2);

    for (Net *net : comp->nets)
    {
        Rect rect = net->get_boundary_excluding(comp);
        if (rect.valid)
        {
            x_series.push_back(rect.min_x);
            x_series.push_back(rect.max_x);
            y_series.push_back(rect.min_y);
            y_series.push_back(rect.max_y);
        }
    }

    if (x_series.empty() || y_series.empty())
        return optimal_region;

    size_t mid_idx = x_series.size() / 2;
    std::nth_element(x_series.begin(), x_series.begin() + mid_idx, x_series.end());
    int x_max_val = x_series[mid_idx];
    std::nth_element(x_series.begin(), x_series.begin() + mid_idx - 1, x_series.end());
    int x_min_val = x_series[mid_idx - 1];
    std::nth_element(y_series.begin(), y_series.begin() + mid_idx, y_series.end());
    int target_y_max = y_series[mid_idx];
    std::nth_element(y_series.begin(), y_series.begin() + mid_idx - 1, y_series.end());
    int target_y_min = y_series[mid_idx - 1];

    int padding_x = comp->width * config_.optimal_region_x_padding_factor;
    x_min_val = std::max(0, x_min_val - padding_x);
    x_max_val = x_max_val + padding_x;

    int n_rows = static_cast<int>(design_.rows.size());
    int row_min_idx = 0;
    int row_max_idx = n_rows - 1;

    while (row_min_idx < n_rows && design_.rows[row_min_idx]->orig_y < target_y_min)
        ++row_min_idx;
    while (row_max_idx >= 0 && design_.rows[row_max_idx]->orig_y > target_y_max)
        --row_max_idx;

    row_min_idx = std::max(0, row_min_idx - 1);
    row_max_idx = std::min(n_rows - 1, row_max_idx + 1);

    if (row_min_idx > row_max_idx)
    {
        return OptimalRegion{};
    }

    optimal_region.is_valid = true;
    optimal_region.x_min = x_min_val;
    optimal_region.x_max = x_max_val;
    optimal_region.row_min_idx = row_min_idx;
    optimal_region.row_max_idx = row_max_idx;
    return optimal_region;
}

void DetailedPlacer::find_best_move_in_row(Component *comp, int row_idx, OptimalRegion &region, BestMove &best_move)
{
    Row *row = design_.rows[row_idx];
    if (!row)
        return;

    int start_site_idx = std::floor((region.x_min - row->orig_x) / (double)row->step_x);
    int end_site_idx = std::floor((region.x_max - row->orig_x) / (double)row->step_x);
    start_site_idx = std::max(0, start_site_idx);
    end_site_idx = std::min(row->num_sites - 1, end_site_idx);

    if (start_site_idx > end_site_idx)
        return;

    int cur_scan_site = start_site_idx;
    int scan_limit_idx = end_site_idx + 1;

    for (Component *cur_comp : row->components)
    {
        if (cur_comp->site_idx > end_site_idx)
            break;

        int comp_start_idx = cur_comp->site_idx;
        int comp_end_idx = cur_comp->site_idx + cur_comp->width_sites;

        if (comp_end_idx <= start_site_idx)
            continue;

        int gap_end = std::min(comp_start_idx, scan_limit_idx);
        if (gap_end > cur_scan_site)
        {
            int gap_size = gap_end - cur_scan_site;
            if (gap_size >= comp->width_sites)
            {
                int target_x = row->orig_x + cur_scan_site * row->step_x;
                try_move(comp, row_idx, cur_scan_site, target_x, row->orig_y, best_move);
            }
        }
        cur_scan_site = std::max(cur_scan_site, std::min(comp_end_idx, scan_limit_idx));

        if (!cur_comp->is_fixed && cur_comp != comp && comp_start_idx >= start_site_idx)
        {
            try_swap(comp, cur_comp, best_move);
        }
    }

    if (cur_scan_site < scan_limit_idx)
    {
        int gap_size = scan_limit_idx - cur_scan_site;
        if (gap_size >= comp->width_sites)
        {
            int target_x = row->orig_x + cur_scan_site * row->step_x;
            try_move(comp, row_idx, cur_scan_site, target_x, row->orig_y, best_move);
        }
    }
}

long long int DetailedPlacer::calc_swap_HPWL_reduction(Component *comp_a, Component *comp_b)
{
    if (!comp_a || !comp_b)
        return 0;

    long long int reduction = 0;

    for (Net *net : comp_a->nets)
    {
        reduction += (net->get_HPWL() - net->get_swap_HPWL(comp_a, comp_b));
    }

    for (Net *net : comp_b->nets)
    {
        bool already_calculated = false;
        for (Net *existing : comp_a->nets)
        {
            if (net == existing)
            {
                already_calculated = true;
                break;
            }
        }

        if (!already_calculated)
        {
            reduction += (net->get_HPWL() - net->get_swap_HPWL(comp_a, comp_b));
        }
    }
    return reduction;
}

long long int DetailedPlacer::calc_move_HPWL_reduction(Component *comp, int target_x, int target_y)
{
    if (!comp)
        return 0;
    long long int HPWL_reduction = 0;
    for (Net *net : comp->nets)
    {
        HPWL_reduction += (net->get_HPWL() - net->get_move_HPWL(comp, target_x, target_y));
    }
    return HPWL_reduction;
}

void DetailedPlacer::try_move(Component *comp, int row_idx, int site_idx, int x, int y, BestMove &best_move)
{
    long long int HPWL_reduction = calc_move_HPWL_reduction(comp, x, y);
    if (HPWL_reduction > best_move.HPWL_reduction)
    {
        best_move.HPWL_reduction = HPWL_reduction;
        best_move.target_comp = nullptr;
        best_move.target_row_idx = row_idx;
        best_move.target_site_idx = site_idx;
        best_move.is_whitespace_move = true;
    }
}

void DetailedPlacer::try_swap(Component *comp_a, Component *comp_b, BestMove &best_move)
{
    if (is_legal(comp_a, comp_b))
    {
        long long int swap_reduction = calc_swap_HPWL_reduction(comp_a, comp_b);
        if (swap_reduction > best_move.HPWL_reduction)
        {
            best_move.HPWL_reduction = swap_reduction;
            best_move.target_comp = comp_b;
            best_move.target_row_idx = -1;
            best_move.target_site_idx = -1;
            best_move.is_whitespace_move = false;
        }
    }
}

bool DetailedPlacer::try_shift(Component *comp, bool is_move_left)
{
    if (!comp || comp->is_fixed || comp->type == "PIN")
        return false;

    Row *row = design_.rows[comp->row_idx];
    int target_site_idx = is_move_left ? (comp->site_idx - 1) : (comp->site_idx + 1);
    int limit = is_move_left ? row->get_left_limit(comp->idx_in_row)
                             : row->get_right_limit(comp->idx_in_row);

    bool blocked = is_move_left ? (target_site_idx < limit)
                                : ((target_site_idx + comp->width_sites) > limit);
    if (blocked)
        return false;

    int target_x = row->get_x(target_site_idx);
    if (calc_move_HPWL_reduction(comp, target_x, row->orig_y) > 0)
    {
        comp->site_idx = target_site_idx;
        comp->x = target_x;
        return true;
    }
    return false;
}

void DetailedPlacer::apply_best_move(Component *comp, BestMove &best_move)
{
    if (best_move.HPWL_reduction <= 0)
        return;

    if (best_move.is_whitespace_move)
    {
        move_component(comp, best_move.target_row_idx, best_move.target_site_idx);
    }
    else if (best_move.target_comp)
    {
        comp->swap_position(best_move.target_comp, design_.rows);
    }
}

void DetailedPlacer::move_component(Component *comp, int target_row_idx, int target_site_idx)
{
    if (!comp)
        return;

    Row *src_row = design_.rows[comp->row_idx];
    Row *target_row = design_.rows[target_row_idx];

    src_row->remove_component(comp);
    target_row->insert_component(comp, target_site_idx);
    comp->row_idx = target_row_idx;
}

bool DetailedPlacer::is_timeout(double reserve_sec)
{
    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - start_time_).count();
    double effective_limit = config_.time_limit_sec - reserve_sec;
    if (effective_limit < 0.0)
        effective_limit = 0.0;

    return elapsed > effective_limit;
}

bool DetailedPlacer::is_legal(Component *comp_a, Component *comp_b)
{
    if (!comp_a || !comp_b)
        return false;
    if (comp_a->width_sites == comp_b->width_sites)
        return true;

    Component *wider = comp_a;
    Component *narrower = comp_b;
    if (wider->width_sites < narrower->width_sites)
        std::swap(wider, narrower);

    Row *row = design_.rows[narrower->row_idx];
    int right_limit = row->get_right_limit(narrower->idx_in_row);
    return (narrower->site_idx + wider->width_sites) <= right_limit;
}

bool DetailedPlacer::is_region_occupied(Row *row, int start_site, int width_sites, Component *ignore_self)
{
    if (!row)
        return false;
    int end_site = start_site + width_sites;

    for (Component *comp : row->components)
    {
        if (comp == ignore_self)
            continue;

        int comp_start = comp->site_idx;
        int comp_end = comp->site_idx + comp->width_sites;

        if (std::max(start_site, comp_start) < std::min(end_site, comp_end))
        {
            return true;
        }
    }
    return false;
}

bool DetailedPlacer::is_independent(const Component *a, const Component *b)
{
    if (!a || !b)
        return false;
    for (Net *n1 : a->nets)
    {
        for (Net *n2 : b->nets)
        {
            if (n1 == n2)
                return false;
        }
    }
    return true;
}
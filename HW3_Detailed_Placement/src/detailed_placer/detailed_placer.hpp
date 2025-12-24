#pragma once
#include "../design/design.hpp"
#include <chrono>
#include <vector>
#include <random>

struct PlacerConfig
{
    double time_limit_sec = 250.0;
    double reserve_for_shift = 10.0;
    int batch_size = 512;
    int ism_set_size = 64;
    int ism_max_candidates = 128;

    int optimal_region_x_padding_factor = 5;
    int ism_range_factor = 5;

    // 0 means using a time-based seed
    unsigned int random_seed = 42;
};
struct OptimalRegion
{
    int row_min_idx;
    int row_max_idx;
    int x_min;
    int x_max;
    bool is_valid = false;
};

struct BestMove
{
    Component *source_comp = nullptr;
    Component *target_comp = nullptr;
    int target_row_idx = -1;
    int target_site_idx = -1;
    long long int HPWL_reduction = 0;
    bool is_whitespace_move = false;
};

struct PlacementState
{
    int x;
    int y;
    int row_idx;
    int site_idx;
    int idx_in_row;
    Row *row_ptr;
};

struct MatchingSolution
{
    long long int HPWL_reduction = 0;
    std::vector<Component *> comps;
    std::vector<PlacementState> states;
};

class HungarianSolver
{
    int n_;
    std::vector<std::vector<long long int>> cost_matrix_;
    std::vector<long long int> row_label_;
    std::vector<long long int> col_label_;
    std::vector<int> match_;
    std::vector<long long int> slack_;
    std::vector<int> way_;

public:
    HungarianSolver(int size);
    void set_cost(int i, int j, long long int cost_val);
    long long int solve();
    std::vector<int> get_assignment();
};

class DetailedPlacer
{
public:
    DetailedPlacer(Design &design, std::chrono::steady_clock::time_point start_time, PlacerConfig config = PlacerConfig());
    void run();

private:
    Design &design_;
    PlacerConfig config_;
    std::chrono::steady_clock::time_point start_time_;
    std::mt19937 rng_;

    void place_components();
    void local_reorder(int window_size);
    void independent_set_matching();
    void global_swap();
    void shift_components();

    OptimalRegion find_optimal_region(Component *comp);
    void find_best_move_in_row(Component *comp, int row_idx, OptimalRegion &region, BestMove &best_move);

    long long int calc_swap_HPWL_reduction(Component *comp_a, Component *comp_b);
    long long int calc_move_HPWL_reduction(Component *comp, int target_x, int target_y);

    void try_move(Component *comp, int row_idx, int site_idx, int x, int y, BestMove &best_move);
    void try_swap(Component *comp_a, Component *comp_b, BestMove &best_move);
    bool try_shift(Component *comp, bool is_move_left);
    void apply_best_move(Component *comp, BestMove &best_move);
    void move_component(Component *comp, int target_row_idx, int target_site_idx);

    bool is_timeout(double reserve_sec = 0.0);
    bool is_legal(Component *comp_a, Component *comp_b);
    bool is_region_occupied(Row *row, int start_site, int width_sites, Component *ignore_self);
    bool is_independent(const Component *a, const Component *b);
};

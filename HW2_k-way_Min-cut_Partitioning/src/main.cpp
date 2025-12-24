#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <string>
#include <algorithm>
#include <climits>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <iterator>
using namespace std;

const double TWO_WAY_LOWER_BOUND = 0.45;
const double TWO_WAY_UPPER_BOUND = 0.55;
const double FOUR_WAY_LOWER_BOUND = 0.225;
const double FOUR_WAY_UPPER_BOUND = 0.275;
const double PERTURBATION_RATIO = 0.01;

class Cell;
class Net;

class Net
{
public:
    string name;
    int cell_num;
    int partition_cnt[2];
    bool is_active;
    vector<Cell *> cell_list;
    Net() {}
    Net(string name, int cell_num) : name(name), cell_num(cell_num), is_active(true)
    {
        partition_cnt[0] = 0;
        partition_cnt[1] = 0;
    }
};

class Cell
{
public:
    string name;
    double size;
    bool is_locked;
    int gain;
    int partition;
    vector<Net *> net_list;
    list<Cell *>::iterator it;
    Cell() {}
    Cell(string name, double size) : name(name), size(size), is_locked(false), gain(0), partition(0) {}
};

class Partition
{
public:
    double size;
    int max_index;
    vector<list<Cell *>> bucket_list;
    Partition() : size(0), max_index(-1) {}
};

void init_partition(int &max_p, double &upper, vector<Cell *> &cell_array, vector<Partition> &partition)
{
    max_p = 0;
    for (auto cell : cell_array)
    {
        int side = (partition[0].size <= partition[1].size) ? 0 : 1;
        if (partition[side].size + cell->size > upper)
            side = 1 - side;
        partition[side].size += cell->size;
        cell->partition = side;
        if (cell->net_list.size() > max_p)
            max_p = cell->net_list.size();
    }
}

void init_cutsize(int &cutsize, vector<Net *> &net_array)
{
    cutsize = 0;
    for (auto net : net_array)
    {
        net->partition_cnt[0] = 0;
        net->partition_cnt[1] = 0;
        for (auto cell : net->cell_list)
        {
            if (cell->partition == 0)
                net->partition_cnt[0]++;
            else if (cell->partition == 1)
                net->partition_cnt[1]++;
        }
        if (net->partition_cnt[0] > 0 && net->partition_cnt[1] > 0)
            cutsize++;
    }
}

void init_gain_and_bucket(int &max_p, vector<Cell *> &cell_array, vector<Partition> &partition)
{
    partition[0].bucket_list.clear();
    partition[1].bucket_list.clear();
    partition[0].bucket_list.assign(2 * max_p + 1, {});
    partition[1].bucket_list.assign(2 * max_p + 1, {});
    partition[0].max_index = -1;
    partition[1].max_index = -1;
    for (auto cell : cell_array)
    {
        int from = cell->partition;
        cell->gain = 0;
        cell->is_locked = false;
        for (auto net : cell->net_list)
        {
            if (!net->is_active)
                continue;
            if (net->partition_cnt[from] == 1)
                cell->gain++;
            else if (net->partition_cnt[!from] == 0)
                cell->gain--;
        }
        if (cell->gain + max_p > partition[from].max_index)
            partition[from].max_index = cell->gain + max_p;
        partition[from].bucket_list[cell->gain + max_p].push_front(cell);
        cell->it = partition[from].bucket_list[cell->gain + max_p].begin();
    }
}

void update_partition(int &max_p, Cell *cell, vector<Partition> &partition)
{
    int from = cell->partition;
    int gain_value = cell->gain + max_p;
    cell->partition = !from;
    partition[from].size -= cell->size;
    partition[!from].size += cell->size;
    partition[from].bucket_list[gain_value].erase(cell->it);
    if (partition[from].bucket_list[gain_value].empty() && gain_value == partition[from].max_index)
    {
        for (int i = gain_value - 1; i >= 0; --i)
        {
            if (!partition[from].bucket_list[i].empty())
            {
                partition[from].max_index = i;
                return;
            }
        }
        partition[from].max_index = -1;
    }
}

void update_bucket(int &max_p, Cell *cell, int diff, vector<Partition> &partition)
{
    int from = cell->partition;
    int old_gain_val = cell->gain - diff + max_p;
    int new_gain_val = cell->gain + max_p;
    partition[from].bucket_list[old_gain_val].erase(cell->it);
    partition[from].bucket_list[new_gain_val].push_front(cell);
    cell->it = partition[from].bucket_list[new_gain_val].begin();
    if (new_gain_val > partition[from].max_index)
    {
        partition[from].max_index = new_gain_val;
    }
    else if (partition[from].bucket_list[old_gain_val].empty() && old_gain_val == partition[from].max_index)
    {
        partition[from].max_index = new_gain_val;
    }
}

void update_gain(int &max_p, Cell *base_cell, vector<Partition> &partition)
{
    base_cell->is_locked = true;
    int from = base_cell->partition;
    update_partition(max_p, base_cell, partition);
    for (auto net : base_cell->net_list)
    {
        if (!net->is_active)
            continue;
        if (net->partition_cnt[!from] == 0)
        {
            for (auto cell : net->cell_list)
            {
                if (!cell->is_locked)
                {
                    cell->gain++;
                    update_bucket(max_p, cell, 1, partition);
                }
            }
        }
        else if (net->partition_cnt[!from] == 1)
        {
            for (auto cell : net->cell_list)
            {
                if (cell->partition == !from && !cell->is_locked)
                {
                    cell->gain--;
                    update_bucket(max_p, cell, -1, partition);
                    break;
                }
            }
        }
        net->partition_cnt[from]--;
        net->partition_cnt[!from]++;
        if (net->partition_cnt[from] == 0)
        {
            for (auto cell : net->cell_list)
            {
                if (!cell->is_locked)
                {
                    cell->gain--;
                    update_bucket(max_p, cell, -1, partition);
                }
            }
        }
        else if (net->partition_cnt[from] == 1)
        {
            for (auto cell : net->cell_list)
            {
                if (cell->partition == from && !cell->is_locked)
                {
                    cell->gain++;
                    update_bucket(max_p, cell, 1, partition);
                    break;
                }
            }
        }
    }
}

void restore_best(int &max_p, int &cutsize, vector<Cell *> &order, int &best_iteration, vector<Cell *> &cell_array, vector<Net *> &net_array, vector<Partition> &partition)
{
    for (int i = order.size() - 1; i >= best_iteration; --i)
    {
        Cell *cell = order[i];
        int from = cell->partition;
        cell->partition = !from;
        partition[from].size -= cell->size;
        partition[!from].size += cell->size;
    }
    init_cutsize(cutsize, net_array);
    init_gain_and_bucket(max_p, cell_array, partition);
}

void perturb_partition(int &index, int &num_to_swap, double &lower, double &upper, vector<Cell *> &cell_array, vector<Partition> &partition)
{
    vector<Cell *> candidates0, candidates1;
    int size = cell_array.size();
    for (int i = 0; i < num_to_swap; ++i)
    {
        if (cell_array[index]->partition == 0)
            candidates0.push_back(cell_array[index]);
        else
            candidates1.push_back(cell_array[index]);
        index = (index + 1) % size;
    }
    int cand_idx_0 = 0;
    int cand_idx_1 = 0;
    while (cand_idx_0 < candidates0.size() && cand_idx_1 < candidates1.size())
    {
        Cell *cell_0 = candidates0[cand_idx_0];
        Cell *cell_1 = candidates1[cand_idx_1];
        double new_size_0 = partition[0].size - cell_0->size + cell_1->size;
        double new_size_1 = partition[1].size - cell_1->size + cell_0->size;
        if (new_size_0 > lower && new_size_0 < upper && new_size_1 > lower && new_size_1 < upper)
        {
            swap(cell_0->partition, cell_1->partition);
            partition[0].size = new_size_0;
            partition[1].size = new_size_1;
            cand_idx_0++;
            cand_idx_1++;
        }
        else
        {
            if (new_size_0 < lower || new_size_0 > upper)
                cand_idx_0++;
            if (new_size_1 < lower || new_size_1 > upper)
                cand_idx_1++;
        }
    }
}

int two_way_partitioning(double &lower, double &upper, vector<Cell *> &cell_array, vector<Net *> &net_array)
{
    vector<Partition> partition(2);
    int max_p = 0;
    int times = 0;
    int index = 0;
    int cutsize;
    int global_best_cutsize;
    int last_pass_cutsize;
    int num_to_swap = cell_array.size() * PERTURBATION_RATIO;
    bool perturb_last = false;
    init_partition(max_p, upper, cell_array, partition);
    init_cutsize(cutsize, net_array);
    init_gain_and_bucket(max_p, cell_array, partition);
    global_best_cutsize = cutsize;
    last_pass_cutsize = cutsize;
    vector<int> best_partition_state(cell_array.size());
    for (int i = 0; i < cell_array.size(); ++i)
    {
        best_partition_state[i] = cell_array[i]->partition;
    }
    while (++times <= 50)
    {
        int best_iteration_in_pass = 0;
        int current_cutsize_in_pass = cutsize;
        int best_cutsize_in_pass = cutsize;
        vector<Cell *> order;
        for (int iteration = 1; iteration <= cell_array.size(); ++iteration)
        {
            Cell *update_cell = NULL;
            int bucket_idx0 = partition[0].max_index;
            int bucket_idx1 = partition[1].max_index;
            while (!update_cell)
            {
                if (bucket_idx0 < 0 && bucket_idx1 < 0)
                    break;
                if (bucket_idx0 > bucket_idx1 || (bucket_idx0 == bucket_idx1 && partition[0].size >= partition[1].size))
                {
                    for (auto &cell : partition[0].bucket_list[bucket_idx0])
                    {
                        if ((partition[1].size + cell->size) < upper && (partition[0].size - cell->size) > lower)
                        {
                            update_cell = cell;
                            break;
                        }
                    }
                    bucket_idx0--;
                }
                else
                {
                    for (auto &cell : partition[1].bucket_list[bucket_idx1])
                    {
                        if ((partition[0].size + cell->size) < upper && (partition[1].size - cell->size) > lower)
                        {
                            update_cell = cell;
                            break;
                        }
                    }
                    bucket_idx1--;
                }
            }
            if (!update_cell)
                break;
            current_cutsize_in_pass -= update_cell->gain;
            order.push_back(update_cell);
            update_gain(max_p, update_cell, partition);
            if (current_cutsize_in_pass < best_cutsize_in_pass)
            {
                best_cutsize_in_pass = current_cutsize_in_pass;
                best_iteration_in_pass = iteration;
            }
        }
        if (best_cutsize_in_pass >= last_pass_cutsize && !perturb_last)
        {
            perturb_last = true;
            partition[0].size = 0;
            partition[1].size = 0;
            for (int i = 0; i < cell_array.size(); ++i)
            {
                cell_array[i]->partition = best_partition_state[i];
                partition[cell_array[i]->partition].size += cell_array[i]->size;
            }
            perturb_partition(index, num_to_swap, lower, upper, cell_array, partition);
            init_cutsize(cutsize, net_array);
            init_gain_and_bucket(max_p, cell_array, partition);
        }
        else
        {
            restore_best(max_p, cutsize, order, best_iteration_in_pass, cell_array, net_array, partition);
            if (best_cutsize_in_pass < global_best_cutsize)
            {
                global_best_cutsize = best_cutsize_in_pass;
                for (int i = 0; i < cell_array.size(); ++i)
                {
                    best_partition_state[i] = cell_array[i]->partition;
                }
            }
            perturb_last = false;
        }
        last_pass_cutsize = best_cutsize_in_pass;
    }
    for (int i = 0; i < cell_array.size(); ++i)
    {
        cell_array[i]->partition = best_partition_state[i];
    }
    init_cutsize(cutsize, net_array);
    return cutsize;
}

int cal_four_way_cutsize(vector<Net *> &net_array)
{
    int cutsize = 0;
    for (auto net : net_array)
    {
        unordered_map<int, int> part_count;
        for (auto cell : net->cell_list)
        {
            part_count[cell->partition]++;
        }
        if (part_count.size() > 1)
            cutsize++;
    }
    return cutsize;
}

int four_way_partitioning(double &total_size, vector<Cell *> &cell_array, vector<Net *> &net_array)
{
    double lower = total_size * 0.47;
    double upper = total_size * 0.53;
    two_way_partitioning(lower, upper, cell_array, net_array);
    vector<Cell *> cell_array0;
    vector<Cell *> cell_array1;
    vector<Net *> net_array0;
    vector<Net *> net_array1;
    for (auto cell : cell_array)
    {
        if (cell->partition == 0)
            cell_array0.push_back(cell);
        else
            cell_array1.push_back(cell);
    }
    unordered_set<Cell *> cell_set0(cell_array0.begin(), cell_array0.end());
    unordered_set<Cell *> cell_set1(cell_array1.begin(), cell_array1.end());
    for (auto net : net_array)
    {
        net->is_active = false;
        bool all_in_0 = true;
        bool all_in_1 = true;
        for (auto cell : net->cell_list)
        {
            if (!cell_set0.count(cell))
                all_in_0 = false;
            if (!cell_set1.count(cell))
                all_in_1 = false;
            if (!all_in_0 && !all_in_1)
                break;
        }
        if (all_in_0)
        {
            net_array0.push_back(net);
            net->is_active = true;
        }
        else if (all_in_1)
        {
            net_array1.push_back(net);
            net->is_active = true;
        }
    }
    lower = total_size * FOUR_WAY_LOWER_BOUND;
    upper = total_size * FOUR_WAY_UPPER_BOUND;
    two_way_partitioning(lower, upper, cell_array0, net_array0);
    two_way_partitioning(lower, upper, cell_array1, net_array1);
    for (auto cell : cell_array1)
    {
        cell->partition = (cell->partition == 0 ? 2 : 3);
    }
    return cal_four_way_cutsize(net_array);
}

void write_output(const string &outpath, int cutsize, int k, const vector<Cell *> &cell_array)
{
    ofstream out(outpath);
    if (!out.is_open())
    {
        cerr << "Error opening output file: " << outpath << endl;
        return;
    }
    out << "CutSize " << cutsize << endl;
    vector<vector<string>> partition(k);
    string part_names = "ABCD";
    for (auto cell : cell_array)
    {
        partition[cell->partition].push_back(cell->name);
    }
    for (int i = 0; i < k; ++i)
    {
        out << "Group" << part_names[i] << " " << partition[i].size() << endl;
        for (const auto &name : partition[i])
        {
            out << name << endl;
        }
    }
    out.close();
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        cerr << "Usage: " << argv[0] << " <input file> <output file> <number of partitions>\n";
        return 1;
    }
    string input_path = argv[1];
    string output_path = argv[2];
    int k = atoi(argv[3]);
    ifstream in(input_path);
    if (!in.is_open())
    {
        cerr << "Error opening file: " << input_path << endl;
        return -1;
    }
    string str; // for useless string
    int C = 0;  // total # of cells
    int N = 0;  // total # of nets
    int P = 0;  // total # of pins
    double total_size = 0;
    unordered_map<string, Cell *> hash_table;
    vector<Cell *> cell_array;
    vector<Net *> net_array;
    in >> str >> C;
    for (int i = 0; i < C; ++i)
    {
        double size;
        string cell_name;
        in >> str >> cell_name >> size;
        Cell *cell = new Cell(cell_name, size);
        hash_table[cell_name] = cell;
        cell_array.push_back(cell);
        total_size += size;
    }
    in >> str >> N;
    for (int i = 0; i < N; ++i)
    {
        int cell_num;
        string net_name;
        in >> str >> net_name >> cell_num;
        P += cell_num;
        Net *net = new Net(net_name, cell_num);
        for (int j = 0; j < cell_num; j++)
        {
            string cell_name;
            in >> str >> cell_name;
            hash_table[cell_name]->net_list.push_back(net);
            net->cell_list.push_back(hash_table[cell_name]);
        }
        net_array.push_back(net);
    }
    in.close();
    unsigned int seed = 2442262799;
    std::shuffle(std::begin(cell_array), std::end(cell_array), std::default_random_engine{seed});
    if (k == 2)
    {
        double lower = total_size * TWO_WAY_LOWER_BOUND;
        double upper = total_size * TWO_WAY_UPPER_BOUND;
        int cutsize = two_way_partitioning(lower, upper, cell_array, net_array);
        write_output(output_path, cutsize, 2, cell_array);
    }
    else if (k == 4)
    {
        int cutsize = four_way_partitioning(total_size, cell_array, net_array);
        write_output(output_path, cutsize, 4, cell_array);
    }
    else
    {
        cerr << "Error: not supported." << endl;
        return -1;
    }
    return 0;
}
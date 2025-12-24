#include "global_router.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <queue>
#include <vector>

void GlobalRouter::Route() {
  RunInitialStage();
  RunMainStage();
  RunRefinementStage();

  std::cout << "=== Final Result ===" << std::endl;
  std::cout << "Overflow: " << design_.CalculateTotalOverflow() << std::endl;
  std::cout << "Wirelength: " << design_.CalculateTotalWirelength()
            << std::endl;
}

void GlobalRouter::RunInitialStage() {
  std::cout << "=== Stage 1: Initial Routing  ===" << std::endl;
  stage_ = Stage::Initial;

  // straight nets
  std::vector<Net*> flat_nets;
  // non-straight nets
  std::vector<Net*> standard_nets;

  for (const auto& net_ptr : design_.nets()) {
    Net* net = net_ptr.get();
    if (net->pins().size() < 2) continue;

    const auto& pin1 = net->pins()[0];
    const auto& pin2 = net->pins()[1];

    if (pin1->x() == pin2->x() || pin1->y() == pin2->y()) {
      flat_nets.push_back(net);
    } else {
      standard_nets.push_back(net);
    }
  }

  // Sort normal_nets by bounding box area ascend, hpwl ascend
  std::sort(standard_nets.begin(), standard_nets.end(), [&](Net* a, Net* b) {
    auto compute_area = [](Net* n) {
      long long bbox_width = std::abs(n->pins()[0]->x() - n->pins()[1]->x());
      long long bbox_height = std::abs(n->pins()[0]->y() - n->pins()[1]->y());
      return (bbox_width + 1) * (bbox_height + 1);
    };

    long long area_a = compute_area(a);
    long long area_b = compute_area(b);
    if (area_a != area_b) return area_a < area_b;

    return a->GetHPWL() < b->GetHPWL();
  });

  for (Net* net : flat_nets) {
    std::vector<Point> path;
    if (MonotonicRoute(net, path) && !path.empty()) {
      net->SetPath(path);
      design_.UpdateEdgeUsage(net->path(), 1);
    }
  }

  for (Net* net : standard_nets) {
    std::vector<Point> path;
    if (MonotonicRoute(net, path) && !path.empty()) {
      net->SetPath(path);
      design_.UpdateEdgeUsage(net->path(), 1);
    }
  }

  std::cout << "  Overflow: " << design_.CalculateTotalOverflow()
            << ", Wirelength: " << design_.CalculateTotalWirelength()
            << std::endl;
}

void GlobalRouter::RunMainStage() {
  const double kTimeLimit = 400.0;
  std::cout << "\n=== Stage 2: Rip-up and Reroute ===" << std::endl;

  stage_ = Stage::Main;
  current_iter_ = 0;

  while (true) {
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = now - start_time_;

    if (elapsed.count() > kTimeLimit) {
      break;
    }

    int total_overflow = design_.CalculateTotalOverflow();
    int total_wirelength = design_.CalculateTotalWirelength();
    int unrouted_count = 0;
    for (const auto& net : design_.nets()) {
      if (!net->is_routed()) unrouted_count++;
    }

    std::cout << "  Iter " << std::setw(3) << current_iter_
              << " | Overflow: " << std::setw(6) << total_overflow
              << " | Wirelength: " << total_wirelength
              << " | Unrouted: " << unrouted_count << std::endl;

    if (total_overflow == 0 && unrouted_count == 0) {
      std::cout << "  >>> Stage 2 Converged! Zero Overflow & All Routed. <<<"
                << std::endl;
      break;
    }

    design_.UpdateHistory();

    std::vector<Net*> candidates;
    for (const auto& net_ptr : design_.nets()) {
      Net* net = net_ptr.get();

      if (!net->is_routed()) {
        candidates.push_back(net);
        continue;
      }

      bool passes_overflow = false;
      const auto& path = net->path();
      for (size_t i = 0; i < path.size() - 1; ++i) {
        Point u = path[i];
        Point v = path[i + 1];
        if (u == v) continue;

        EdgeType type =
            (u.y == v.y) ? EdgeType::kHorizontal : EdgeType::kVertical;
        int x = std::min(u.x, v.x);
        int y = std::min(u.y, v.y);

        if (design_.GetEdge(x, y, type).is_overflow()) {
          passes_overflow = true;
          break;
        }
      }

      if (passes_overflow) {
        candidates.push_back(net);
      }
    }

    auto compute_overflow_count = [&](Net* net) -> int {
      if (!net->is_routed()) return 1e9;
      int count = 0;
      const auto& path = net->path();
      for (size_t i = 0; i < path.size() - 1; ++i) {
        Point u = path[i];
        Point v = path[i + 1];
        if (u == v) continue;

        EdgeType type =
            (u.y == v.y) ? EdgeType::kHorizontal : EdgeType::kVertical;
        int x = std::min(u.x, v.x);
        int y = std::min(u.y, v.y);

        if (design_.GetEdge(x, y, type).is_overflow()) {
          count++;
        }
      }
      return count;
    };

    // sort candidates by overflow count descend, area descend, hpwl descend
    std::sort(candidates.begin(), candidates.end(), [&](Net* a, Net* b) {
      int overflow_count_a = compute_overflow_count(a);
      int overflow_count_b = compute_overflow_count(b);
      if (overflow_count_a != overflow_count_b)
        return overflow_count_a > overflow_count_b;  // Descending

      auto compute_area = [](Net* n) {
        long long bbox_width = std::abs(n->pins()[0]->x() - n->pins()[1]->x());
        long long bbox_height = std::abs(n->pins()[0]->y() - n->pins()[1]->y());
        return (bbox_width + 1) * (bbox_height + 1);
      };

      long long area_a = compute_area(a);
      long long area_b = compute_area(b);
      if (area_a != area_b) return area_a > area_b;  // Descending

      return a->GetHPWL() > b->GetHPWL();  // Descending
    });

    for (Net* net : candidates) {
      if (net->is_routed()) {
        design_.UpdateEdgeUsage(net->path(), -1);
      }

      std::vector<Point> new_path;
      bool success = MazeRoute(net, new_path);

      if (success && !new_path.empty()) {
        net->SetPath(new_path);
      }

      if (net->is_routed()) {
        design_.UpdateEdgeUsage(net->path(), 1);
      }
    }

    current_iter_++;
  }
}

void GlobalRouter::RunRefinementStage() {
  const double TIME_LIMIT = 420.0;
  if (design_.CalculateTotalOverflow() != 0) {
    std::cout << "\n[Stage 3 Skipped] Overflow is not zero." << std::endl;
    return;
  }

  std::cout << "\n=== Stage 3: Wirelength Refinement  ===" << std::endl;
  stage_ = Stage::Refinement;
  int iter_count = 0;

  while (true) {
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = now - start_time_;

    if (elapsed.count() > TIME_LIMIT) {
      break;
    }

    std::vector<Candidate> candidates;
    for (const auto& net_ptr : design_.nets()) {
      Net* net = net_ptr.get();
      if (!net->is_routed()) continue;

      int current_len = static_cast<int>(net->path().size()) - 1;
      int hpwl = net->GetHPWL();
      int diff = current_len - hpwl;

      if (diff > 0) {
        candidates.push_back({net, diff});
      }
    }

    if (candidates.empty()) {
      std::cout << "  >>> Stage 3 Converged! No more nets to optimize. <<<"
                << std::endl;
      break;
    }

    // sort candidates by diff descend, area descend , hpwl descsecnd
    std::sort(candidates.begin(), candidates.end(),
              [](const Candidate& a, const Candidate& b) {
                if (a.diff != b.diff) return a.diff > b.diff;

                auto get_area = [](Net* n) {
                  long long bbox_width =
                      std::abs(n->pins()[0]->x() - n->pins()[1]->x());
                  long long bbox_height =
                      std::abs(n->pins()[0]->y() - n->pins()[1]->y());
                  return (bbox_width + 1) * (bbox_height + 1);
                };

                long long area_a = get_area(a.net);
                long long area_b = get_area(b.net);
                if (area_a != area_b) return area_a > area_b;

                return a.net->GetHPWL() > b.net->GetHPWL();
              });

    int improved_count = 0;
    for (auto& cand : candidates) {
      Net* net = cand.net;
      const std::vector<Point>& old_path = net->path();
      size_t old_length = old_path.size();

      design_.UpdateEdgeUsage(old_path, -1);

      std::vector<Point> new_path;
      bool success = MazeRoute(net, new_path);

      if (success && !new_path.empty() && new_path.size() < old_length) {
        net->SetPath(new_path);
        design_.UpdateEdgeUsage(new_path, 1);
        improved_count++;
      } else {
        design_.UpdateEdgeUsage(old_path, 1);
      }
    }

    std::cout << "  Refine Iter " << iter_count << ": Improved "
              << improved_count << " nets. " << std::endl;

    if (improved_count == 0) {
      std::cout
          << "  >>> Stage 3 Converged! No further improvements possible. <<<"
          << std::endl;
      break;
    }

    iter_count++;
  }
}

double GlobalRouter::GetMonotonicCost(int x, int y, EdgeType type) {
  // [Base Cost]
  // Formula: Cost += W * (util ^ K)
  constexpr double kBaseW = 0.6;  // Weight for normal utilization.
  constexpr int kBaseK = 3;       // Exponent (steepness) for normal usage.

  // [Utilization Cost]
  // Formula: If util > T, Cost += W * ((util - T) / (1 - T)) ^ K
  constexpr double kUtilT = 0.82;  // Threshold (0.0~1.0) to trigger penalty.
  constexpr double kUtilW = 12.0;  // Weight of the utilization penalty.
  constexpr int kUtilK = 4;        // Exponent of the utilization penalty.

  // [Overflow Penalty]
  // Formula: If demand > cap, Cost += W * (overflow ^ 2)
  constexpr double kOverW = 35.0;  // Weight for squared overflow penalty.

  constexpr double kMaxCost = 1e15;  // cost upper bound

  const GridEdge& edge = design_.GetEdge(x, y, type);

  // Avoid division by zero.
  double capacity =
      (edge.capacity > 0) ? static_cast<double>(edge.capacity) : 1e-9;
  double new_demand = static_cast<double>(edge.demand) + 1.0;
  // Utilization
  double utilization = new_demand / capacity;

  // 1. Base Cost
  double cost = 1.0 + kBaseW * std::pow(utilization, kBaseK);

  // 2. Utilization Cost
  if (utilization > kUtilT) {
    double excess = (utilization - kUtilT) / (1.0 - kUtilT);
    cost += kUtilW * std::pow(excess, kUtilK);
  }

  // 3. Overflow Penalty
  if (new_demand > capacity) {
    double overflow = new_demand - capacity;
    cost += kOverW * overflow * overflow;
  }

  return std::min(cost, kMaxCost);
}

bool GlobalRouter::MonotonicRoute(Net* net, std::vector<Point>& routed_path) {
  if (net->pins().size() < 2) return false;

  Point start_point(net->pins()[0]->x(), net->pins()[0]->y());
  Point end_point(net->pins()[1]->x(), net->pins()[1]->y());

  if (start_point == end_point) {
    routed_path = {start_point};
    return true;
  }

  int dir_x = (end_point.x >= start_point.x) ? 1 : -1;
  int dir_y = (end_point.y >= start_point.y) ? 1 : -1;

  int min_x = std::min(start_point.x, end_point.x);
  int max_x = std::max(start_point.x, end_point.x);
  int min_y = std::min(start_point.y, end_point.y);
  int max_y = std::max(start_point.y, end_point.y);

  int width = design_.width();
  int height = design_.height();

  static std::vector<std::vector<double>> cost_map;
  static std::vector<std::vector<Point>> parent_map;
  static int cached_width = 0, cached_height = 0;

  if (cached_width != width || cached_height != height) {
    cost_map.assign(
        width, std::vector<double>(height, std::numeric_limits<double>::max()));
    parent_map.assign(width, std::vector<Point>(height, {-1, -1}));
    cached_width = width;
    cached_height = height;
  } else {
    for (int x = min_x; x <= max_x; ++x) {
      for (int y = min_y; y <= max_y; ++y) {
        cost_map[x][y] = std::numeric_limits<double>::max();
        parent_map[x][y] = {-1, -1};
      }
    }
  }

  std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open_set;
  cost_map[start_point.x][start_point.y] = 0;
  open_set.push({start_point.x, start_point.y, 0});

  while (!open_set.empty()) {
    Node current = open_set.top();
    open_set.pop();

    int curr_x = current.x;
    int curr_y = current.y;

    if (curr_x == end_point.x && curr_y == end_point.y) break;

    // if (current.f_score > cost_map[curr_x][curr_y]) continue;

    if (curr_x != end_point.x) {
      int next_x = curr_x + dir_x;

      if (next_x >= min_x && next_x <= max_x) {
        int edge_x = std::min(curr_x, next_x);
        int edge_y = curr_y;

        double step_cost =
            GetMonotonicCost(edge_x, edge_y, EdgeType::kHorizontal);

        if (cost_map[curr_x][curr_y] + step_cost < cost_map[next_x][curr_y]) {
          cost_map[next_x][curr_y] = cost_map[curr_x][curr_y] + step_cost;
          parent_map[next_x][curr_y] = {curr_x, curr_y};
          open_set.push({next_x, curr_y, cost_map[next_x][curr_y]});
        }
      }
    }

    if (curr_y != end_point.y) {
      int next_y = curr_y + dir_y;

      if (next_y >= min_y && next_y <= max_y) {
        int edge_x = curr_x;
        int edge_y = std::min(curr_y, next_y);

        double step_cost =
            GetMonotonicCost(edge_x, edge_y, EdgeType::kVertical);

        if (cost_map[curr_x][curr_y] + step_cost < cost_map[curr_x][next_y]) {
          cost_map[curr_x][next_y] = cost_map[curr_x][curr_y] + step_cost;
          parent_map[curr_x][next_y] = {curr_x, curr_y};
          open_set.push({curr_x, next_y, cost_map[curr_x][next_y]});
        }
      }
    }
  }

  if (parent_map[end_point.x][end_point.y].x == -1) return false;

  routed_path.clear();
  Point curr = end_point;
  while (!(curr == start_point)) {
    routed_path.push_back(curr);
    curr = parent_map[curr.x][curr.y];
  }
  routed_path.push_back(start_point);
  std::reverse(routed_path.begin(), routed_path.end());

  return true;
}

double GlobalRouter::GetMazeCost(int x, int y, EdgeType type) {
  // [Refinement Stage]
  // Cost to strictly block overflows in the final stage.
  constexpr double KPenalty = 1e9;

  // [Under-Capacity Cost]
  // Formula: 1.0 + (util * W)
  constexpr double kUnderW = 0.5;  // Weight for load balancing when not full.

  // [Congestion Penalty Scaling]
  // Formula: 1.0 + (Slope * History * (Ratio ^ K))
  constexpr double kIterS = 0.05;  // Step size for scaling per iteration.
  constexpr double kBaseK = 2.5;   // Starting exponent (steepness).
  constexpr double kMaxK = 5.0;    // Maximum exponent limit.
  constexpr double kSlope = 1.0;   // Base slope.

  constexpr double kMaxCost = 1e15;  // cost upper bound

  const GridEdge& edge = design_.GetEdge(x, y, type);

  // Avoid division by zero.
  double capacity = std::max(1.0, static_cast<double>(edge.capacity));
  double new_demand = static_cast<double>(edge.demand) + 1.0;

  double cost = 0.0;

  // 1. Refinement Stage
  if (stage_ == Stage::Refinement) {
    cost = (new_demand > capacity) ? KPenalty : 1.0;
  }
  // 2. Under-Capacity Cost
  else if (new_demand <= capacity) {
    cost = 1.0 + (new_demand / capacity) * kUnderW;
  }
  // 3. Over-Capacity (Congestion) Penalty
  else {
    double history = std::max(1.0, static_cast<double>(edge.history));
    double ratio = new_demand / capacity;
    double iter_factor = current_iter_ * kIterS;
    double slope = kSlope + iter_factor;
    double k = std::min(kMaxK, kBaseK + iter_factor);
    cost = 1.0 + slope * history * std::pow(ratio, k);
  }
  return std::min(cost, kMaxCost);
}

bool GlobalRouter::MazeRoute(Net* net, std::vector<Point>& route_path) {
  if (net->pins().size() < 2) return false;

  Point start_point(net->pins()[0]->x(), net->pins()[0]->y());
  Point end_point(net->pins()[1]->x(), net->pins()[1]->y());

  if (start_point == end_point) {
    route_path = {start_point};
    return true;
  }

  int width = design_.width();
  int height = design_.height();

  static std::vector<std::vector<double>> g_score;
  static std::vector<std::vector<Point>> parent;
  static int cached_width = 0, cached_height = 0;

  if (cached_width != width || cached_height != height) {
    g_score.assign(
        width, std::vector<double>(height, std::numeric_limits<double>::max()));
    parent.assign(width, std::vector<Point>(height, {-1, -1}));
    cached_width = width;
    cached_height = height;
  } else {
    for (int x = 0; x < width; ++x) {
      for (int y = 0; y < height; ++y) {
        g_score[x][y] = std::numeric_limits<double>::max();
        parent[x][y] = {-1, -1};
      }
    }
  }

  std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open_set;

  g_score[start_point.x][start_point.y] = 0;
  double start_h = std::abs(start_point.x - end_point.x) +
                   std::abs(start_point.y - end_point.y);

  open_set.push({start_point.x, start_point.y, start_h});

  const int kDirX[] = {1, -1, 0, 0};
  const int kDirY[] = {0, 0, 1, -1};

  while (!open_set.empty()) {
    Node current = open_set.top();
    open_set.pop();

    int curr_x = current.x;
    int curr_y = current.y;

    if (curr_x == end_point.x && curr_y == end_point.y) break;

    for (int i = 0; i < 4; ++i) {
      int next_x = curr_x + kDirX[i];
      int next_y = curr_y + kDirY[i];

      if (next_x < 0 || next_x >= width || next_y < 0 || next_y >= height)
        continue;

      EdgeType type =
          (next_x == curr_x) ? EdgeType::kVertical : EdgeType::kHorizontal;
      int edge_x = std::min(curr_x, next_x);
      int edge_y = std::min(curr_y, next_y);

      double edge_cost = GetMazeCost(edge_x, edge_y, type);
      double new_g = g_score[curr_x][curr_y] + edge_cost;

      if (new_g < g_score[next_x][next_y]) {
        g_score[next_x][next_y] = new_g;

        double h_score =
            std::abs(next_x - end_point.x) + std::abs(next_y - end_point.y);
        open_set.push({next_x, next_y, new_g + h_score});

        parent[next_x][next_y] = {curr_x, curr_y};
      }
    }
  }

  if (parent[end_point.x][end_point.y].x == -1) return false;

  route_path.clear();
  Point curr = end_point;
  while (!(curr == start_point)) {
    route_path.push_back(curr);
    curr = parent[curr.x][curr.y];
  }
  route_path.push_back(start_point);
  std::reverse(route_path.begin(), route_path.end());

  return true;
}
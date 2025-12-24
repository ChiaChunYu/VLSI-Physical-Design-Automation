#pragma once

#include <chrono>
#include <vector>

#include "../design/design.hpp"

enum class Stage { Initial, Main, Refinement };

struct Node {
  int x, y;
  double f_score;

  bool operator>(const Node& other) const { return f_score > other.f_score; }
};

struct Candidate {
  Net* net;
  int diff;
};

class GlobalRouter {
 public:
  explicit GlobalRouter(Design& design,
                        std::chrono::steady_clock::time_point start_time)
      : design_(design), start_time_(start_time) {}

  void Route();

 private:
  Design& design_;
  int current_iter_ = 0;
  std::chrono::steady_clock::time_point start_time_;
  Stage stage_ = Stage::Initial;

  void RunInitialStage();
  void RunMainStage();
  void RunRefinementStage();

  double GetMonotonicCost(int x, int y, EdgeType type);
  bool MonotonicRoute(Net* net, std::vector<Point>& path);

  double GetMazeCost(int x, int y, EdgeType type);
  bool MazeRoute(Net* net, std::vector<Point>& path);
};

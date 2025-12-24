#pragma once

#include <memory>
#include <string>
#include <vector>

class Net;
enum class EdgeType { kHorizontal = 0, kVertical = 1 };

struct Point {
  int x = 0;
  int y = 0;

  Point() = default;
  Point(int x, int y) : x(x), y(y) {}

  bool operator==(const Point& other) const {
    return x == other.x && y == other.y;
  }
  bool operator!=(const Point& other) const { return !(*this == other); }
};

struct Segment {
  Point start;
  Point end;

  Segment() = default;
  Segment(Point start, Point end) : start(start), end(end) {}
  Segment(int x1, int y1, int x2, int y2)
      : start(Point(x1, y1)), end(Point(x2, y2)) {}
};

struct GridEdge {
  int capacity = 0;
  int demand = 0;
  int history = 1;

  int overflow() const { return (demand > capacity) ? (demand - capacity) : 0; }
  bool is_overflow() const { return demand > capacity; }
};

class Pin {
 public:
  Pin(std::string name, int x, int y);

  int x() const { return x_; }
  int y() const { return y_; }
  std::string name() const { return name_; }
  Net* net() const { return net_; }

  void SetNet(Net* net);

 private:
  int x_;
  int y_;
  std::string name_;
  Net* net_ = nullptr;
};

class Net {
 public:
  Net(int id, std::string name, int num_pins);

  int id() const { return id_; }
  std::string name() const { return name_; }
  int num_pins() const { return num_pins_; }
  bool is_routed() const { return is_routed_; }

  const std::vector<Pin*>& pins() const { return pins_; }
  const std::vector<Point>& path() const { return path_; }

  void AddPin(Pin* pin);
  void SetPath(const std::vector<Point>& path);
  void ClearPath();
  int GetHPWL() const;

 private:
  int id_;
  std::string name_;
  int num_pins_;
  bool is_routed_ = false;

  std::vector<Pin*> pins_;
  std::vector<Point> path_;
};

class Design {
 public:
  Design() = default;

  int width() const { return grid_width_; }
  int height() const { return grid_height_; }
  int h_capacity() const { return horizontal_capacity_; }
  int v_capacity() const { return vertical_capacity_; }

  const std::vector<std::unique_ptr<Net>>& nets() const { return nets_; }
  const std::vector<std::unique_ptr<Pin>>& pins() const { return pins_; }

  void SetGridDimensions(int width, int height);
  void SetCapacity(int h_capacity, int v_capacity);
  void SetNumNets(int num_nets);

  void AddNet(std::unique_ptr<Net> net);
  void AddPin(std::unique_ptr<Pin> pin);
  void AddDemand(int x, int y, EdgeType type, int val);

  const GridEdge& GetEdge(int x, int y, EdgeType type) const;
  void UpdateHistory();
  void UpdateEdgeUsage(const std::vector<Point>& path, int val);
  int CalculateTotalOverflow() const;
  int CalculateTotalWirelength() const; 

 private:
  int grid_width_ = 0;
  int grid_height_ = 0;
  int horizontal_capacity_ = 0;
  int vertical_capacity_ = 0;
  int num_nets_ = 0;

  std::vector<std::unique_ptr<Net>> nets_;
  std::vector<std::unique_ptr<Pin>> pins_;
  std::vector<std::vector<GridEdge>> h_edges_;
  std::vector<std::vector<GridEdge>> v_edges_;
};
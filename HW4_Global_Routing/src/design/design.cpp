#include "design.hpp"

// --- Pin Implementation ---

Pin::Pin(std::string name, int x, int y)
    : name_(std::move(name)), x_(x), y_(y), net_(nullptr) {}

void Pin::SetNet(Net* net) {
  net_ = net;
}

// --- Net Implementation ---

Net::Net(int id, std::string name, int num_pins)
    : id_(id), name_(std::move(name)), num_pins_(num_pins) {}

void Net::AddPin(Pin* pin) {
  pins_.push_back(pin);
  if (pin) {
    pin->SetNet(this);
  }
}

void Net::SetPath(const std::vector<Point>& path) {
  path_ = path;
  is_routed_ = !path_.empty();
}

void Net::ClearPath() {
  path_.clear();
  is_routed_ = false;
}

int Net::GetHPWL() const {
  if (pins_.empty()) return 0;

  int min_x = pins_[0]->x();
  int max_x = pins_[0]->x();
  int min_y = pins_[0]->y();
  int max_y = pins_[0]->y();

  for (const auto* pin : pins_) {
    if (pin->x() < min_x) min_x = pin->x();
    if (pin->x() > max_x) max_x = pin->x();
    if (pin->y() < min_y) min_y = pin->y();
    if (pin->y() > max_y) max_y = pin->y();
  }

  return (max_x - min_x) + (max_y - min_y);
}

// --- Design Implementation ---

void Design::SetGridDimensions(int width, int height) {
  grid_width_ = width;
  grid_height_ = height;

  if (width > 0 && height > 0) {
    h_edges_.assign(width - 1, std::vector<GridEdge>(height));
    v_edges_.assign(width, std::vector<GridEdge>(height - 1));
  }
}

void Design::SetCapacity(int h_cap, int v_cap) {
  horizontal_capacity_ = h_cap;
  vertical_capacity_ = v_cap;

  for (auto& col : h_edges_) {
    for (auto& edge : col) edge.capacity = h_cap;
  }
  for (auto& col : v_edges_) {
    for (auto& edge : col) edge.capacity = v_cap;
  }
}

void Design::SetNumNets(int num_nets) {
  num_nets_ = num_nets;
  nets_.reserve(num_nets_);
}

void Design::AddNet(std::unique_ptr<Net> net) {
  nets_.push_back(std::move(net));
}

void Design::AddPin(std::unique_ptr<Pin> pin) {
  pins_.push_back(std::move(pin));
}

void Design::AddDemand(int x, int y, EdgeType type, int val) {
  if (type == EdgeType::kHorizontal) {
    if (x >= 0 && x < static_cast<int>(h_edges_.size()) && y >= 0 &&
        y < static_cast<int>(h_edges_[0].size())) {
      auto& edge = h_edges_[x][y];
      edge.demand += val;
      if (edge.demand < 0) edge.demand = 0;
    }
  } else {
    if (x >= 0 && x < static_cast<int>(v_edges_.size()) && y >= 0 &&
        y < static_cast<int>(v_edges_[0].size())) {
      auto& edge = v_edges_[x][y];
      edge.demand += val;
      if (edge.demand < 0) edge.demand = 0;
    }
  }
}

const GridEdge& Design::GetEdge(int x, int y, EdgeType type) const {
  if (type == EdgeType::kHorizontal) {
    return h_edges_.at(x).at(y);
  } else {
    return v_edges_.at(x).at(y);
  }
}

void Design::UpdateHistory() {
  for (auto& col : h_edges_) {
    for (auto& edge : col) {
      if (edge.is_overflow()) {
        edge.history += 1;
      }
    }
  }

  for (auto& col : v_edges_) {
    for (auto& edge : col) {
      if (edge.is_overflow()) {
        edge.history += 1;
      }
    }
  }
}

void Design::UpdateEdgeUsage(const std::vector<Point>& path, int val) {
  if (path.size() < 2) return;

  for (size_t i = 0; i < path.size() - 1; ++i) {
    const Point& u = path[i];
    const Point& v = path[i + 1];

    if (u == v) continue;

    if (u.y == v.y) {
      int x = std::min(u.x, v.x);
      AddDemand(x, u.y, EdgeType::kHorizontal, val);
    } else {
      int y = std::min(u.y, v.y);
      AddDemand(u.x, y, EdgeType::kVertical, val);
    }
  }
}

int Design::CalculateTotalOverflow() const {
  int total_overflow = 0;

  for (const auto& col : h_edges_) {
    for (const auto& edge : col) {
      total_overflow += edge.overflow();
    }
  }

  for (const auto& col : v_edges_) {
    for (const auto& edge : col) {
      total_overflow += edge.overflow();
    }
  }
  return total_overflow;
}

int Design::CalculateTotalWirelength() const {
  int total_wirelength = 0;

  for (const auto& net_ptr : nets_) {
    const auto& path = net_ptr->path();
    if (path.size() < 2) continue;
    total_wirelength += (path.size() - 1);
  }

  return total_wirelength;
}
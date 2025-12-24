#include "writer.hpp"

#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

void Writer::WriteOutput(const std::string& filename, const Design& design) {
  std::ofstream outfile(filename);

  if (!outfile.is_open()) {
    throw std::runtime_error("Error: Could not open file " + filename +
                             " for writing.");
  }

  int total_wirelength = design.CalculateTotalWirelength();
  outfile << "Wirelength " << total_wirelength << "\n";

  for (const auto& net : design.nets()) {
    outfile << "Net " << net->name() << "\n";

    const auto& path = net->path();
    if (path.size() < 2) continue;
    Point start = path[0];
    bool is_vertical = (path[1].x == start.x);

    for (size_t i = 1; i < path.size() - 1; ++i) {
      Point curr = path[i];
      Point next = path[i + 1];

      bool next_is_vertical = (next.x == curr.x);

      if (is_vertical != next_is_vertical) {
        outfile << "Segment " << start.x << " " << start.y << " " << curr.x
                << " " << curr.y << "\n";

        start = curr;
        is_vertical = next_is_vertical;
      }
    }
    outfile << "Segment " << start.x << " " << start.y << " " << path.back().x
            << " " << path.back().y << "\n";
  }
  outfile.close();
}
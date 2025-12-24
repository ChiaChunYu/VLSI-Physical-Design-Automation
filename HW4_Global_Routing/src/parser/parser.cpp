#include "parser.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

void Parser::Parse(const std::string& filename, Design& design) {
  std::ifstream infile(filename);

  if (!infile.is_open()) {
    throw std::runtime_error("Error: Could not open file " + filename +
                             " for parsing.");
  }

  std::string useless;

  int width, height;
  infile >> useless >> width >> height;
  design.SetGridDimensions(width, height);

  int h_capacity, v_capacity;
  infile >> useless >> h_capacity >> v_capacity;
  design.SetCapacity(h_capacity, v_capacity);

  int num_nets;
  infile >> useless >> num_nets;
  design.SetNumNets(num_nets);

  int net_id_counter = 0;

  for (int i = 0; i < num_nets; ++i) {
    std::string net_name;
    int num_pins;
    infile >> useless >> net_name >> num_pins;

    auto new_net = std::make_unique<Net>(net_id_counter++, net_name, num_pins);

    for (int j = 0; j < num_pins; ++j) {
      std::string pin_name;
      int x, y;
      infile >> useless >> pin_name >> x >> y;

      auto new_pin = std::make_unique<Pin>(pin_name, x, y);
      new_net->AddPin(new_pin.get());
      design.AddPin(std::move(new_pin));
    }
    design.AddNet(std::move(new_net));
  }
  infile.close();
}
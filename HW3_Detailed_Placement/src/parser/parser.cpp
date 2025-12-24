#include "parser.hpp"
#include <cmath>
#include <iostream>

void Parser::parse_lef(const std::string &filename, Design &design)
{
    std::ifstream lef_file;
    lef_file.open(filename);
    if (!lef_file)
    {
        std::cerr << "Error opening LEF file: " << filename << std::endl;
        return;
    }
    std::string line;
    std::vector<Macro *> macros;
    Site *current_site = nullptr;
    LefParserState state = LefParserState::IDLE;
    while (getline(lef_file, line))
    {
        if (line.empty())
        {
            continue;
        }
        std::vector<std::string> tokens;
        std::stringstream ss(line);
        while (ss >> line)
        {
            tokens.push_back(line);
        }
        std::string first_token = tokens[0];
        switch (state)
        {
        case LefParserState::IDLE:
            if (first_token == "UNITS")
            {
                state = LefParserState::IN_UNITS;
            }
            else if (first_token == "SITE")
            {
                Site *site = new Site;
                site->name = tokens[1];
                current_site = site;
                design.site_map[site->name] = site;
                state = LefParserState::IN_SITE;
            }
            else if (first_token == "MACRO")
            {
                Macro *macro = new Macro;
                macro->name = tokens[1];
                macros.push_back(macro);
                design.macro_map[macro->name] = macro;
                state = LefParserState::IN_MACRO;
            }
            break;
        case LefParserState::IN_UNITS:
            if (first_token == "DATABASE")
            {
                design.unit.dbu_per_micron = stod(tokens[2]);
            }
            else if (first_token == "END" && tokens[1] == "UNITS")
            {
                state = LefParserState::IDLE;
            }
            break;
        case LefParserState::IN_SITE:
            if (first_token == "CLASS")
            {
                current_site->type = tokens[1];
            }
            else if (first_token == "SIZE")
            {
                current_site->width = stod(tokens[1]);
                current_site->height = stod(tokens[3]);
            }
            else if (first_token == "END" && tokens[1] == current_site->name)
            {
                state = LefParserState::IDLE;
            }
            break;
        case LefParserState::IN_MACRO:
            Macro *macro = macros.back();
            if (first_token == "CLASS")
            {
                macro->type = tokens[1];
            }
            else if (first_token == "SIZE")
            {
                macro->width = stod(tokens[1]);
                macro->height = stod(tokens[3]);
            }
            else if (first_token == "SITE")
            {
                macro->site_name = tokens[1];
            }
            else if (first_token == "END" && tokens[1] == macros.back()->name)
            {
                state = LefParserState::IDLE;
            }
            break;
        }
    }
    lef_file.close();
}
void Parser::parse_def(const std::string &filename, Design &design)
{
    std::ifstream def_file;
    def_file.open(filename);
    if (!def_file)
    {
        std::cerr << "Error opening DEF file: " << filename << std::endl;
        return;
    }

    std::string line;
    DefParserState state = DefParserState::IDLE;
    while (getline(def_file, line))
    {
        design.def_file.push_back(line);
        if (line.empty())
        {
            continue;
        }
        std::vector<std::string> tokens;
        std::stringstream ss(line);
        while (ss >> line)
        {
            tokens.push_back(line);
        }
        std::string first_token = tokens[0];
        switch (state)
        {
        case DefParserState::IDLE:
            if (first_token == "DESIGN")
            {
                state = DefParserState::IN_DESIGN;
            }
            break;
        case DefParserState::IN_DESIGN:
            if (first_token == "ROW")
            {
                Row *row = new Row;
                row->name = tokens[1];
                row->site_name = tokens[2];
                row->orig_x = stoi(tokens[3]);
                row->orig_y = stoi(tokens[4]);
                row->orientation = tokens[5];
                row->num_sites = stoi(tokens[7]);
                row->step_x = stoi(tokens[11]);
                row->step_y = stoi(tokens[12]);
                design.rows.push_back(row);
            }
            else if (first_token == "COMPONENTS")
            {
                state = DefParserState::IN_COMPONENTS;
            }
            else if (first_token == "PINS")
            {
                state = DefParserState::IN_PINS;
            }
            else if (first_token == "NETS")
            {
                state = DefParserState::IN_NETS;
            }
            else if (first_token == "END" && tokens[1] == "DESIGN")
            {
                state = DefParserState::IDLE;
            }
            break;
        case DefParserState::IN_COMPONENTS:
            if (first_token == "END" && tokens[1] == "COMPONENTS")
            {
                state = DefParserState::IN_DESIGN;
                break;
            }
            for (int i = 0; i < tokens.size(); ++i)
            {
                if (tokens[i] == "-")
                {
                    Component *comp = new Component;
                    comp->name = tokens[i + 1];
                    comp->macro_name = tokens[i + 2];
                    Macro *macro = design.macro_map[comp->macro_name];
                    comp->site_name = macro->site_name;
                    comp->width = round(macro->width * design.unit.dbu_per_micron);
                    comp->height = round(macro->height * design.unit.dbu_per_micron);
                    comp->type = macro->type;
                    design.components.push_back(comp);
                    design.component_map[comp->name] = comp;
                }
                else if (tokens[i] == "PLACED")
                {
                    Component *comp = design.components.back();
                    comp->is_fixed = false;
                    comp->x = stoi(tokens[i + 2]);
                    comp->y = stoi(tokens[i + 3]);
                    comp->orientation = tokens[i + 5];
                }
                else if (tokens[i] == "FIXED")
                {
                    Component *comp = design.components.back();
                    comp->is_fixed = true;
                    comp->x = stoi(tokens[i + 2]);
                    comp->y = stoi(tokens[i + 3]);
                    comp->orientation = tokens[i + 5];
                }
            }
            break;
        case DefParserState::IN_PINS:
            if (first_token == "END" && tokens[1] == "PINS")
            {
                state = DefParserState::IN_DESIGN;
                break;
            }
            for (int i = 0; i < tokens.size(); ++i)
            {
                if (tokens[i] == "-")
                {
                    Component *comp = new Component;
                    comp->name = tokens[i + 1];
                    comp->type = "PIN";
                    comp->is_fixed = true;
                    design.component_map[comp->name] = comp;
                    design.components.push_back(comp);
                }
                else if (tokens[i] == "PLACED")
                {
                    Component *comp = design.components.back();
                    comp->x = stoi(tokens[i + 2]);
                    comp->y = stoi(tokens[i + 3]);
                    comp->orientation = tokens[i + 5];
                }
            }
            break;
        case DefParserState::IN_NETS:
            if (first_token == "END" && tokens[1] == "NETS")
            {
                state = DefParserState::IN_DESIGN;
                break;
            }
            for (int i = 0; i < tokens.size(); ++i)
            {
                if (tokens[i] == "-")
                {
                    Net *net = new Net;
                    net->name = tokens[i + 1];
                    design.nets.push_back(net);
                }
                else if (tokens[i] == "(")
                {
                    Net *current_net = design.nets.back();
                    Component *comp;
                    if (tokens[i + 1] == "PIN")
                    {
                        comp = design.component_map[tokens[i + 2]];
                    }
                    else
                    {
                        comp = design.component_map[tokens[i + 1]];
                    }
                    current_net->components.push_back(comp);
                    comp->nets.push_back(current_net);
                }
            }
            break;
        }
    }
    def_file.close();
}

void Parser::parse(const std::string &lef_filename, const std::string &def_filename, Design &design)
{
    parse_lef(lef_filename, design);
    parse_def(def_filename, design);
}

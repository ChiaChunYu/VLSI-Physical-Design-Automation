#include "result_writer.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

void ResultWriter::write_def(const std::string &filename, const Design &design)
{
    std::ofstream def_file(filename);
    if (!def_file)
    {
        std::cerr << "Error opening DEF file for writing: " << filename << std::endl;
        return;
    }
    bool in_components_section = false;
    const Component *current_comp = nullptr;
    for (const std::string &raw_line : design.def_file)
    {
        std::string line = raw_line;
        if (line.empty())
        {
            def_file << line << '\n';
            continue;
        }
        size_t first_char_pos = line.find_first_not_of(" \t");
        std::string indent = (first_char_pos == std::string::npos) ? "" : line.substr(0, first_char_pos);
        std::string content = (first_char_pos == std::string::npos) ? "" : line.substr(first_char_pos);
        std::vector<std::string> tokens;
        {
            std::stringstream ss(content);
            std::string tok;
            while (ss >> tok)
            {
                tokens.push_back(tok);
            }
        }
        if (tokens.empty())
        {
            def_file << raw_line << '\n';
            continue;
        }
        if (!in_components_section)
        {
            if (tokens[0] == "COMPONENTS")
                in_components_section = true;
            def_file << raw_line << '\n';
            continue;
        }
        else
        {
            if (tokens[0] == "END" && tokens.size() > 1 && tokens[1] == "COMPONENTS")
            {
                in_components_section = false;
                current_comp = nullptr;
                def_file << raw_line << '\n';
                continue;
            }
        }
        if (in_components_section)
        {
            std::vector<std::string> new_tokens = tokens;
            for (int i = 0; i < new_tokens.size(); ++i)
            {
                if (tokens[i] == "-")
                {
                    std::string comp_name = tokens[i + 1];
                    auto it = design.component_map.find(comp_name);
                    if (it != design.component_map.end())
                    {
                        current_comp = it->second;
                    }
                }
                if (tokens[i] == "PLACED" || tokens[i] == "FIXED")
                {
                    new_tokens[i + 2] = std::to_string(current_comp->x);
                    new_tokens[i + 3] = std::to_string(current_comp->y);
                    new_tokens[i + 5] = current_comp->orientation;
                }
            }
            std::string new_content;
            for (size_t t = 0; t < new_tokens.size(); ++t)
            {
                if (t > 0)
                    new_content += ' ';
                new_content += new_tokens[t];
            }
            line = indent + new_content;
            def_file << line << '\n';
        }
        else
        {
            def_file << raw_line << '\n';
        }
    }
    def_file.close();
}

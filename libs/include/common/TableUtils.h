#pragma once

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

inline void printTable(const std::vector<std::string>& columnNames,
                       const std::vector<std::vector<double>>& columns) {
    assert(!columns.empty());
    assert(columns.size() == columnNames.size());
    assert(std::all_of(columns.begin(), columns.end(),
                       [&](const auto& col) { return col.size() == columns[0].size(); }));

    std::string header = "  ";
    for (std::size_t i = 0; i < columnNames.size(); ++i) {
        if (i > 0) {
            header += "  |  ";
        }
        header += columnNames[i];
    }
    header += "  ";
    const std::string separator(header.size(), '-');

    std::cout << separator << "\n" << header << "\n" << separator << "\n";

    for (std::size_t i = 0; i < columns[0].size(); ++i) {
        for (std::size_t j = 0; j < columns.size(); ++j) {
            if (j > 0) {
                std::cout << " | ";
            }
            std::cout << columns[j][i];
        }
        std::cout << "\n";
    }
}

inline void writeCsv(const std::string& filename, const std::vector<std::string>& columnNames,
                     const std::vector<std::vector<double>>& columns) {
    assert(!columns.empty());
    assert(columns.size() == columnNames.size());
    assert(std::all_of(columns.begin(), columns.end(),
                       [&](const auto& col) { return col.size() == columns[0].size(); }));

    std::ofstream file(filename);

    for (std::size_t i = 0; i < columnNames.size(); ++i) {
        if (i > 0) {
            file << ",";
        }
        file << columnNames[i];
    }
    file << "\n";

    for (std::size_t i = 0; i < columns[0].size(); ++i) {
        for (std::size_t j = 0; j < columns.size(); ++j) {
            if (j > 0) {
                file << ",";
            }
            file << columns[j][i];
        }
        file << "\n";
    }
}
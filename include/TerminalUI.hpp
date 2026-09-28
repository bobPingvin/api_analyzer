#pragma once
#include "Models.hpp"
#include <vector>

class TerminalUI {
public:
    static void printHeader();
    static void printAnalysis(const std::vector<Controller>& controllers);
    static void runInteractive(std::vector<Controller>& controllers, const std::vector<DTOModel>& dtos);
};
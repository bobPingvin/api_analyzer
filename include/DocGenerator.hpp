#pragma once
#include "Models.hpp"
#include <string>
#include <vector>

class DocGenerator {
public:
    static bool generateMarkdown(const std::string& filePath, const std::vector<Controller>& controllers, const std::vector<DTOModel>& dtos);
    static bool generateHTML(const std::string& filePath, const std::vector<Controller>& controllers, const std::vector<DTOModel>& dtos);
    static bool generateJSON(const std::string& filePath, const std::vector<Controller>& controllers, const std::vector<DTOModel>& dtos);
    static bool generateReport(const std::string& filePath, const std::vector<Controller>& controllers);
};
#pragma once
#include "Models.hpp"
#include <filesystem>
#include <vector>
#include <mutex>

class ApiParser {
public:
    // Параллельный обход и разбор
    std::vector<Controller> parseDirectory(const std::filesystem::path& dirPath);
    std::vector<DTOModel> parseDTOs(const std::filesystem::path& dirPath);

private:
    void parseFile(const std::filesystem::path& filePath, 
                   std::vector<Controller>& controllers, 
                   std::vector<DTOModel>& dtos, 
                   std::mutex& mtx);
};
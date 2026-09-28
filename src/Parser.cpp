#include "Parser.hpp"
#include "Logger.hpp"
#include <fstream>
#include <sstream>
#include <regex>
#include <future>
#include <algorithm>

static std::string trim(const std::string& s) {
    auto start = s.begin();
    while (start != s.end() && std::isspace(*start)) start++;
    auto end = s.end();
    do { end--; } while (std::distance(start, end) > 0 && std::isspace(*end));
    return std::string(start, end + 1);
}

void ApiParser::parseFile(const std::filesystem::path& filePath,
                          std::vector<Controller>& controllers,
                          std::vector<DTOModel>& dtos,
                          std::mutex& mtx) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        Logger::instance().error("Не удалось открыть файл: " + filePath.string());
        return;
    }

    std::string line;
    Controller currentController;
    bool hasController = false;

    EndpointMethod currentMethod;
    bool pendingMethod = false;

    // Чтение для поиска DTO структур
    std::regex structRegex(R"(struct\s+(\w+)\s*\{)");
    std::regex fieldRegex(R"(^\s*([\w:<>]+)\s+(\w+);)");

    bool insideStruct = false;
    DTOModel currentDTO;

    std::string methodBodyAccumulator;
    int braceBalance = 0;
    bool capturingBody = false;

    while (std::getline(file, line)) {
        std::string tLine = trim(line);

        // Поиск структур данных (DTO)
        std::smatch match;
        if (!insideStruct && std::regex_search(tLine, match, structRegex)) {
            insideStruct = true;
            currentDTO = DTOModel();
            currentDTO.name = match[1].str();
        } else if (insideStruct) {
            if (tLine.find('}') != std::string::npos) {
                insideStruct = false;
                std::lock_guard<std::mutex> lock(mtx);
                dtos.push_back(currentDTO);
            } else if (std::regex_search(tLine, match, fieldRegex)) {
                currentDTO.fields.push_back({match[2].str(), match[1].str()});
            }
        }

        // Парсер аннотаций
        if (tLine.rfind("//", 0) == 0) {
            std::string comment = trim(tLine.substr(2));

            if (comment.rfind("@Controller", 0) == 0) {
                if (hasController) {
                    std::lock_guard<std::mutex> lock(mtx);
                    controllers.push_back(currentController);
                    currentController = Controller();
                }
                currentController.name = trim(comment.substr(11));
                currentController.filePath = filePath.string();
                hasController = true;
            } else if (comment.rfind("@Route", 0) == 0) {
                currentController.basePath = trim(comment.substr(6));
            } else if (comment.rfind("@GET", 0) == 0 || comment.rfind("@POST", 0) == 0 ||
                       comment.rfind("@PUT", 0) == 0 || comment.rfind("@DELETE", 0) == 0) {
                
                if (pendingMethod) {
                    currentController.methods.push_back(currentMethod);
                }
                currentMethod = EndpointMethod();
                pendingMethod = true;

                std::istringstream iss(comment);
                iss >> currentMethod.httpMethod >> currentMethod.path;
                currentMethod.httpMethod = currentMethod.httpMethod.substr(1); // убираем @
            } else if (comment.rfind("@Param", 0) == 0) {
                std::istringstream iss(comment.substr(6));
                Parameter p;
                iss >> p.name >> p.type;
                currentMethod.params.push_back(p);
            } else if (comment.rfind("@Body", 0) == 0) {
                currentMethod.requestBodyType = trim(comment.substr(5));
            } else if (comment.rfind("@Return", 0) == 0) {
                currentMethod.returnType = trim(comment.substr(7));
            } else if (comment.rfind("@Async", 0) == 0) {
                currentMethod.isAsync = true;
            } else if (comment.rfind("@Middleware", 0) == 0) {
                currentMethod.middlewares.push_back(trim(comment.substr(11)));
            } else if (comment.rfind("@Filter", 0) == 0) {
                currentMethod.filters.push_back(trim(comment.substr(7)));
            }
        } else if (pendingMethod && !capturingBody && tLine.find('(') != std::string::npos && tLine.find(';') == std::string::npos) {
            // Нашли заголовок функции
            std::regex funcNameRegex(R"((\w+)\s*\()");
            if (std::regex_search(tLine, match, funcNameRegex)) {
                currentMethod.functionName = match[1].str();
            }
            capturingBody = true;
            braceBalance = 0;
            methodBodyAccumulator.clear();
        }

        if (capturingBody) {
            methodBodyAccumulator += line + "\n";
            for (char c : line) {
                if (c == '{') braceBalance++;
                if (c == '}') braceBalance--;
            }
            if (braceBalance == 0 && line.find('}') != std::string::npos) {
                capturingBody = false;
                pendingMethod = false;

                // Быстрый анализ тела метода
                if (methodBodyAccumulator.find("catch") != std::string::npos) {
                    currentMethod.hasTryCatch = true;
                }
                if (methodBodyAccumulator.find("if") != std::string::npos && 
                   (methodBodyAccumulator.find("throw") != std::string::npos || 
                    methodBodyAccumulator.find("return") != std::string::npos ||
                    methodBodyAccumulator.find("validate") != std::string::npos ||
                    methodBodyAccumulator.find("empty()") != std::string::npos)) {
                    currentMethod.hasValidation = true;
                }

                currentController.methods.push_back(currentMethod);
                currentMethod = EndpointMethod();
            }
        }
    }

    if (hasController) {
        std::lock_guard<std::mutex> lock(mtx);
        controllers.push_back(currentController);
    }
}

std::vector<Controller> ApiParser::parseDirectory(const std::filesystem::path& dirPath) {
    std::vector<Controller> controllers;
    std::vector<DTOModel> dtos;
    std::mutex mtx;
    std::vector<std::future<void>> futures;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(dirPath)) {
        if (entry.is_regular_file()) {
            auto ext = entry.path().extension().string();
            if (ext == ".cpp" || ext == ".hpp" || ext == ".h" || ext == ".cc") {
                futures.push_back(std::async(std::launch::async, &ApiParser::parseFile, this, entry.path(), 
                                             std::ref(controllers), std::ref(dtos), std::ref(mtx)));
            }
        }
    }

    for (auto& f : futures) {
        f.get();
    }

    Logger::instance().info("Успешно проанализировано файлов. Найдено контроллеров: " + std::to_string(controllers.size()));
    return controllers;
}

std::vector<DTOModel> ApiParser::parseDTOs(const std::filesystem::path& dirPath) {
    std::vector<Controller> controllers;
    std::vector<DTOModel> dtos;
    std::mutex mtx;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(dirPath)) {
        if (entry.is_regular_file()) {
            auto ext = entry.path().extension().string();
            if (ext == ".hpp" || ext == ".h" || ext == ".cpp") {
                parseFile(entry.path(), controllers, dtos, mtx);
            }
        }
    }
    return dtos;
}
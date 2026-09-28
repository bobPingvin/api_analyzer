#include "DocGenerator.hpp"
#include "Logger.hpp"
#include <fstream>
#include <sstream>

bool DocGenerator::generateMarkdown(const std::string& filePath, const std::vector<Controller>& controllers, const std::vector<DTOModel>& dtos) {
    std::ofstream out(filePath);
    if (!out.is_open()) return false;

    out << "# Спецификация RESTful API\n\n";
    for (const auto& ctrl : controllers) {
        out << "## Контроллер: `" << ctrl.name << "`\n";
        out << "**Базовый путь:** `" << ctrl.basePath << "`\n\n";

        for (const auto& m : ctrl.methods) {
            out << "### `" << m.httpMethod << " " << m.path << "`\n";
            out << "- **Функция:** `" << m.functionName << "`\n";
            if (m.isAsync) out << "- **Асинхронный:** Да\n";
            if (!m.requestBodyType.empty()) out << "- **Body:** `" << m.requestBodyType << "`\n";
            out << "- **Возвращаемый тип:** `" << m.returnType << "`\n";

            if (!m.params.empty()) {
                out << "\n| Параметр | Тип |\n|---|---|\n";
                for (const auto& p : m.params) {
                    out << "| " << p.name << " | " << p.type << " |\n";
                }
            }
            out << "\n---\n";
        }
    }

    if (!dtos.empty()) {
        out << "## Модели данных (DTO)\n\n";
        for (const auto& dto : dtos) {
            out << "### " << dto.name << "\n";
            out << "| Поле | Тип |\n|---|---|\n";
            for (const auto& f : dto.fields) {
                out << "| " << f.name << " | " << f.type << " |\n";
            }
            out << "\n";
        }
    }
    return true;
}

bool DocGenerator::generateHTML(const std::string& filePath, const std::vector<Controller>& controllers, const std::vector<DTOModel>& dtos) {
    std::ofstream out(filePath);
    if (!out.is_open()) return false;

    out << "<!DOCTYPE html><html lang='ru'><head><meta charset='UTF-8'><title>API Documentation</title>";
    out << "<style>body{font-family:sans-serif;margin:40px;background:#f9f9f9;color:#333;}"
        << "h1,h2{color:#2c3e50;} table{border-collapse:collapse;width:100%;margin:10px 0;}"
        << "th,td{border:1px solid #ddd;padding:8px;text-align:left;} th{background:#eee;}"
        << ".badge{padding:4px 8px;border-radius:4px;color:#fff;font-weight:bold;}"
        << ".badge-GET{background:#61affe;}.badge-POST{background:#49cc90;}.badge-PUT{background:#fca130;}.badge-DELETE{background:#f93e3e;}"
        << ".card{background:#fff;padding:15px;margin-bottom:20px;border-radius:6px;box-shadow:0 1px 3px rgba(0,0,0,0.1);}</style></head><body>";
    out << "<h1>Спецификация RESTful API</h1>";

    for (const auto& ctrl : controllers) {
        out << "<h2>Контроллер: " << ctrl.name << " (" << ctrl.basePath << ")</h2>";
        for (const auto& m : ctrl.methods) {
            out << "<div class='card'>";
            out << "<h3><span class='badge badge-" << m.httpMethod << "'>" << m.httpMethod << "</span> " << m.path << "</h3>";
            out << "<p><strong>Возвращает:</strong> <code>" << m.returnType << "</code></p>";
            if (!m.params.empty()) {
                out << "<table><tr><th>Параметр</th><th>Тип</th></tr>";
                for (const auto& p : m.params) {
                    out << "<tr><td>" << p.name << "</td><td>" << p.type << "</td></tr>";
                }
                out << "</table>";
            }
            out << "</div>";
        }
    }

    if (!dtos.empty()) {
        out << "<h2>Модели Данных</h2>";
        for (const auto& dto : dtos) {
            out << "<div class='card'><h3>" << dto.name << "</h3><table><tr><th>Поле</th><th>Тип</th></tr>";
            for (const auto& f : dto.fields) {
                out << "<tr><td>" << f.name << "</td><td>" << f.type << "</td></tr>";
            }
            out << "</table></div>";
        }
    }
    out << "</body></html>";
    return true;
}

bool DocGenerator::generateJSON(const std::string& filePath, const std::vector<Controller>& controllers, const std::vector<DTOModel>& dtos) {
    std::ofstream out(filePath);
    if (!out.is_open()) return false;

    out << "{\n  \"controllers\": [\n";
    for (size_t i = 0; i < controllers.size(); ++i) {
        const auto& c = controllers[i];
        out << "    {\n      \"name\": \"" << c.name << "\",\n      \"route\": \"" << c.basePath << "\",\n      \"methods\": [\n";
        for (size_t j = 0; j < c.methods.size(); ++j) {
            const auto& m = c.methods[j];
            out << "        {\n";
            out << "          \"httpMethod\": \"" << m.httpMethod << "\",\n";
            out << "          \"path\": \"" << m.path << "\",\n";
            out << "          \"returnType\": \"" << m.returnType << "\"\n";
            out << "        }" << (j + 1 < c.methods.size() ? "," : "") << "\n";
        }
        out << "      ]\n    }" << (i + 1 < controllers.size() ? "," : "") << "\n";
    }
    out << "  ],\n  \"dtos\": [\n";
    for (size_t i = 0; i < dtos.size(); ++i) {
        const auto& d = dtos[i];
        out << "    {\n      \"name\": \"" << d.name << "\",\n      \"fields\": [\n";
        for (size_t j = 0; j < d.fields.size(); ++j) {
            out << "        {\"name\": \"" << d.fields[j].name << "\", \"type\": \"" << d.fields[j].type << "\"}"
                << (j + 1 < d.fields.size() ? "," : "") << "\n";
        }
        out << "      ]\n    }" << (i + 1 < dtos.size() ? "," : "") << "\n";
    }
    out << "  ]\n}\n";
    return true;
}

bool DocGenerator::generateReport(const std::string& filePath, const std::vector<Controller>& controllers) {
    std::ofstream out(filePath);
    if (!out.is_open()) return false;

    out << "=== ОТЧЕТ ОБ АРХИТЕКТУРНОМ АНАЛИЗЕ API ===\n\n";
    int counter = 1;
    for (const auto& ctrl : controllers) {
        for (const auto& m : ctrl.methods) {
            for (const auto& rec : m.recommendations) {
                out << counter++ << ". [" << ctrl.name << "] " << m.httpMethod << " " << m.path << " -> " << rec << "\n";
            }
        }
    }
    return true;
}
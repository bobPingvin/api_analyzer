#include "Parser.hpp"
#include "Analyzer.hpp"
#include "DocGenerator.hpp"
#include "TerminalUI.hpp"
#include "Logger.hpp"
#include <iostream>
#include <filesystem>

void printHelp() {
    std::cout << "Использование: ./api-analyzer [ОПЦИИ]\n"
              << "Параметры:\n"
              << "  --source <path>       Путь к проекту API\n"
              << "  --format <md|html|json> Формат вывода документации\n"
              << "  --output <file>       Файл для сохранения документации\n"
              << "  --report <file>       Файл для сохранения отчета рекомендаций\n"
              << "  --interactive         Интерактивный режим терминала\n"
              << "  --help                Вызов справки\n";
}

int main(int argc, char* argv[]) {
    std::string sourcePath = ".";
    std::string format = "markdown";
    std::string outputFile = "";
    std::string reportFile = "";
    bool interactive = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--source" && i + 1 < argc) {
            sourcePath = argv[++i];
        } else if (arg == "--format" && i + 1 < argc) {
            format = argv[++i];
        } else if (arg == "--output" && i + 1 < argc) {
            outputFile = argv[++i];
        } else if (arg == "--report" && i + 1 < argc) {
            reportFile = argv[++i];
        } else if (arg == "--interactive") {
            interactive = true;
        } else if (arg == "--help") {
            printHelp();
            return 0;
        }
    }

    if (!std::filesystem::exists(sourcePath)) {
        std::cerr << "Ошибка: Директория не найдена: " << sourcePath << std::endl;
        Logger::instance().error("Директория не найдена: " + sourcePath);
        return 1;
    }

    Logger::instance().info("Старт анализа директории: " + sourcePath);
    ApiParser parser;
    auto controllers = parser.parseDirectory(sourcePath);
    auto dtos = parser.parseDTOs(sourcePath);

    ArchitectureAnalyzer::analyze(controllers);

    if (interactive) {
        TerminalUI::runInteractive(controllers, dtos);
        return 0;
    }

    TerminalUI::printAnalysis(controllers);

    if (!outputFile.empty()) {
        if (format == "html") {
            DocGenerator::generateHTML(outputFile, controllers, dtos);
        } else if (format == "json") {
            DocGenerator::generateJSON(outputFile, controllers, dtos);
        } else {
            DocGenerator::generateMarkdown(outputFile, controllers, dtos);
        }
        std::cout << "Документация сохранена в файл: " << outputFile << std::endl;
        Logger::instance().info("Документация сгенерирована: " + outputFile);
    }

    if (!reportFile.empty()) {
        DocGenerator::generateReport(reportFile, controllers);
        std::cout << "Отчёт сохранён в файл: " << reportFile << std::endl;
        Logger::instance().info("Отчёт рекомендаций сохранен: " + reportFile);
    }

    return 0;
}
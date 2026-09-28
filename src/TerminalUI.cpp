#include "TerminalUI.hpp"
#include "DocGenerator.hpp"
#include <iostream>

#define ANSI_RESET   "\033[0m"
#define ANSI_BOLD    "\033[1m"
#define ANSI_RED     "\033[31m"
#define ANSI_GREEN   "\033[32m"
#define ANSI_YELLOW  "\033[33m"
#define ANSI_CYAN    "\033[36m"

void TerminalUI::printHeader() {
    std::cout << ANSI_CYAN << ANSI_BOLD << "=== Анализ API ===" << ANSI_RESET << "\n\n";
}

void TerminalUI::printAnalysis(const std::vector<Controller>& controllers) {
    printHeader();
    for (const auto& ctrl : controllers) {
        std::cout << ANSI_BOLD << "Контроллер: " << ctrl.name << ANSI_RESET << "\n  Методы:\n";
        for (const auto& m : ctrl.methods) {
            std::cout << "    - " << ANSI_GREEN << m.httpMethod << " " << m.path << ANSI_RESET << "\n";
            if (!m.params.empty()) {
                std::cout << "        Параметры: ";
                for (size_t i = 0; i < m.params.size(); ++i) {
                    std::cout << m.params[i].name << " (" << m.params[i].type << ")" << (i + 1 < m.params.size() ? ", " : "");
                }
                std::cout << "\n";
            }
            if (!m.returnType.empty()) {
                std::cout << "        Возвращаемые данные: " << m.returnType << "\n";
            }
            for (const auto& issue : m.detectedIssues) {
                std::cout << "        " << ANSI_RED << "Проблемы: " << issue << ANSI_RESET << "\n";
            }
            std::cout << "\n";
        }
    }

    std::cout << ANSI_YELLOW << ANSI_BOLD << "Рекомендации:" << ANSI_RESET << "\n";
    int recIdx = 1;
    for (const auto& ctrl : controllers) {
        for (const auto& m : ctrl.methods) {
            for (const auto& rec : m.recommendations) {
                std::cout << "  " << recIdx++ << ". " << rec << " (" << m.httpMethod << " " << m.path << ")\n";
            }
        }
    }
    std::cout << "\n";
}

void TerminalUI::runInteractive(std::vector<Controller>& controllers, const std::vector<DTOModel>& dtos) {
    while (true) {
        std::cout << "\n" << ANSI_CYAN << ANSI_BOLD << "=== Анализ API (Интерактивный режим) ===" << ANSI_RESET << "\n";
        std::cout << "Доступные контроллеры:\n";
        for (size_t i = 0; i < controllers.size(); ++i) {
            std::cout << "  " << (i + 1) << ". " << controllers[i].name << "\n";
        }
        std::cout << "  0. Выход\n";
        std::cout << "\nВыберите контроллер (0-" << controllers.size() << "): ";

        int choice = 0;
        if (!(std::cin >> choice) || choice < 0 || choice > (int)controllers.size()) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }
        if (choice == 0) break;

        const auto& ctrl = controllers[choice - 1];
        std::cout << "\n" << ANSI_BOLD << "Контроллер: " << ctrl.name << ANSI_RESET << "\nМетоды:\n";
        for (size_t j = 0; j < ctrl.methods.size(); ++j) {
            const auto& m = ctrl.methods[j];
            std::cout << "  " << (j + 1) << ". " << m.httpMethod << " " << m.path << "\n";
            for (const auto& issue : m.detectedIssues) {
                std::cout << "      " << ANSI_RED << "Проблема: " << issue << ANSI_RESET << "\n";
            }
        }

        std::cout << "\nВыберите метод для подробного анализа (1-" << ctrl.methods.size() << "): ";
        int mChoice = 0;
        if (std::cin >> mChoice && mChoice >= 1 && mChoice <= (int)ctrl.methods.size()) {
            const auto& m = ctrl.methods[mChoice - 1];
            std::cout << "\n" << ANSI_BOLD << "Детальный отчёт: " << m.httpMethod << " " << m.path << ANSI_RESET << "\n";
            std::cout << "Функция реализации: " << m.functionName << "\n";
            std::cout << "Возвращаемый тип: " << m.returnType << "\n";
            std::cout << "Рекомендации:\n";
            for (const auto& r : m.recommendations) {
                std::cout << "  - " << ANSI_YELLOW << r << ANSI_RESET << "\n";
            }
        }

        std::cout << "\nСохранить документацию? (y/n): ";
        char saveChoice;
        std::cin >> saveChoice;
        if (saveChoice == 'y' || saveChoice == 'Y') {
            std::cout << "Имя файла: ";
            std::string outName;
            std::cin >> outName;
            if (outName.find(".html") != std::string::npos) {
                DocGenerator::generateHTML(outName, controllers, dtos);
            } else if (outName.find(".json") != std::string::npos) {
                DocGenerator::generateJSON(outName, controllers, dtos);
            } else {
                DocGenerator::generateMarkdown(outName, controllers, dtos);
            }
            std::cout << ANSI_GREEN << "[Сохранено успешно]" << ANSI_RESET << "\n";
        }
    }
}
#include "Analyzer.hpp"
#include "Logger.hpp"
#include <algorithm>

void ArchitectureAnalyzer::analyze(std::vector<Controller>& controllers) {
    for (auto& ctrl : controllers) {
        for (auto& m : ctrl.methods) {
            m.detectedIssues.clear();
            m.recommendations.clear();

            // 1. Проверка обработки исключений
            if (!m.hasTryCatch) {
                m.detectedIssues.push_back("Отсутствует обработка исключений.");
                m.recommendations.push_back("Добавьте глобальную или локальную обработку исключений (try-catch).");
            }

            // 2. Валидация входных данных
            if (!m.params.empty() && !m.hasValidation) {
                m.detectedIssues.push_back("Отсутствует валидация параметров.");
                m.recommendations.push_back("Добавьте валидацию входных параметров перед обращением к БД.");
            }

            // 3. Неправильное использование HTTP-методов
            std::string lowerPath = m.path;
            std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), ::tolower);

            if (m.httpMethod == "GET" && (lowerPath.find("create") != std::string::npos ||
                                          lowerPath.find("delete") != std::string::npos ||
                                          lowerPath.find("update") != std::string::npos)) {
                m.detectedIssues.push_back("GET метод используется для мутирующих операций.");
                m.recommendations.push_back("Используйте POST, PUT или DELETE для изменения состояния ресурса.");
            }

            // 4. Рекомендации по производительности (кэширование)
            if (m.httpMethod == "GET" && (m.path.find("{id}") != std::string::npos || m.params.size() <= 2)) {
                m.detectedIssues.push_back("Отсутствует кэширование.");
                m.recommendations.push_back("Добавьте кэширование для часто запрашиваемых данных (In-Memory / Redis).");
            }
        }
    }
    Logger::instance().info("Анализ архитектурных рекомендаций завершен.");
}
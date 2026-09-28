#pragma once
#include <string>
#include <vector>
#include <map>

struct Parameter {
    std::string name;
    std::string type;
    std::string example;
};

struct EndpointMethod {
    std::string httpMethod;   // GET, POST, PUT, DELETE
    std::string path;
    std::string functionName;
    std::string returnType;
    std::string requestBodyType;
    std::vector<Parameter> params;
    std::vector<std::string> middlewares;
    std::vector<std::string> filters;
    bool isAsync = false;

    // Архитектурные метрики
    bool hasTryCatch = false;
    bool hasValidation = false;
    std::vector<std::string> detectedIssues;
    std::vector<std::string> recommendations;
};

struct Controller {
    std::string name;
    std::string basePath;
    std::string filePath;
    std::vector<EndpointMethod> methods;
};

struct DTOField {
    std::string name;
    std::string type;
};

struct DTOModel {
    std::string name;
    std::vector<DTOField> fields;
};
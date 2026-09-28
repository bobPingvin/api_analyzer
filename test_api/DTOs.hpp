#pragma once
#include <string>

struct UserCreateRequest {
    std::string username;
    std::string email;
};

struct User {
    int id;
    std::string username;
    std::string email;
};

struct Product {
    int id;
    std::string name;
    double price;
};
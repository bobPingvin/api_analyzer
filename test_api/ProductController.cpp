#include "DTOs.hpp"

// @Controller ProductController
// @Route /products

// @GET /products/{id}
// @Param id int
// @Return Product
Product getProduct(int id) {
    // try-catch присутствует, но кэширования нет
    try {
        if (id <= 0) return Product{};
        return Product{id, "Keyboard", 49.99};
    } catch (...) {
        return Product{};
    }
}
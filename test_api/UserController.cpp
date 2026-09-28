#include "DTOs.hpp"
#include <vector>

// @Controller UserController
// @Route /users

// @GET /users
// @Param page int
// @Param limit int
// @Return List<User>
std::vector<User> getUsers(int page, int limit) {
    // Внимание: валидации параметров нет
    std::vector<User> users;
    return users;
}

// @POST /users
// @Body UserCreateRequest
// @Return User
User createUser(const UserCreateRequest& req) {
    // Нет блока try-catch
    User u;
    u.id = 1;
    u.username = req.username;
    return u;
}
#pragma once
#include <string>

class SQLExecutor {
   public:
    static std::string run(const std::string& sql);
};

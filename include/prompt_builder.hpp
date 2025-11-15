#pragma once
#include <string>
#include <vector>

#include "schema_loader.hpp"

class PromptBuilder {
   public:
    static std::string build(const std::string& userQuery,
                             const std::vector<TableInfo>& schema);
};

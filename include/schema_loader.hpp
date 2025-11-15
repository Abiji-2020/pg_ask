#pragma once
#include <string>
#include <vector>

struct TableInfo {
    std::string schema;
    std::string table;
    std::vector<std::pair<std::string, std::string>> columns;
};

class SchemaLoader {
   public:
    static std::vector<TableInfo> load();
};

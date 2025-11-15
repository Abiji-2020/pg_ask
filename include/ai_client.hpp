#pragma once
#include <string>

class AIClient {
   public:
    virtual ~AIClient() {}
    virtual std::string complete(const std::string& prompt) = 0;
};

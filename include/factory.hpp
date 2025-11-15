#pragma once
#include <memory>

#include "ai_client.hpp"

class ClientFactory {
   public:
    static ClientFactory& instance();
    AIClient* client();

   private:
    ClientFactory() = default;
    std::unique_ptr<AIClient> client_;
};

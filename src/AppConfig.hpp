#ifndef AppConfig_hpp
#define AppConfig_hpp

#include <string>

namespace app {

class AppConfig {
public:
    std::string host = "0.0.0.0";
    uint16_t port = 8000;
};

}

#endif
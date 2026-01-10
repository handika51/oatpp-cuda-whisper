#include "MyControllerTest.hpp"
#include "oatpp/core/base/Environment.hpp"
#include <iostream>

int main() {
    oatpp::base::Environment::init();
    
    app::test::MyControllerTest().run();
    
    oatpp::base::Environment::destroy();
    return 0;
}
#include "MyControllerTest.hpp"
#include "errorhandler/GlobalErrorHandlerTest.hpp" 
#include "oatpp/core/base/Environment.hpp"
#include <iostream>

int main() {
    oatpp::base::Environment::init();
    
    app::test::MyControllerTest().run();
    app::test::errorhandler::GlobalErrorHandlerTest().run();
    
    oatpp::base::Environment::destroy();
    return 0;
}
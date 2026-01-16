#include "AudioServiceTest.hpp"
#include "errorhandler/GlobalErrorHandlerTest.hpp"
#include <iostream>

void runTests() {
    OATPP_RUN_TEST(app::test::AudioServiceTest);
    OATPP_RUN_TEST(app::test::errorhandler::GlobalErrorHandlerTest);
}

int main() {
    oatpp::base::Environment::init();
    runTests();
    oatpp::base::Environment::destroy();
    return 0;
}
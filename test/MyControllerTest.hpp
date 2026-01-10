#ifndef MyControllerTest_hpp
#define MyControllerTest_hpp

#include "oatpp-test/UnitTest.hpp"

namespace app { namespace test {

class MyControllerTest : public oatpp::test::UnitTest {
public:
    MyControllerTest() : oatpp::test::UnitTest("TEST[MyControllerTest]") {}
    void onRun() override;
};

}}

#endif
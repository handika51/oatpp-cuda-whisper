#ifndef GlobalErrorHandlerTest_hpp
#define GlobalErrorHandlerTest_hpp

#include "oatpp-test/UnitTest.hpp"

namespace app { namespace test { namespace errorhandler {

class GlobalErrorHandlerTest : public oatpp::test::UnitTest {
public:
    GlobalErrorHandlerTest();
    void onRun() override;
};

}}}

#endif // GlobalErrorHandlerTest_hpp
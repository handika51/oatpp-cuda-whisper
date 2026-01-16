#ifndef AudioServiceTest_hpp
#define AudioServiceTest_hpp

#include "oatpp-test/UnitTest.hpp"

namespace app { namespace test {

class AudioServiceTest : public oatpp::test::UnitTest {
public:
    AudioServiceTest() : oatpp::test::UnitTest("TEST[AudioServiceTest]") {}
    void onRun() override;
};

}}

#endif
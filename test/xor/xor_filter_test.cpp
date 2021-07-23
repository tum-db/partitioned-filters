#include <gtest/gtest.h>
#include "xor_filter_test.hpp"

namespace test::nxor {

    INSTANTIATE_TYPED_TEST_CASE_P(XorStandardTestTypes, FilterTest, XorStandardTestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(XorFuseTestTypes, FilterTest, XorFuseTestTypes);

}

MAIN();

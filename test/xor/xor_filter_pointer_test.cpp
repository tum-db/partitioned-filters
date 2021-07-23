#include <gtest/gtest.h>
#include "xor_filter_pointer_test.hpp"

namespace test::nxor::pointer {

    INSTANTIATE_TYPED_TEST_CASE_P(XorTestTypes, FilterPointerTest, XorTestTypes);

}

MAIN();

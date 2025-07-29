#include <gtest/gtest.h>
#include "maskbloom_filter_test.hpp"

namespace test::maskbloom {

    INSTANTIATE_TYPED_TEST_CASE_P(MaskBloom4TestTypes, FilterTest, MaskBloom4TestTypes);

}

MAIN();

#include <gtest/gtest.h>
#include "fastfilter_test.hpp"

namespace test::fastfilter {

    INSTANTIATE_TYPED_TEST_CASE_P(FastfilterXorTestTypes, FilterTest, FastfilterXorTestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(FastfilterBloomTestTypes, FilterTest, FastfilterBloomTestTypes);

}

MAIN();

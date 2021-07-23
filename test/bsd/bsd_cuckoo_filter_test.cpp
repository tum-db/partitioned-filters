#include <gtest/gtest.h>
#include "bsd_cuckoo_filter_test.hpp"

namespace test::bsd::cuckoo {

    INSTANTIATE_TYPED_TEST_CASE_P(BSDCuckooStandard2TestTypes, FilterTest, BSDCuckooStandard2TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(BSDCuckooStandard4TestTypes, FilterTest, BSDCuckooStandard4TestTypes);

}

MAIN();

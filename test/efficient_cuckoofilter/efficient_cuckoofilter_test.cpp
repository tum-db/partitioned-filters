#include <gtest/gtest.h>
#include "efficient_cuckoofilter_test.hpp"

namespace test::efficientcuckoofilter {

    INSTANTIATE_TYPED_TEST_CASE_P(EfficientCuckoofilterStandardTestTypes, FilterTest,
            EfficientCuckoofilterStandardTestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(EfficientCuckoofilterOptimizedTestTypes, FilterTest,
            EfficientCuckoofilterOptimizedTestTypes);

}

MAIN();

#include <gtest/gtest.h>
#include "morton_ota_filter_test.hpp"

namespace test::morton_ota {

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooMortonOTA2_30TestTypes, FilterTest, CuckooMortonOTA2_30TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooMortonOTA3_30TestTypes, FilterTest, CuckooMortonOTA3_30TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooMortonOTA7_30TestTypes, FilterTest, CuckooMortonOTA7_30TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooMortonOTA15_30TestTypes, FilterTest, CuckooMortonOTA15_30TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooMortonOTA3_25TestTypes, FilterTest, CuckooMortonOTA3_25TestTypes);

}

MAIN();

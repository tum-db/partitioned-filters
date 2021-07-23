#include <gtest/gtest.h>
#include "bsd_bloom_filter_test.hpp"

namespace test::bsd::bloom {

    INSTANTIATE_TYPED_TEST_CASE_P(BSDBloomNaive32TestTypes, FilterTest, BSDBloomNaive32TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(BSDBloomBlocked32TestTypes, FilterTest, BSDBloomBlocked32TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(BSDBloomBlocked256TestTypes, FilterTest, BSDBloomBlocked256TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(BSDBloomBlocked512TestTypes, FilterTest, BSDBloomBlocked512TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(BSDBloomSectorized256TestTypes, FilterTest, BSDBloomSectorized256TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(BSDBloomSectorized512TestTypes, FilterTest, BSDBloomSectorized512TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(BSDBloomGrouped2TestTypes, FilterTest, BSDBloomGrouped2TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(BSDBloomGrouped4TestTypes, FilterTest, BSDBloomGrouped4TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(BSDBloomGrouped8TestTypes, FilterTest, BSDBloomGrouped8TestTypes);

}

MAIN();

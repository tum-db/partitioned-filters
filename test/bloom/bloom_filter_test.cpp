#include <gtest/gtest.h>
#include "bloom_filter_test.hpp"

namespace test::bloom {

    INSTANTIATE_TYPED_TEST_CASE_P(BloomNaive32TestTypes, FilterTest, BloomNaive32TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(BloomNaive64TestTypes, FilterTest, BloomNaive64TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(BloomBlocked32TestTypes, FilterTest, BloomBlocked32TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(BloomBlocked64TestTypes, FilterTest, BloomBlocked64TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(BloomBlocked256TestTypes, FilterTest, BloomBlocked256TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(BloomBlocked512TestTypes, FilterTest, BloomBlocked512TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(BloomSectorized128TestTypes, FilterTest, BloomSectorized128TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(BloomSectorized256TestTypes, FilterTest, BloomSectorized256TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(BloomSectorized512TestTypes, FilterTest, BloomSectorized512TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(BloomGrouped2BlockedTestTypes, FilterTest, BloomGrouped2TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(BloomGrouped4BlockedTestTypes, FilterTest, BloomGrouped4TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(BloomGrouped8BlockedTestTypes, FilterTest, BloomGrouped8TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(BloomSectorized128VerticalTestTypes, FilterTest, BloomSectorized128VerticalTestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(BloomSectorized256VerticalTestTypes, FilterTest, BloomSectorized256VerticalTestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(BloomSectorized512VerticalTestTypes, FilterTest, BloomSectorized512VerticalTestTypes);

}

MAIN();

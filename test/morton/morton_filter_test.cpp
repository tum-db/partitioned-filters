#include <gtest/gtest.h>
#include "morton_filter_test.hpp"

namespace test::morton {

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooMorton2_30TestTypes, FilterTest, CuckooMorton2_30TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooMorton3_30TestTypes, FilterTest, CuckooMorton3_30TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooMorton7_30TestTypes, FilterTest, CuckooMorton7_30TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooMorton15_30TestTypes, FilterTest, CuckooMorton15_30TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooMorton3_25TestTypes, FilterTest, CuckooMorton3_25TestTypes);

}

MAIN();

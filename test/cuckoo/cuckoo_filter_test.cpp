#include <gtest/gtest.h>
#include "cuckoo_filter_test.hpp"

namespace test::cuckoo {

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooStandard2TestTypes, FilterTest, CuckooStandard2TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooStandard3TestTypes, FilterTest, CuckooStandard3TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooStandard4TestTypes, FilterTest, CuckooStandard4TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooStandard5TestTypes, FilterTest, CuckooStandard5TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooStandard6TestTypes, FilterTest, CuckooStandard6TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooStandard7TestTypes, FilterTest, CuckooStandard7TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooStandard8TestTypes, FilterTest, CuckooStandard8TestTypes);

}

MAIN();

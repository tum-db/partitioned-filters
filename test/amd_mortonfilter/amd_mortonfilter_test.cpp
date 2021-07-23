#include <gtest/gtest.h>
#include "amd_mortonfilter_test.hpp"

namespace test::amdmortonfilter {

    INSTANTIATE_TYPED_TEST_CASE_P(AMDMortonfilterMorton1TestTypes, FilterTest, AMDMortonfilterMorton1TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(AMDMortonfilterMorton3TestTypes, FilterTest, AMDMortonfilterMorton3TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(AMDMortonfilterMorton7TestTypes, FilterTest, AMDMortonfilterMorton7TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(AMDMortonfilterMorton15TestTypes, FilterTest, AMDMortonfilterMorton15TestTypes);

}

MAIN();

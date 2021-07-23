#include <gtest/gtest.h>
#include "tester.hpp"
#include "bloom/bloom_filter_test.hpp"
#include "bloom/bloom_filter_pointer_test.hpp"
#include "fastfilter/fastfilter_test.hpp"
#include "xor/xor_filter_test.hpp"
#include "xor/xor_filter_pointer_test.hpp"
#include "cuckoo/cuckoo_filter_test.hpp"
#include "morton/morton_filter_test.hpp"
#include "morton/morton_ota_filter_test.hpp"
#include "cuckoo/cuckoo_filter_pointer_test.hpp"
#include "efficient_cuckoofilter/efficient_cuckoofilter_test.hpp"
#include "vacuumfilter/vacuumfilter_test.hpp"
#include "amd_mortonfilter/amd_mortonfilter_test.hpp"
#include "div/div_test.hpp"

#ifdef ENABLE_BSD

#include "bsd/bsd_bloom_filter_test.hpp"
#include "bsd/bsd_cuckoo_filter_test.hpp"

#endif

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

namespace test::bloom::pointer {

    INSTANTIATE_TYPED_TEST_CASE_P(BloomTestTypes, FilterPointerTest, BloomTestTypes);

}

namespace test::nxor {

    INSTANTIATE_TYPED_TEST_CASE_P(XorStandardTestTypes, FilterTest, XorStandardTestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(XorFuseTestTypes, FilterTest, XorFuseTestTypes);

}

namespace test::nxor::pointer {

    INSTANTIATE_TYPED_TEST_CASE_P(XorTestTypes, FilterPointerTest, XorTestTypes);

}

namespace test::cuckoo {

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooStandard2TestTypes, FilterTest, CuckooStandard2TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooStandard3TestTypes, FilterTest, CuckooStandard3TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooStandard4TestTypes, FilterTest, CuckooStandard4TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooStandard5TestTypes, FilterTest, CuckooStandard5TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooStandard6TestTypes, FilterTest, CuckooStandard6TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooStandard7TestTypes, FilterTest, CuckooStandard7TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooStandard8TestTypes, FilterTest, CuckooStandard8TestTypes);

}

namespace test::cuckoo::pointer {

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooTestTypes, FilterPointerTest, CuckooTestTypes);

}

namespace test::morton {

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooMorton2_30TestTypes, FilterTest, CuckooMorton2_30TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooMorton3_30TestTypes, FilterTest, CuckooMorton3_30TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooMorton7_30TestTypes, FilterTest, CuckooMorton7_30TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooMorton15_30TestTypes, FilterTest, CuckooMorton15_30TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooMorton3_25TestTypes, FilterTest, CuckooMorton3_25TestTypes);

}

namespace test::morton_ota {

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooMortonOTA2_30TestTypes, FilterTest, CuckooMortonOTA2_30TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooMortonOTA3_30TestTypes, FilterTest, CuckooMortonOTA3_30TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooMortonOTA7_30TestTypes, FilterTest, CuckooMortonOTA7_30TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooMortonOTA15_30TestTypes, FilterTest, CuckooMortonOTA15_30TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooMortonOTA3_25TestTypes, FilterTest, CuckooMortonOTA3_25TestTypes);

}

namespace test::fastfilter {

    INSTANTIATE_TYPED_TEST_CASE_P(FastfilterXorTestTypes, FilterTest, FastfilterXorTestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(FastfilterBloomTestTypes, FilterTest, FastfilterBloomTestTypes);

}

namespace test::efficientcuckoofilter {

    INSTANTIATE_TYPED_TEST_CASE_P(EfficientCuckoofilterStandardTestTypes, FilterTest,
            EfficientCuckoofilterStandardTestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(EfficientCuckoofilterOptimizedTestTypes, FilterTest,
            EfficientCuckoofilterOptimizedTestTypes);

}

namespace test::vacuumfilter {

    INSTANTIATE_TYPED_TEST_CASE_P(VacuumFilterStandardTestTypes, FilterTest, VacuumFilterStandardTestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(VacuumFilterSemiSortedTestTypes, FilterTest, VacuumFilterSemiSortedTestTypes);

}

namespace test::amdmortonfilter {

    INSTANTIATE_TYPED_TEST_CASE_P(AMDMortonfilterMorton1TestTypes, FilterTest, AMDMortonfilterMorton1TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(AMDMortonfilterMorton3TestTypes, FilterTest, AMDMortonfilterMorton3TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(AMDMortonfilterMorton7TestTypes, FilterTest, AMDMortonfilterMorton7TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(AMDMortonfilterMorton15TestTypes, FilterTest, AMDMortonfilterMorton15TestTypes);

}

#ifdef ENABLE_BSD

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

namespace test::bsd::cuckoo {

    INSTANTIATE_TYPED_TEST_CASE_P(BSDCuckooStandard2TestTypes, FilterTest, BSDCuckooStandard2TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(BSDCuckooStandard4TestTypes, FilterTest, BSDCuckooStandard4TestTypes);

}

#endif

namespace test::div {

    INSTANTIATE_TYPED_TEST_CASE_P(Div0TestTypes, DivModTest, Div0TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(Div10TestTypes, DivModTest, Div10TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(Div20TestTypes, DivModTest, Div20TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(Div30TestTypes, DivModTest, Div30TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(Div40TestTypes, DivModTest, Div40TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(Div50TestTypes, DivModTest, Div50TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(Div60TestTypes, DivModTest, Div60TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(Div70TestTypes, DivModTest, Div70TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(Div80TestTypes, DivModTest, Div80TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(Div90TestTypes, DivModTest, Div90TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(Div100TestTypes, DivModTest, Div100TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(Div110TestTypes, DivModTest, Div110TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(Div120TestTypes, DivModTest, Div120TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(Div130TestTypes, DivModTest, Div130TestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(Div140TestTypes, DivModTest, Div140TestTypes);

}

MAIN();

#include <gtest/gtest.h>
#include "vacuumfilter_test.hpp"

namespace test::vacuumfilter {

    INSTANTIATE_TYPED_TEST_CASE_P(VacuumFilterStandardTestTypes, FilterTest, VacuumFilterStandardTestTypes);

    INSTANTIATE_TYPED_TEST_CASE_P(VacuumFilterSemiSortedTestTypes, FilterTest, VacuumFilterSemiSortedTestTypes);

}

MAIN();

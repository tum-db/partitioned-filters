#include <gtest/gtest.h>
#include "umbra_test.hpp"

namespace test::umbra
{
    INSTANTIATE_TYPED_TEST_CASE_P(UmbraBloomTestTypes, FilterTest, UmbraBloomTestTypes);
}

MAIN();

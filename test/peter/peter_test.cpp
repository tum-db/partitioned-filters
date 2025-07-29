#include <gtest/gtest.h>
#include "peter_test.hpp"

namespace test::peter
{
    INSTANTIATE_TYPED_TEST_CASE_P(PeterBloomTestTypes, FilterTest, PeterBloomTestTypes);
}

MAIN();

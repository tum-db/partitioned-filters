#include <gtest/gtest.h>
#include "impala_test.hpp"

namespace test::impala {

    INSTANTIATE_TYPED_TEST_CASE_P(ImpalaBloomTestTypes, FilterTest, ImpalaBloomTestTypes);

}

MAIN();

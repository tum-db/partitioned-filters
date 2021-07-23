#include <gtest/gtest.h>
#include "bloom_filter_pointer_test.hpp"

namespace test::bloom::pointer {

    INSTANTIATE_TYPED_TEST_CASE_P(BloomTestTypes, FilterPointerTest, BloomTestTypes);

}

MAIN();

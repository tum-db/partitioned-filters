#include <gtest/gtest.h>
#include "cuckoo_filter_pointer_test.hpp"

namespace test::cuckoo::pointer {

    INSTANTIATE_TYPED_TEST_CASE_P(CuckooTestTypes, FilterPointerTest, CuckooTestTypes);

}

MAIN();

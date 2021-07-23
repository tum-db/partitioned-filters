#pragma once

#include <cstddef>
#include "../div_mod_test.hpp"

namespace test::div {

    #define DivTestTypes(Divisor)                                                                                   \
    using Div##Divisor##TestTypes = ::testing::Types<DivModTestConfig<Divisor + 1>, DivModTestConfig<Divisor + 2>,  \
            DivModTestConfig<Divisor + 3>, DivModTestConfig<Divisor + 4>, DivModTestConfig<Divisor + 5>,            \
            DivModTestConfig<Divisor + 6>, DivModTestConfig<Divisor + 7>, DivModTestConfig<Divisor + 8>,            \
            DivModTestConfig<Divisor + 9>, DivModTestConfig<Divisor + 10>>

    DivTestTypes(0);
    DivTestTypes(10);
    DivTestTypes(20);
    DivTestTypes(30);
    DivTestTypes(40);
    DivTestTypes(50);
    DivTestTypes(60);
    DivTestTypes(70);
    DivTestTypes(80);
    DivTestTypes(90);
    DivTestTypes(100);
    DivTestTypes(110);
    DivTestTypes(120);
    DivTestTypes(130);
    DivTestTypes(140);

}

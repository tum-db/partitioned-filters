#pragma once

#include <functional>
#include "filter_base.hpp"
#include "filter_factory.hpp"

namespace filters {

    template<typename OP>
    class FilterPointer {

        static_assert(OP::simd == parameter::SIMD::Scalar);
        static_assert(OP::partitioning == parameter::Partitioning::Disabled);
        static_assert(OP::multiThreading == parameter::MultiThreading::Disabled);

        using Vector = simd::Vector<OP::registerSize, OP::simd>;
        using T = typename Vector::T;

        using f_ctor = std::function<void *(size_t /*s*/)>;
        using f_dtor = std::function<void(void */*instance*/)>;
        using f_add_all = std::function<bool(void */*instance*/, const T * /*values*/, const size_t /*length*/)>;
        using f_contains = std::function<bool(const void */*instance*/, const T /*value*/)>;

        void *instance{nullptr};

        f_ctor i_ctor;
        f_dtor i_dtor;
        f_add_all i_add_all;
        f_contains i_contains;

      public:

        ~FilterPointer() {
            if (instance) {
                i_dtor(instance);
            }
        }

        template<FilterType type>
        void create(size_t k, size_t s) {
            if (instance) {
                i_dtor(instance);
            }

            switch (k) {
                case 1:
                    FilterFactory<type, OP>::template bind<1>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                case 2:
                    FilterFactory<type, OP>::template bind<2>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                case 3:
                    FilterFactory<type, OP>::template bind<3>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                case 4:
                    FilterFactory<type, OP>::template bind<4>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                case 5:
                    FilterFactory<type, OP>::template bind<5>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                case 6:
                    FilterFactory<type, OP>::template bind<6>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                case 7:
                    FilterFactory<type, OP>::template bind<7>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                case 8:
                    FilterFactory<type, OP>::template bind<8>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                case 9:
                    FilterFactory<type, OP>::template bind<9>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                case 10:
                    FilterFactory<type, OP>::template bind<10>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                case 11:
                    FilterFactory<type, OP>::template bind<11>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                case 12:
                    FilterFactory<type, OP>::template bind<12>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                case 13:
                    FilterFactory<type, OP>::template bind<13>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                case 14:
                    FilterFactory<type, OP>::template bind<14>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                case 15:
                    FilterFactory<type, OP>::template bind<15>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                case 16:
                    FilterFactory<type, OP>::template bind<16>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                case 17:
                    FilterFactory<type, OP>::template bind<17>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                case 18:
                    FilterFactory<type, OP>::template bind<18>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                case 19:
                    FilterFactory<type, OP>::template bind<19>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                case 20:
                    FilterFactory<type, OP>::template bind<20>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                case 21:
                    FilterFactory<type, OP>::template bind<21>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                case 22:
                    FilterFactory<type, OP>::template bind<22>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                case 23:
                    FilterFactory<type, OP>::template bind<23>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                case 24:
                    FilterFactory<type, OP>::template bind<24>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                case 25:
                    FilterFactory<type, OP>::template bind<25>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                case 26:
                    FilterFactory<type, OP>::template bind<26>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                case 27:
                    FilterFactory<type, OP>::template bind<27>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                case 28:
                    FilterFactory<type, OP>::template bind<28>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                case 29:
                    FilterFactory<type, OP>::template bind<29>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                case 30:
                    FilterFactory<type, OP>::template bind<30>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                case 31:
                    FilterFactory<type, OP>::template bind<31>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                case 32:
                    FilterFactory<type, OP>::template bind<32>(i_ctor, i_dtor, i_add_all, i_contains);
                    break;
                default:
                    throw std::runtime_error{"k is not supported!"};
            }
            instance = i_ctor(s);
        }

        forceinline
        bool add_all(const T *values, const size_t length) {
            return i_add_all(instance, values, length);
        }

        forceinline
        bool contains(T value) const {
            return i_contains(instance, value);
        }

    };

}
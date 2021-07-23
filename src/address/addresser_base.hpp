#pragma once

#include <cstddef>
#include <math/math.hpp>
#include <parameter/parameter.hpp>
#include <compiler/compiler_hints.hpp>
#include <simd/helper.hpp>
#include <libdivide.h>
#include <stdexcept>

namespace filters::addresser {

    using AddressingMode = parameter::AddressingMode;

    template<AddressingMode mode, typename Vector>
    struct Addresser {

        using M = typename Vector::M;

        forceinline
        Vector compute_address(const Vector &/*index*/, const Vector &/*addresses*/, const M /*m*/) const {
            throw std::runtime_error{"not implemented!"};
        }

        forceinline
        Vector compute_address_vertical(const size_t /*index*/, const Vector &/*addresses*/) const {
            throw std::runtime_error{"not implemented!"};
        }

        forceinline
        Vector compute_address_horizontal(const size_t /*index*/, const Vector &/*addresses*/) const {
            throw std::runtime_error{"not implemented!"};
        }

        forceinline
        Vector
        compute_alternative_address(const Vector &/*index*/, const Vector &/*address*/, const Vector &/*signature*/,
                                    const M /*m*/) const {
            throw std::runtime_error{"not implemented!"};
        }

        forceinline
        Vector compute_alternative_address_vertical(const size_t /*index*/, const Vector &/*address*/,
                                                    const Vector &/*signature*/) const {
            throw std::runtime_error{"not implemented!"};
        }

        forceinline
        Vector compute_alternative_address_horizontal(const size_t /*index*/, const Vector &/*address*/,
                                                      const Vector &/*signature*/) const {
            throw std::runtime_error{"not implemented!"};
        }

        forceinline
        size_t get_size(size_t /*index*/) const {
            throw std::runtime_error{"not implemented!"};
        }

        forceinline
        Vector get_vsize(size_t /*index*/) const {
            throw std::runtime_error{"not implemented!"};
        }

        template<size_t /*n*/>
        forceinline
        Vector fingerprint(const Vector &/*hash*/) const {
            throw std::runtime_error{"not implemented!"};
        }
    };

} // filters::addresser
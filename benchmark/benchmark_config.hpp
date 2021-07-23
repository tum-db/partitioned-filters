#pragma once

#include <filter.hpp>
#include "fixture/fixture.hpp"
#include "generator/generator.hpp"

namespace filters {
    template<FilterType _filter, typename FP, typename OP, size_t _k, GeneratorType _generator, FixtureType _fixture>
    struct BenchmarkConfig {
        static constexpr FilterType filter = _filter;
        using FilterParameter = FP;
        using OptimizationParameter = OP;
        static constexpr size_t k = _k;
        static constexpr GeneratorType generator = _generator;
        static constexpr FixtureType fixture = _fixture;
    };
}
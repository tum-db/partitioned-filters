#pragma once

#include "filter_base.hpp"

/*
 * Own Implementations
 */
#include <bloom/bloom_filter.hpp>
#include <xor/xor_filter.hpp>
#include <cuckoo/cuckoo_filter.hpp>

/*
 * Reference Implementations
 */
#include <reference/fastfilter/fastfilter_filter.hpp>
#include <reference/impala/impala_bloom_filter.hpp>
#include <reference/efficient_cuckoofilter/efficient_cuckoofilter_filter.hpp>
#include <reference/amd_mortonfilter/amd_mortonfilter_filter.hpp>
#include <reference/vacuumfilter/vacuumfilter_filter.hpp>

#ifdef ENABLE_BSD

#include <reference/bsd/bsd_filter.hpp>

#endif

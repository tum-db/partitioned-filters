#pragma once

#if defined(HAVE_NUMA)
#include <numa.h>
#endif

namespace filters {

    class DataContainer {
      private:

        using key = std::tuple<GeneratorType, parameter::RegisterSize, size_t, size_t, size_t>;

        std::map<key, std::tuple<void *, void *, size_t, size_t>> data;

        static void *alloc(size_t &n_bytes) {
            // always align for cache-lines
            static constexpr size_t alignment = 64;
            if (n_bytes % alignment != 0) {
                n_bytes = (n_bytes / alignment + 1) * alignment;
            }
            #if defined(HAVE_NUMA)
            return numa_alloc_interleaved(n_bytes);
            #else
            return aligned_alloc(alignment, n_bytes);
            #endif
        }

      public:

        ~DataContainer() {
            for (auto t : data) {
                #if defined(HAVE_NUMA)
                numa_free(std::get<0>(t.second), std::get<2>(t.second));
                numa_free(std::get<1>(t.second), std::get<3>(t.second));
                #else
                free(std::get<0>(t.second));
                free(std::get<1>(t.second));
                #endif
            }
            data.clear();
        }

        template<GeneratorType generatorType, typename Vector>
        bool contains(size_t n_elements_build, size_t n_elements_lookup, size_t shared) {
            auto k = std::tuple(generatorType, Vector::registerSize, n_elements_build, n_elements_lookup, shared);
            return data.contains(k);
        }

        template<GeneratorType generatorType, typename Vector>
        bool createIfNotExists(size_t n_elements_build, size_t n_elements_lookup, size_t shared) {
            auto k = std::tuple(generatorType, Vector::registerSize, n_elements_build, n_elements_lookup, shared);
            if (not data.contains(k)) {
                using T = typename Vector::T;
                size_t n_bytes_build = sizeof(T) * n_elements_build;
                size_t n_bytes_lookup = sizeof(T) * n_elements_lookup;
                void *build_data = alloc(n_bytes_build);
                void *lookup_data = alloc(n_bytes_lookup);
                data[k] = std::tuple(build_data, lookup_data, n_bytes_lookup, n_bytes_build);
                return true;
            } else {
                return false;
            }
        }

        template<GeneratorType generatorType, typename Vector>
        typename Vector::T *buildData(size_t n_elements_build, size_t n_elements_lookup, size_t shared) {
            auto k = std::tuple(generatorType, Vector::registerSize, n_elements_build, n_elements_lookup, shared);
            return reinterpret_cast<typename Vector::T *>(std::get<0>(data[k]));
        }

        template<GeneratorType generatorType, typename Vector>
        typename Vector::T *lookupData(size_t n_elements_build, size_t n_elements_lookup, size_t shared) {
            auto k = std::tuple(generatorType, Vector::registerSize, n_elements_build, n_elements_lookup, shared);
            return reinterpret_cast<typename Vector::T *>(std::get<1>(data[k]));
        }
    };

    static DataContainer dataContainer{};
}
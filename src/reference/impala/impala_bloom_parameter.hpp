#pragma once

namespace filters::impala {

    struct ImpalaBloomParameter {

        static std::string to_string() {
            return {};
        }
    };

    template<size_t> using Standard = ImpalaBloomParameter;

} // filters::impala

#pragma once

namespace filters::umbra {

    struct UmbraBloomParameter {

        static std::string to_string() {
            return {};
        }
    };

    template<size_t> using Standard = UmbraBloomParameter;

} // filters::impala

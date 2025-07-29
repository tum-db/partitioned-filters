#pragma once

namespace filters::peter {

    struct PeterBloomParameter {

        static std::string to_string() {
            return {};
        }
    };

    template<size_t> using Standard = PeterBloomParameter;

} // filters::peter

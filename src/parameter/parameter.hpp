#pragma once

#include <cstddef>
#include <string>

namespace filters::parameter {

    enum class AddressingMode : size_t {
        PowerOfTwo, Magic, Lemire, Vacuum, Morton
    };

    enum class HashingMode : size_t {
        Identity, Murmur, Fasthash, TwoIndependentMultiplyShift, Mul, SimpleTabulation, Cityhash
    };

    enum class Partitioning : size_t {
        Enabled = 1, Disabled = 0
    };

    enum class RegisterSize : size_t {
        _32bit = 32, _64bit = 64
    };

    enum class SIMD : size_t {
        Scalar = 0, AVX2 = 256, AVX512 = 512
    };

    enum class MultiThreading : size_t {
        Enabled = 1, Disabled = 0
    };

    template<AddressingMode _addressingMode, HashingMode _hashingMode, Partitioning _partitioning, RegisterSize _registerSize, SIMD _simd, MultiThreading _multiThreading>
    struct OptimizationParameter {
        static constexpr AddressingMode addressingMode = _addressingMode;
        static constexpr HashingMode hashingMode = _hashingMode;
        static constexpr Partitioning partitioning = _partitioning;
        static constexpr RegisterSize registerSize = _registerSize;
        static constexpr SIMD simd = _simd;
        static constexpr MultiThreading multiThreading = _multiThreading;

        static std::string to_string() {
            std::string s_addressingMode;
            switch (addressingMode) {
                case AddressingMode::PowerOfTwo:
                    s_addressingMode = "PowerOfTwo";
                    break;
                case AddressingMode::Magic:
                    s_addressingMode = "Magic";
                    break;
                case AddressingMode::Lemire:
                    s_addressingMode = "Lemire";
                    break;
                case AddressingMode::Vacuum:
                    s_addressingMode = "Vacuum";
                    break;
            }

            std::string s_hashingMode;
            switch (hashingMode) {
                case HashingMode::Identity:
                    s_hashingMode = "Identity";
                    break;
                case HashingMode::Murmur:
                    s_hashingMode = "Murmur";
                    break;
                case HashingMode::Fasthash:
                    s_hashingMode = "Fasthash";
                    break;
                case HashingMode::TwoIndependentMultiplyShift:
                    s_hashingMode = "TwoIndependentMultiplyShift";
                    break;
                case HashingMode::SimpleTabulation:
                    s_hashingMode = "SimpleTabulation";
                    break;
                case HashingMode::Mul:
                    s_hashingMode = "Mul";
                    break;
            }

            std::string s_partitioning = (partitioning == Partitioning::Enabled)
                                         ? "Enabled"
                                         : "Disabled";
            std::string s_registerSize = std::to_string(static_cast<size_t>(registerSize)) + " bits";

            std::string s_simd = (partitioning == Partitioning::Enabled)
                                 ? "Enabled"
                                 : "Disabled";
            switch (simd) {
                case SIMD::Scalar:
                    s_simd = "Scalar";
                    break;
                case SIMD::AVX2:
                    s_simd = "AVX2";
                    break;
                case SIMD::AVX512:
                    s_simd = "AVX512";
                    break;
            }

            std::string s_multiThreading = (multiThreading == MultiThreading::Enabled)
                                           ? "Enabled"
                                           : "Disabled";

            std::string s = "{";
            s += "\"addressing_mode\": \"" + s_addressingMode + "\", ";
            s += "\"hashing_mode\": \"" + s_hashingMode + "\", ";
            s += "\"partitioning\": \"" + s_partitioning + "\", ";
            s += "\"register_size\": \"" + s_registerSize + "\", ";
            s += "\"simd\": \"" + s_simd + "\", ";
            s += "\"multi_threading\": \"" + s_multiThreading + "\"}";

            return s;
        }
    };


    /*
     * Murmur, Unoptimized
     */

    using PowerOfTwoMurmurScalar64 = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using MagicMurmurScalar64 = OptimizationParameter<AddressingMode::Magic, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using LemireMurmurScalar64 = OptimizationParameter<AddressingMode::Lemire, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using VacuumMurmurScalar64 = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Disabled>;

    using PowerOfTwoMurmurAVX264 = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX2,
            MultiThreading::Disabled>;
    using MagicMurmurAVX264 = OptimizationParameter<AddressingMode::Magic, HashingMode::Murmur, Partitioning::Disabled,
            RegisterSize::_64bit, SIMD::AVX2, MultiThreading::Disabled>;
    using LemireMurmurAVX264 = OptimizationParameter<AddressingMode::Lemire, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX2,
            MultiThreading::Disabled>;
    using VacuumMurmurAVX264 = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX2,
            MultiThreading::Disabled>;

    using PowerOfTwoMurmurAVX51264 = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using MagicMurmurAVX51264 = OptimizationParameter<AddressingMode::Magic, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using LemireMurmurAVX51264 = OptimizationParameter<AddressingMode::Lemire, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using VacuumMurmurAVX51264 = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Disabled>;

    using PowerOfTwoMurmurScalar32 = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using MagicMurmurScalar32 = OptimizationParameter<AddressingMode::Magic, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using LemireMurmurScalar32 = OptimizationParameter<AddressingMode::Lemire, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using VacuumMurmurScalar32 = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Disabled>;

    using PowerOfTwoMurmurAVX232 = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX2,
            MultiThreading::Disabled>;
    using MagicMurmurAVX232 = OptimizationParameter<AddressingMode::Magic, HashingMode::Murmur, Partitioning::Disabled,
            RegisterSize::_32bit, SIMD::AVX2, MultiThreading::Disabled>;
    using LemireMurmurAVX232 = OptimizationParameter<AddressingMode::Lemire, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX2,
            MultiThreading::Disabled>;
    using VacuumMurmurAVX232 = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX2,
            MultiThreading::Disabled>;

    using PowerOfTwoMurmurAVX51232 = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using MagicMurmurAVX51232 = OptimizationParameter<AddressingMode::Magic, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using LemireMurmurAVX51232 = OptimizationParameter<AddressingMode::Lemire, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using VacuumMurmurAVX51232 = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Disabled>;

    using PowerOfTwoMurmurScalar64MT = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Enabled>;
    using MagicMurmurScalar64MT = OptimizationParameter<AddressingMode::Magic, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Enabled>;
    using LemireMurmurScalar64MT = OptimizationParameter<AddressingMode::Lemire, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Enabled>;
    using VacuumMurmurScalar64MT = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Enabled>;

    using PowerOfTwoMurmurAVX264MT = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX2, MultiThreading::Enabled>;
    using MagicMurmurAVX264MT = OptimizationParameter<AddressingMode::Magic, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX2, MultiThreading::Enabled>;
    using LemireMurmurAVX264MT = OptimizationParameter<AddressingMode::Lemire, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX2, MultiThreading::Enabled>;
    using VacuumMurmurAVX264MT = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX2, MultiThreading::Enabled>;

    using PowerOfTwoMurmurAVX51264MT = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Enabled>;
    using MagicMurmurAVX51264MT = OptimizationParameter<AddressingMode::Magic, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Enabled>;
    using LemireMurmurAVX51264MT = OptimizationParameter<AddressingMode::Lemire, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Enabled>;
    using VacuumMurmurAVX51264MT = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Enabled>;

    using PowerOfTwoMurmurScalar32MT = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Enabled>;
    using MagicMurmurScalar32MT = OptimizationParameter<AddressingMode::Magic, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Enabled>;
    using LemireMurmurScalar32MT = OptimizationParameter<AddressingMode::Lemire, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Enabled>;
    using VacuumMurmurScalar32MT = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Enabled>;

    using PowerOfTwoMurmurAVX232MT = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX2, MultiThreading::Enabled>;
    using MagicMurmurAVX232MT = OptimizationParameter<AddressingMode::Magic, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX2, MultiThreading::Enabled>;
    using LemireMurmurAVX232MT = OptimizationParameter<AddressingMode::Lemire, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX2, MultiThreading::Enabled>;
    using VacuumMurmurAVX232MT = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX2, MultiThreading::Enabled>;

    using PowerOfTwoMurmurAVX51232MT = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Enabled>;
    using MagicMurmurAVX51232MT = OptimizationParameter<AddressingMode::Magic, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Enabled>;
    using LemireMurmurAVX51232MT = OptimizationParameter<AddressingMode::Lemire, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Enabled>;
    using VacuumMurmurAVX51232MT = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Murmur,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Enabled>;


    /*
     * Fasthash, Unoptimized
     */

    using PowerOfTwoFasthashScalar64 = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Fasthash,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using MagicFasthashScalar64 = OptimizationParameter<AddressingMode::Magic, HashingMode::Fasthash,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using LemireFasthashScalar64 = OptimizationParameter<AddressingMode::Lemire, HashingMode::Fasthash,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using VacuumFasthashScalar64 = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Fasthash,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Disabled>;

    using PowerOfTwoFasthashAVX264 = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Fasthash,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX2,
            MultiThreading::Disabled>;
    using MagicFasthashAVX264 = OptimizationParameter<AddressingMode::Magic, HashingMode::Fasthash,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX2,
            MultiThreading::Disabled>;
    using LemireFasthashAVX264 = OptimizationParameter<AddressingMode::Lemire, HashingMode::Fasthash,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX2,
            MultiThreading::Disabled>;
    using VacuumFasthashAVX264 = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Fasthash,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX2,
            MultiThreading::Disabled>;

    using PowerOfTwoFasthashAVX51264 = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Fasthash,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using MagicFasthashAVX51264 = OptimizationParameter<AddressingMode::Magic, HashingMode::Fasthash,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using LemireFasthashAVX51264 = OptimizationParameter<AddressingMode::Lemire, HashingMode::Fasthash,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using VacuumFasthashAVX51264 = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Fasthash,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Disabled>;

    using PowerOfTwoFasthashScalar32 = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Fasthash,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using MagicFasthashScalar32 = OptimizationParameter<AddressingMode::Magic, HashingMode::Fasthash,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using LemireFasthashScalar32 = OptimizationParameter<AddressingMode::Lemire, HashingMode::Fasthash,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using VacuumFasthashScalar32 = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Fasthash,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Disabled>;

    using PowerOfTwoFasthashAVX232 = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Fasthash,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX2,
            MultiThreading::Disabled>;
    using MagicFasthashAVX232 = OptimizationParameter<AddressingMode::Magic, HashingMode::Fasthash,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX2,
            MultiThreading::Disabled>;
    using LemireFasthashAVX232 = OptimizationParameter<AddressingMode::Lemire, HashingMode::Fasthash,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX2,
            MultiThreading::Disabled>;
    using VacuumFasthashAVX232 = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Fasthash,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX2,
            MultiThreading::Disabled>;

    using PowerOfTwoFasthashAVX51232 = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Fasthash,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using MagicFasthashAVX51232 = OptimizationParameter<AddressingMode::Magic, HashingMode::Fasthash,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using LemireFasthashAVX51232 = OptimizationParameter<AddressingMode::Lemire, HashingMode::Fasthash,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using VacuumFasthashAVX51232 = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Fasthash,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Disabled>;


    /*
     * TwoIndependentMultiplyShift, Unoptimized
     */

    using PowerOfTwoTwoIndependentMultiplyShiftScalar64 = OptimizationParameter<AddressingMode::PowerOfTwo,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using MagicTwoIndependentMultiplyShiftScalar64 = OptimizationParameter<AddressingMode::Magic,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using LemireTwoIndependentMultiplyShiftScalar64 = OptimizationParameter<AddressingMode::Lemire,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using VacuumTwoIndependentMultiplyShiftScalar64 = OptimizationParameter<AddressingMode::Vacuum,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Disabled>;

    using PowerOfTwoTwoIndependentMultiplyShiftAVX264 = OptimizationParameter<AddressingMode::PowerOfTwo,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX2,
            MultiThreading::Disabled>;
    using MagicTwoIndependentMultiplyShiftAVX264 = OptimizationParameter<AddressingMode::Magic,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX2,
            MultiThreading::Disabled>;
    using LemireTwoIndependentMultiplyShiftAVX264 = OptimizationParameter<AddressingMode::Lemire,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX2,
            MultiThreading::Disabled>;
    using VacuumTwoIndependentMultiplyShiftAVX264 = OptimizationParameter<AddressingMode::Vacuum,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX2,
            MultiThreading::Disabled>;

    using PowerOfTwoTwoIndependentMultiplyShiftAVX51264 = OptimizationParameter<AddressingMode::PowerOfTwo,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using MagicTwoIndependentMultiplyShiftAVX51264 = OptimizationParameter<AddressingMode::Magic,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using LemireTwoIndependentMultiplyShiftAVX51264 = OptimizationParameter<AddressingMode::Lemire,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using VacuumTwoIndependentMultiplyShiftAVX51264 = OptimizationParameter<AddressingMode::Vacuum,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Disabled>;

    using PowerOfTwoTwoIndependentMultiplyShiftScalar32 = OptimizationParameter<AddressingMode::PowerOfTwo,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using MagicTwoIndependentMultiplyShiftScalar32 = OptimizationParameter<AddressingMode::Magic,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using LemireTwoIndependentMultiplyShiftScalar32 = OptimizationParameter<AddressingMode::Lemire,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using VacuumTwoIndependentMultiplyShiftScalar32 = OptimizationParameter<AddressingMode::Vacuum,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Disabled>;

    using PowerOfTwoTwoIndependentMultiplyShiftAVX232 = OptimizationParameter<AddressingMode::PowerOfTwo,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX2,
            MultiThreading::Disabled>;
    using MagicTwoIndependentMultiplyShiftAVX232 = OptimizationParameter<AddressingMode::Magic,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX2,
            MultiThreading::Disabled>;
    using LemireTwoIndependentMultiplyShiftAVX232 = OptimizationParameter<AddressingMode::Lemire,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX2,
            MultiThreading::Disabled>;
    using VacuumTwoIndependentMultiplyShiftAVX232 = OptimizationParameter<AddressingMode::Vacuum,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX2,
            MultiThreading::Disabled>;

    using PowerOfTwoTwoIndependentMultiplyShiftAVX51232 = OptimizationParameter<AddressingMode::PowerOfTwo,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using MagicTwoIndependentMultiplyShiftAVX51232 = OptimizationParameter<AddressingMode::Magic,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using LemireTwoIndependentMultiplyShiftAVX51232 = OptimizationParameter<AddressingMode::Lemire,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using VacuumTwoIndependentMultiplyShiftAVX51232 = OptimizationParameter<AddressingMode::Vacuum,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Disabled>;

    using PowerOfTwoTwoIndependentMultiplyShiftScalar64MT = OptimizationParameter<AddressingMode::PowerOfTwo,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Enabled>;
    using MagicTwoIndependentMultiplyShiftScalar64MT = OptimizationParameter<AddressingMode::Magic,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Enabled>;
    using LemireTwoIndependentMultiplyShiftScalar64MT = OptimizationParameter<AddressingMode::Lemire,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Enabled>;
    using VacuumTwoIndependentMultiplyShiftScalar64MT = OptimizationParameter<AddressingMode::Vacuum,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Enabled>;

    using PowerOfTwoTwoIndependentMultiplyShiftAVX264MT = OptimizationParameter<AddressingMode::PowerOfTwo,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX2,
            MultiThreading::Enabled>;
    using MagicTwoIndependentMultiplyShiftAVX264MT = OptimizationParameter<AddressingMode::Magic,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX2,
            MultiThreading::Enabled>;
    using LemireTwoIndependentMultiplyShiftAVX264MT = OptimizationParameter<AddressingMode::Lemire,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX2,
            MultiThreading::Enabled>;
    using VacuumTwoIndependentMultiplyShiftAVX264MT = OptimizationParameter<AddressingMode::Vacuum,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX2,
            MultiThreading::Enabled>;

    using PowerOfTwoTwoIndependentMultiplyShiftAVX51264MT = OptimizationParameter<AddressingMode::PowerOfTwo,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Enabled>;
    using MagicTwoIndependentMultiplyShiftAVX51264MT = OptimizationParameter<AddressingMode::Magic,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Enabled>;
    using LemireTwoIndependentMultiplyShiftAVX51264MT = OptimizationParameter<AddressingMode::Lemire,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Enabled>;
    using VacuumTwoIndependentMultiplyShiftAVX51264MT = OptimizationParameter<AddressingMode::Vacuum,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Enabled>;

    using PowerOfTwoTwoIndependentMultiplyShiftScalar32MT = OptimizationParameter<AddressingMode::PowerOfTwo,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Enabled>;
    using MagicTwoIndependentMultiplyShiftScalar32MT = OptimizationParameter<AddressingMode::Magic,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Enabled>;
    using LemireTwoIndependentMultiplyShiftScalar32MT = OptimizationParameter<AddressingMode::Lemire,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Enabled>;
    using VacuumTwoIndependentMultiplyShiftScalar32MT = OptimizationParameter<AddressingMode::Vacuum,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Enabled>;

    using PowerOfTwoTwoIndependentMultiplyShiftAVX232MT = OptimizationParameter<AddressingMode::PowerOfTwo,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX2,
            MultiThreading::Enabled>;
    using MagicTwoIndependentMultiplyShiftAVX232MT = OptimizationParameter<AddressingMode::Magic,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX2,
            MultiThreading::Enabled>;
    using LemireTwoIndependentMultiplyShiftAVX232MT = OptimizationParameter<AddressingMode::Lemire,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX2,
            MultiThreading::Enabled>;
    using VacuumTwoIndependentMultiplyShiftAVX232MT = OptimizationParameter<AddressingMode::Vacuum,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX2,
            MultiThreading::Enabled>;

    using PowerOfTwoTwoIndependentMultiplyShiftAVX51232MT = OptimizationParameter<AddressingMode::PowerOfTwo,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Enabled>;
    using MagicTwoIndependentMultiplyShiftAVX51232MT = OptimizationParameter<AddressingMode::Magic,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Enabled>;
    using LemireTwoIndependentMultiplyShiftAVX51232MT = OptimizationParameter<AddressingMode::Lemire,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Enabled>;
    using VacuumTwoIndependentMultiplyShiftAVX51232MT = OptimizationParameter<AddressingMode::Vacuum,
            HashingMode::TwoIndependentMultiplyShift, Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Enabled>;


    /*
     * SimpleTabulation, Unoptimized
     */

    using PowerOfTwoSimpleTabulationScalar64 = OptimizationParameter<AddressingMode::PowerOfTwo,
            HashingMode::SimpleTabulation, Partitioning::Disabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using MagicSimpleTabulationScalar64 = OptimizationParameter<AddressingMode::Magic, HashingMode::SimpleTabulation,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using LemireSimpleTabulationScalar64 = OptimizationParameter<AddressingMode::Lemire, HashingMode::SimpleTabulation,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using VacuumSimpleTabulationScalar64 = OptimizationParameter<AddressingMode::Vacuum, HashingMode::SimpleTabulation,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Disabled>;

    using PowerOfTwoSimpleTabulationAVX264 = OptimizationParameter<AddressingMode::PowerOfTwo,
            HashingMode::SimpleTabulation, Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX2,
            MultiThreading::Disabled>;
    using MagicSimpleTabulationAVX264 = OptimizationParameter<AddressingMode::Magic, HashingMode::SimpleTabulation,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX2,
            MultiThreading::Disabled>;
    using LemireSimpleTabulationAVX264 = OptimizationParameter<AddressingMode::Lemire, HashingMode::SimpleTabulation,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX2,
            MultiThreading::Disabled>;
    using VacuumSimpleTabulationAVX264 = OptimizationParameter<AddressingMode::Vacuum, HashingMode::SimpleTabulation,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX2,
            MultiThreading::Disabled>;

    using PowerOfTwoSimpleTabulationAVX51264 = OptimizationParameter<AddressingMode::PowerOfTwo,
            HashingMode::SimpleTabulation, Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using MagicSimpleTabulationAVX51264 = OptimizationParameter<AddressingMode::Magic, HashingMode::SimpleTabulation,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using LemireSimpleTabulationAVX51264 = OptimizationParameter<AddressingMode::Lemire, HashingMode::SimpleTabulation,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using VacuumSimpleTabulationAVX51264 = OptimizationParameter<AddressingMode::Vacuum, HashingMode::SimpleTabulation,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Disabled>;

    using PowerOfTwoSimpleTabulationScalar32 = OptimizationParameter<AddressingMode::PowerOfTwo,
            HashingMode::SimpleTabulation, Partitioning::Disabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using MagicSimpleTabulationScalar32 = OptimizationParameter<AddressingMode::Magic, HashingMode::SimpleTabulation,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using LemireSimpleTabulationScalar32 = OptimizationParameter<AddressingMode::Lemire, HashingMode::SimpleTabulation,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using VacuumSimpleTabulationScalar32 = OptimizationParameter<AddressingMode::Vacuum, HashingMode::SimpleTabulation,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Disabled>;

    using PowerOfTwoSimpleTabulationAVX232 = OptimizationParameter<AddressingMode::PowerOfTwo,
            HashingMode::SimpleTabulation, Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX2,
            MultiThreading::Disabled>;
    using MagicSimpleTabulationAVX232 = OptimizationParameter<AddressingMode::Magic, HashingMode::SimpleTabulation,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX2,
            MultiThreading::Disabled>;
    using LemireSimpleTabulationAVX232 = OptimizationParameter<AddressingMode::Lemire, HashingMode::SimpleTabulation,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX2,
            MultiThreading::Disabled>;
    using VacuumSimpleTabulationAVX232 = OptimizationParameter<AddressingMode::Vacuum, HashingMode::SimpleTabulation,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX2,
            MultiThreading::Disabled>;

    using PowerOfTwoSimpleTabulationAVX51232 = OptimizationParameter<AddressingMode::PowerOfTwo,
            HashingMode::SimpleTabulation, Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using MagicSimpleTabulationAVX51232 = OptimizationParameter<AddressingMode::Magic, HashingMode::SimpleTabulation,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using LemireSimpleTabulationAVX51232 = OptimizationParameter<AddressingMode::Lemire, HashingMode::SimpleTabulation,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using VacuumSimpleTabulationAVX51232 = OptimizationParameter<AddressingMode::Vacuum, HashingMode::SimpleTabulation,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Disabled>;


    /*
     * Mul, Unoptimized
     */

    using PowerOfTwoMulScalar64 = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Mul,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using MagicMulScalar64 = OptimizationParameter<AddressingMode::Magic, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_64bit, SIMD::Scalar, MultiThreading::Disabled>;
    using LemireMulScalar64 = OptimizationParameter<AddressingMode::Lemire, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_64bit, SIMD::Scalar, MultiThreading::Disabled>;
    using VacuumMulScalar64 = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_64bit, SIMD::Scalar, MultiThreading::Disabled>;

    using PowerOfTwoMulAVX264 = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Mul,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX2,
            MultiThreading::Disabled>;
    using MagicMulAVX264 = OptimizationParameter<AddressingMode::Magic, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_64bit, SIMD::AVX2, MultiThreading::Disabled>;
    using LemireMulAVX264 = OptimizationParameter<AddressingMode::Lemire, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_64bit, SIMD::AVX2, MultiThreading::Disabled>;
    using VacuumMulAVX264 = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_64bit, SIMD::AVX2, MultiThreading::Disabled>;

    using PowerOfTwoMulAVX51264 = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Mul,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using MagicMulAVX51264 = OptimizationParameter<AddressingMode::Magic, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_64bit, SIMD::AVX512, MultiThreading::Disabled>;
    using LemireMulAVX51264 = OptimizationParameter<AddressingMode::Lemire, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_64bit, SIMD::AVX512, MultiThreading::Disabled>;
    using VacuumMulAVX51264 = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_64bit, SIMD::AVX512, MultiThreading::Disabled>;

    using PowerOfTwoMulScalar32 = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Mul,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using MagicMulScalar32 = OptimizationParameter<AddressingMode::Magic, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_32bit, SIMD::Scalar, MultiThreading::Disabled>;
    using LemireMulScalar32 = OptimizationParameter<AddressingMode::Lemire, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_32bit, SIMD::Scalar, MultiThreading::Disabled>;
    using VacuumMulScalar32 = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_32bit, SIMD::Scalar, MultiThreading::Disabled>;

    using PowerOfTwoMulAVX232 = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Mul,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX2,
            MultiThreading::Disabled>;
    using MagicMulAVX232 = OptimizationParameter<AddressingMode::Magic, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_32bit, SIMD::AVX2, MultiThreading::Disabled>;
    using LemireMulAVX232 = OptimizationParameter<AddressingMode::Lemire, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_32bit, SIMD::AVX2, MultiThreading::Disabled>;
    using VacuumMulAVX232 = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_32bit, SIMD::AVX2, MultiThreading::Disabled>;

    using PowerOfTwoMulAVX51232 = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Mul,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using MagicMulAVX51232 = OptimizationParameter<AddressingMode::Magic, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_32bit, SIMD::AVX512, MultiThreading::Disabled>;
    using LemireMulAVX51232 = OptimizationParameter<AddressingMode::Lemire, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_32bit, SIMD::AVX512, MultiThreading::Disabled>;
    using VacuumMulAVX51232 = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_32bit, SIMD::AVX512, MultiThreading::Disabled>;

    using PowerOfTwoMulScalar64MT = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Mul,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Enabled>;
    using MagicMulScalar64MT = OptimizationParameter<AddressingMode::Magic, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_64bit, SIMD::Scalar, MultiThreading::Enabled>;
    using LemireMulScalar64MT = OptimizationParameter<AddressingMode::Lemire, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_64bit, SIMD::Scalar, MultiThreading::Enabled>;
    using VacuumMulScalar64MT = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_64bit, SIMD::Scalar, MultiThreading::Enabled>;

    using PowerOfTwoMulAVX264MT = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Mul,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX2, MultiThreading::Enabled>;
    using MagicMulAVX264MT = OptimizationParameter<AddressingMode::Magic, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_64bit, SIMD::AVX2, MultiThreading::Enabled>;
    using LemireMulAVX264MT = OptimizationParameter<AddressingMode::Lemire, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_64bit, SIMD::AVX2, MultiThreading::Enabled>;
    using VacuumMulAVX264MT = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_64bit, SIMD::AVX2, MultiThreading::Enabled>;

    using PowerOfTwoMulAVX51264MT = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Mul,
            Partitioning::Disabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Enabled>;
    using MagicMulAVX51264MT = OptimizationParameter<AddressingMode::Magic, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_64bit, SIMD::AVX512, MultiThreading::Enabled>;
    using LemireMulAVX51264MT = OptimizationParameter<AddressingMode::Lemire, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_64bit, SIMD::AVX512, MultiThreading::Enabled>;
    using VacuumMulAVX51264MT = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_64bit, SIMD::AVX512, MultiThreading::Enabled>;

    using PowerOfTwoMulScalar32MT = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Mul,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Enabled>;
    using MagicMulScalar32MT = OptimizationParameter<AddressingMode::Magic, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_32bit, SIMD::Scalar, MultiThreading::Enabled>;
    using LemireMulScalar32MT = OptimizationParameter<AddressingMode::Lemire, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_32bit, SIMD::Scalar, MultiThreading::Enabled>;
    using VacuumMulScalar32MT = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_32bit, SIMD::Scalar, MultiThreading::Enabled>;

    using PowerOfTwoMulAVX232MT = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Mul,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX2, MultiThreading::Enabled>;
    using MagicMulAVX232MT = OptimizationParameter<AddressingMode::Magic, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_32bit, SIMD::AVX2, MultiThreading::Enabled>;
    using LemireMulAVX232MT = OptimizationParameter<AddressingMode::Lemire, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_32bit, SIMD::AVX2, MultiThreading::Enabled>;
    using VacuumMulAVX232MT = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_32bit, SIMD::AVX2, MultiThreading::Enabled>;

    using PowerOfTwoMulAVX51232MT = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Mul,
            Partitioning::Disabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Enabled>;
    using MagicMulAVX51232MT = OptimizationParameter<AddressingMode::Magic, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_32bit, SIMD::AVX512, MultiThreading::Enabled>;
    using LemireMulAVX51232MT = OptimizationParameter<AddressingMode::Lemire, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_32bit, SIMD::AVX512, MultiThreading::Enabled>;
    using VacuumMulAVX51232MT = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Mul, Partitioning::Disabled,
            RegisterSize::_32bit, SIMD::AVX512, MultiThreading::Enabled>;

    /*
     * Murmur, Partitioned
     */

    using PowerOfTwoMurmurScalar64Partitioned = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using MagicMurmurScalar64Partitioned = OptimizationParameter<AddressingMode::Magic, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using LemireMurmurScalar64Partitioned = OptimizationParameter<AddressingMode::Lemire, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using VacuumMurmurScalar64Partitioned = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Disabled>;

    using PowerOfTwoMurmurAVX264Partitioned = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::AVX2, MultiThreading::Disabled>;
    using MagicMurmurAVX264Partitioned = OptimizationParameter<AddressingMode::Magic, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::AVX2, MultiThreading::Disabled>;
    using LemireMurmurAVX264Partitioned = OptimizationParameter<AddressingMode::Lemire, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::AVX2, MultiThreading::Disabled>;
    using VacuumMurmurAVX264Partitioned = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::AVX2, MultiThreading::Disabled>;

    using PowerOfTwoMurmurAVX51264Partitioned = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using MagicMurmurAVX51264Partitioned = OptimizationParameter<AddressingMode::Magic, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using LemireMurmurAVX51264Partitioned = OptimizationParameter<AddressingMode::Lemire, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using VacuumMurmurAVX51264Partitioned = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Disabled>;

    using PowerOfTwoMurmurScalar32Partitioned = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using MagicMurmurScalar32Partitioned = OptimizationParameter<AddressingMode::Magic, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using LemireMurmurScalar32Partitioned = OptimizationParameter<AddressingMode::Lemire, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using VacuumMurmurScalar32Partitioned = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Disabled>;

    using PowerOfTwoMurmurAVX232Partitioned = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::AVX2, MultiThreading::Disabled>;
    using MagicMurmurAVX232Partitioned = OptimizationParameter<AddressingMode::Magic, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::AVX2, MultiThreading::Disabled>;
    using LemireMurmurAVX232Partitioned = OptimizationParameter<AddressingMode::Lemire, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::AVX2, MultiThreading::Disabled>;
    using VacuumMurmurAVX232Partitioned = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::AVX2, MultiThreading::Disabled>;

    using PowerOfTwoMurmurAVX51232Partitioned = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using MagicMurmurAVX51232Partitioned = OptimizationParameter<AddressingMode::Magic, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using LemireMurmurAVX51232Partitioned = OptimizationParameter<AddressingMode::Lemire, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using VacuumMurmurAVX51232Partitioned = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Disabled>;

    using PowerOfTwoMurmurScalar64PartitionedMT = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Enabled>;
    using MagicMurmurScalar64PartitionedMT = OptimizationParameter<AddressingMode::Magic, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Enabled>;
    using LemireMurmurScalar64PartitionedMT = OptimizationParameter<AddressingMode::Lemire, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Enabled>;
    using VacuumMurmurScalar64PartitionedMT = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Enabled>;

    using PowerOfTwoMurmurAVX264PartitionedMT = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::AVX2, MultiThreading::Enabled>;
    using MagicMurmurAVX264PartitionedMT = OptimizationParameter<AddressingMode::Magic, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::AVX2, MultiThreading::Enabled>;
    using LemireMurmurAVX264PartitionedMT = OptimizationParameter<AddressingMode::Lemire, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::AVX2, MultiThreading::Enabled>;
    using VacuumMurmurAVX264PartitionedMT = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::AVX2, MultiThreading::Enabled>;

    using PowerOfTwoMurmurAVX51264PartitionedMT = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Enabled>;
    using MagicMurmurAVX51264PartitionedMT = OptimizationParameter<AddressingMode::Magic, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Enabled>;
    using LemireMurmurAVX51264PartitionedMT = OptimizationParameter<AddressingMode::Lemire, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Enabled>;
    using VacuumMurmurAVX51264PartitionedMT = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Enabled>;

    using PowerOfTwoMurmurScalar32PartitionedMT = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Enabled>;
    using MagicMurmurScalar32PartitionedMT = OptimizationParameter<AddressingMode::Magic, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Enabled>;
    using LemireMurmurScalar32PartitionedMT = OptimizationParameter<AddressingMode::Lemire, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Enabled>;
    using VacuumMurmurScalar32PartitionedMT = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Enabled>;

    using PowerOfTwoMurmurAVX232PartitionedMT = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::AVX2, MultiThreading::Enabled>;
    using MagicMurmurAVX232PartitionedMT = OptimizationParameter<AddressingMode::Magic, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::AVX2, MultiThreading::Enabled>;
    using LemireMurmurAVX232PartitionedMT = OptimizationParameter<AddressingMode::Lemire, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::AVX2, MultiThreading::Enabled>;
    using VacuumMurmurAVX232PartitionedMT = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::AVX2, MultiThreading::Enabled>;

    using PowerOfTwoMurmurAVX51232PartitionedMT = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Enabled>;
    using MagicMurmurAVX51232PartitionedMT = OptimizationParameter<AddressingMode::Magic, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Enabled>;
    using LemireMurmurAVX51232PartitionedMT = OptimizationParameter<AddressingMode::Lemire, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Enabled>;
    using VacuumMurmurAVX51232PartitionedMT = OptimizationParameter<AddressingMode::Vacuum, HashingMode::Murmur,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Enabled>;


    /*
     * Mul, Partitioned
     */

    using PowerOfTwoMulScalar64Partitioned = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Mul,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using MagicMulScalar64Partitioned = OptimizationParameter<AddressingMode::Magic, HashingMode::Mul,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using LemireMulScalar64Partitioned = OptimizationParameter<AddressingMode::Lemire, HashingMode::Mul,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::Scalar,
            MultiThreading::Disabled>;

    using PowerOfTwoMulAVX264Partitioned = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Mul,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::AVX2, MultiThreading::Disabled>;
    using MagicMulAVX264Partitioned = OptimizationParameter<AddressingMode::Magic, HashingMode::Mul,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::AVX2, MultiThreading::Disabled>;
    using LemireMulAVX264Partitioned = OptimizationParameter<AddressingMode::Lemire, HashingMode::Mul,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::AVX2, MultiThreading::Disabled>;

    using PowerOfTwoMulAVX51264Partitioned = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Mul,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using MagicMulAVX51264Partitioned = OptimizationParameter<AddressingMode::Magic, HashingMode::Mul,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using LemireMulAVX51264Partitioned = OptimizationParameter<AddressingMode::Lemire, HashingMode::Mul,
            Partitioning::Enabled, RegisterSize::_64bit, SIMD::AVX512,
            MultiThreading::Disabled>;

    using PowerOfTwoMulScalar32Partitioned = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Mul,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using MagicMulScalar32Partitioned = OptimizationParameter<AddressingMode::Magic, HashingMode::Mul,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Disabled>;
    using LemireMulScalar32Partitioned = OptimizationParameter<AddressingMode::Lemire, HashingMode::Mul,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::Scalar,
            MultiThreading::Disabled>;

    using PowerOfTwoMulAVX232Partitioned = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Mul,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::AVX2, MultiThreading::Disabled>;
    using MagicMulAVX232Partitioned = OptimizationParameter<AddressingMode::Magic, HashingMode::Mul,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::AVX2, MultiThreading::Disabled>;
    using LemireMulAVX232Partitioned = OptimizationParameter<AddressingMode::Lemire, HashingMode::Mul,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::AVX2, MultiThreading::Disabled>;

    using PowerOfTwoMulAVX51232Partitioned = OptimizationParameter<AddressingMode::PowerOfTwo, HashingMode::Mul,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using MagicMulAVX51232Partitioned = OptimizationParameter<AddressingMode::Magic, HashingMode::Mul,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Disabled>;
    using LemireMulAVX51232Partitioned = OptimizationParameter<AddressingMode::Lemire, HashingMode::Mul,
            Partitioning::Enabled, RegisterSize::_32bit, SIMD::AVX512,
            MultiThreading::Disabled>;

} // filters::parameter

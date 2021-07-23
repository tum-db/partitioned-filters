#pragma once

namespace filters::simd {

    template<size_t divisor, typename Vector>
    forceinline
    static Vector div(const Vector &dividend) {
        if constexpr (math::is_power_of_two(divisor)) {
            return dividend >> math::const_log2(divisor);
        } else if constexpr (not Vector::avx) {
            return Vector(dividend.vector / divisor);
        } else {
            if constexpr (divisor % 3 == 0 and math::is_power_of_two(divisor / 3)) {
                return dividend.mulh32(Vector(2863311531)) >> (1 + math::const_log2(divisor / 3));
            } else if constexpr (divisor % 5 == 0 and math::is_power_of_two(divisor / 5)) {
                return dividend.mulh32(Vector(3435973837)) >> (2 + math::const_log2(divisor / 5));
            } else if constexpr (divisor == 7) {
                Vector quotient = dividend.mulh32(Vector(613566757));
                Vector temp = (dividend - quotient) >> 1;
                return (quotient + temp) >> 2;
            } else if constexpr (divisor % 9 == 0 and math::is_power_of_two(divisor / 9)) {
                return dividend.mulh32(Vector(954437177)) >> (1 + math::const_log2(divisor / 9));
            } else if constexpr (divisor % 11 == 0 and math::is_power_of_two(divisor / 11)) {
                return dividend.mulh32(Vector(3123612579)) >> (3 + math::const_log2(divisor / 11));
            } else if constexpr (divisor % 13 == 0 and math::is_power_of_two(divisor / 13)) {
                return dividend.mulh32(Vector(1321528399)) >> (2 + math::const_log2(divisor / 13));
            } else if constexpr (divisor == 14) {
                return (dividend >> 1).mulh32(Vector(2454267027)) >> 2;
            } else if constexpr (divisor % 15 == 0 and math::is_power_of_two(divisor / 15)) {
                return dividend.mulh32(Vector(2290649225)) >> (3 + math::const_log2(divisor / 15));
            } else if constexpr (divisor % 17 == 0 and math::is_power_of_two(divisor / 17)) {
                return dividend.mulh32(Vector(4042322161)) >> (4 + math::const_log2(divisor / 17));
            } else if constexpr (divisor == 19) {
                Vector quotient = dividend.mulh32(Vector(2938661835));
                Vector temp = (dividend - quotient) >> 1;
                return (quotient + temp) >> 4;
            } else if constexpr (divisor == 21) {
                Vector quotient = dividend.mulh32(Vector(2249744775));
                Vector temp = (dividend - quotient) >> 1;
                return (quotient + temp) >> 4;
            } else if constexpr (divisor % 23 == 0 and math::is_power_of_two(divisor / 23)) {
                return dividend.mulh32(Vector(2987803337)) >> (4 + math::const_log2(divisor / 23));
            } else if constexpr (divisor % 25 == 0 and math::is_power_of_two(divisor / 25)) {
                return dividend.mulh32(Vector(1374389535)) >> (3 + math::const_log2(divisor / 25));
            } else if constexpr (divisor == 27) {
                Vector quotient = dividend.mulh32(Vector(795364315));
                Vector temp = (dividend - quotient) >> 1;
                return (quotient + temp) >> 4;
            } else if constexpr (divisor == 28) {
                return (dividend >> 2).mulh32(Vector(613566757));
            } else if constexpr (divisor % 29 == 0 and math::is_power_of_two(divisor / 29)) {
                return dividend.mulh32(Vector(2369637129)) >> (4 + math::const_log2(divisor / 29));
            } else if constexpr (divisor == 31) {
                Vector quotient = dividend.mulh32(Vector(138547333));
                Vector temp = (dividend - quotient) >> 1;
                return (quotient + temp) >> 4;
            } else if constexpr (divisor % 33 == 0 and math::is_power_of_two(divisor / 33)) {
                return dividend.mulh32(Vector(1041204193)) >> (3 + math::const_log2(divisor / 33));
            } else if constexpr (divisor == 35) {
                Vector quotient = dividend.mulh32(Vector(3558687189));
                Vector temp = (dividend - quotient) >> 1;
                return (quotient + temp) >> 5;
            } else if constexpr (divisor == 37) {
                Vector quotient = dividend.mulh32(Vector(3134165325));
                Vector temp = (dividend - quotient) >> 1;
                return (quotient + temp) >> 5;
            } else if constexpr (divisor == 38) {
                return (dividend >> 1).mulh32(Vector(1808407283)) >> 3;
            } else if constexpr (divisor == 39) {
                Vector quotient = dividend.mulh32(Vector(2753184165));
                Vector temp = (dividend - quotient) >> 1;
                return (quotient + temp) >> 5;
            } else if constexpr (divisor % 41 == 0 and math::is_power_of_two(divisor / 41)) {
                return dividend.mulh32(Vector(3352169597)) >> (5 + math::const_log2(divisor / 41));
            } else if constexpr (divisor == 42) {
                return (dividend >> 1).mulh32(Vector(818089009)) >> 2;
            } else if constexpr (divisor % 43 == 0 and math::is_power_of_two(divisor / 43)) {
                return dividend.mulh32(Vector(799063683)) >> (3 + math::const_log2(divisor / 43));
            } else if constexpr (divisor == 45) {
                Vector quotient = dividend.mulh32(Vector(1813430637));
                Vector temp = (dividend - quotient) >> 1;
                return (quotient + temp) >> 5;
            } else if constexpr (divisor % 47 == 0 and math::is_power_of_two(divisor / 47)) {
                return dividend.mulh32(Vector(2924233053)) >> (5 + math::const_log2(divisor / 47));
            } else if constexpr (divisor % 49 == 0 and math::is_power_of_two(divisor / 49)) {
                return dividend.mulh32(Vector(1402438301)) >> (4 + math::const_log2(divisor / 49));
            } else if constexpr (divisor % 51 == 0 and math::is_power_of_two(divisor / 51)) {
                return dividend.mulh32(Vector(2694881441)) >> (5 + math::const_log2(divisor / 51));
            } else if constexpr (divisor == 53) {
                Vector quotient = dividend.mulh32(Vector(891408307));
                Vector temp = (dividend - quotient) >> 1;
                return (quotient + temp) >> 5;
            } else if constexpr (divisor == 54) {
                return (dividend >> 1).mulh32(Vector(1272582903)) >> 3;
            } else if constexpr (divisor == 55) {
                Vector quotient = dividend.mulh32(Vector(702812831));
                Vector temp = (dividend - quotient) >> 1;
                return (quotient + temp) >> 5;
            } else if constexpr (divisor == 56) {
                return (dividend >> 3).mulh32(Vector(613566757));
            } else if constexpr (divisor == 57) {
                Vector quotient = dividend.mulh32(Vector(527452125));
                Vector temp = (dividend - quotient) >> 1;
                return (quotient + temp) >> 5;
            } else if constexpr (divisor % 59 == 0 and math::is_power_of_two(divisor / 59)) {
                return dividend.mulh32(Vector(582368447)) >> (3 + math::const_log2(divisor / 59));
            } else if constexpr (divisor % 61 == 0 and math::is_power_of_two(divisor / 61)) {
                return dividend.mulh32(Vector(1126548799)) >> (4 + math::const_log2(divisor / 61));
            } else if constexpr (divisor == 62) {
                return (dividend >> 1).mulh32(Vector(2216757315)) >> 4;
            } else if constexpr (divisor == 63) {
                Vector quotient = dividend.mulh32(Vector(68174085));
                Vector temp = (dividend - quotient) >> 1;
                return (quotient + temp) >> 5;
            } else if constexpr (divisor % 65 == 0 and math::is_power_of_two(divisor / 65)) {
                return dividend.mulh32(Vector(4228890877)) >> (6 + math::const_log2(divisor / 65));
            } else if constexpr (divisor % 67 == 0 and math::is_power_of_two(divisor / 67)) {
                return dividend.mulh32(Vector(128207979)) >> (1 + math::const_log2(divisor / 67));
            } else if constexpr (divisor % 69 == 0 and math::is_power_of_two(divisor / 69)) {
                return dividend.mulh32(Vector(1991868891)) >> (5 + math::const_log2(divisor / 69));
            } else if constexpr (divisor == 70) {
                return (dividend >> 1).mulh32(Vector(3926827243)) >> 5;
            } else if constexpr (divisor % 71 == 0 and math::is_power_of_two(divisor / 71)) {
                return dividend.mulh32(Vector(3871519817)) >> (6 + math::const_log2(divisor / 71));
            } else if constexpr (divisor == 73) {
                Vector quotient = dividend.mulh32(Vector(3235934265));
                Vector temp = (dividend - quotient) >> 1;
                return (quotient + temp) >> 6;
            } else if constexpr (divisor == 74) {
                return (dividend >> 1).mulh32(Vector(3714566311)) >> 5;
            } else if constexpr (divisor % 75 == 0 and math::is_power_of_two(divisor / 75)) {
                return dividend.mulh32(Vector(458129845)) >> (3 + math::const_log2(divisor / 75));
            } else if constexpr (divisor == 76) {
                return (dividend >> 2).mulh32(Vector(452101821)) >> 1;
            } else if constexpr (divisor % 77 == 0 and math::is_power_of_two(divisor / 77)) {
                return dividend.mulh32(Vector(892460737)) >> (4 + math::const_log2(divisor / 77));
            } else if constexpr (divisor == 78) {
                return (dividend >> 1).mulh32(Vector(3524075731)) >> 5;
            } else if constexpr (divisor % 79 == 0 and math::is_power_of_two(divisor / 79)) {
                return dividend.mulh32(Vector(3479467177)) >> (6 + math::const_log2(divisor / 79));
            } else if constexpr (divisor % 81 == 0 and math::is_power_of_two(divisor / 81)) {
                return dividend.mulh32(Vector(3393554407)) >> (6 + math::const_log2(divisor / 81));
            } else if constexpr (divisor % 83 == 0 and math::is_power_of_two(divisor / 83)) {
                return dividend.mulh32(Vector(827945503)) >> (4 + math::const_log2(divisor / 83));
            } else if constexpr (divisor == 84) {
                return (dividend >> 2).mulh32(Vector(818089009)) >> 2;
            } else if constexpr (divisor % 85 == 0 and math::is_power_of_two(divisor / 85)) {
                return dividend.mulh32(Vector(3233857729)) >> (6 + math::const_log2(divisor / 85));
            } else if constexpr (divisor % 87 == 0 and math::is_power_of_two(divisor / 87)) {
                return dividend.mulh32(Vector(789879043)) >> (4 + math::const_log2(divisor / 87));
            } else if constexpr (divisor % 89 == 0 and math::is_power_of_two(divisor / 89)) {
                return dividend.mulh32(Vector(3088515809)) >> (6 + math::const_log2(divisor / 89));
            } else if constexpr (divisor == 90) {
                return (dividend >> 1).mulh32(Vector(3054198967)) >> 5;
            } else if constexpr (divisor == 91) {
                Vector quotient = dividend.mulh32(Vector(1746305385));
                Vector temp = (dividend - quotient) >> 1;
                return (quotient + temp) >> 6;
            } else if constexpr (divisor % 93 == 0 and math::is_power_of_two(divisor / 93)) {
                return dividend.mulh32(Vector(2955676419)) >> (6 + math::const_log2(divisor / 93));
            } else if constexpr (divisor == 95) {
                Vector quotient = dividend.mulh32(Vector(1491936009));
                Vector temp = (dividend - quotient) >> 1;
                return (quotient + temp) >> 6;
            } else if constexpr (divisor == 97) {
                Vector quotient = dividend.mulh32(Vector(1372618415));
                Vector temp = (dividend - quotient) >> 1;
                return (quotient + temp) >> 6;
            } else if constexpr (divisor % 99 == 0 and math::is_power_of_two(divisor / 99)) {
                return dividend.mulh32(Vector(2776544515)) >> (6 + math::const_log2(divisor / 99));
            } else if constexpr (divisor == 101) {
                Vector quotient = dividend.mulh32(Vector(1148159575));
                Vector temp = (dividend - quotient) >> 1;
                return (quotient + temp) >> 6;
            } else if constexpr (divisor == 103) {
                Vector quotient = dividend.mulh32(Vector(1042467791));
                Vector temp = (dividend - quotient) >> 1;
                return (quotient + temp) >> 6;
            } else if constexpr (divisor == 105) {
                Vector quotient = dividend.mulh32(Vector(940802361));
                Vector temp = (dividend - quotient) >> 1;
                return (quotient + temp) >> 6;
            } else if constexpr (divisor == 106) {
                return (dividend >> 1).mulh32(Vector(1296593901)) >> 4;
            } else if constexpr (divisor == 107) {
                Vector quotient = dividend.mulh32(Vector(842937507));
                Vector temp = (dividend - quotient) >> 1;
                return (quotient + temp) >> 6;
            } else if constexpr (divisor == 108) {
                return (dividend >> 2).mulh32(Vector(1272582903)) >> 3;
            } else if constexpr (divisor == 109) {
                Vector quotient = dividend.mulh32(Vector(748664025));
                Vector temp = (dividend - quotient) >> 1;
                return (quotient + temp) >> 6;
            } else if constexpr (divisor == 110) {
                return (dividend >> 1).mulh32(Vector(156180629)) >> 1;
            } else if constexpr (divisor == 111) {
                Vector quotient = dividend.mulh32(Vector(657787785));
                Vector temp = (dividend - quotient) >> 1;
                return (quotient + temp) >> 6;
            } else if constexpr (divisor == 112) {
                return (dividend >> 4).mulh32(Vector(613566758));
            } else if constexpr (divisor == 113) {
                Vector quotient = dividend.mulh32(Vector(570128403));
                Vector temp = (dividend - quotient) >> 1;
                return (quotient + temp) >> 6;
            } else if constexpr (divisor == 114) {
                return (dividend >> 1).mulh32(Vector(2411209711)) >> 5;
            } else if constexpr (divisor == 115) {
                Vector quotient = dividend.mulh32(Vector(485518043));
                Vector temp = (dividend - quotient) >> 1;
                return (quotient + temp) >> 6;
            } else if constexpr (divisor == 117) {
                Vector quotient = dividend.mulh32(Vector(403800345));
                Vector temp = (dividend - quotient) >> 1;
                return (quotient + temp) >> 6;
            } else if constexpr (divisor % 119 == 0 and math::is_power_of_two(divisor / 119)) {
                return dividend.mulh32(Vector(1154949189)) >> (5 + math::const_log2(divisor / 119));
            } else if constexpr (divisor == 121) {
                Vector quotient = dividend.mulh32(Vector(248469183));
                Vector temp = (dividend - quotient) >> 1;
                return (quotient + temp) >> 6;
            } else if constexpr (divisor == 123) {
                Vector quotient = dividend.mulh32(Vector(174592167));
                Vector temp = (dividend - quotient) >> 1;
                return (quotient + temp) >> 6;
            } else if constexpr (divisor == 124) {
                return (dividend >> 2).mulh32(Vector(554189329)) >> 2;
            } else if constexpr (divisor % 125 == 0 and math::is_power_of_two(divisor / 125)) {
                return dividend.mulh32(Vector(274877907)) >> (3 + math::const_log2(divisor / 125));
            } else if constexpr (divisor == 126) {
                return (dividend >> 1).mulh32(Vector(2181570691)) >> 5;
            } else if constexpr (divisor == 127) {
                Vector quotient = dividend.mulh32(Vector(33818641));
                Vector temp = (dividend - quotient) >> 1;
                return (quotient + temp) >> 6;
            } else if constexpr (divisor % 129 == 0 and math::is_power_of_two(divisor / 129)) {
                return dividend.mulh32(Vector(266354561)) >> (3 + math::const_log2(divisor / 129));
            } else if constexpr (divisor % 131 == 0 and math::is_power_of_two(divisor / 131)) {
                return dividend.mulh32(Vector(4196609267)) >> (7 + math::const_log2(divisor / 131));
            } else if constexpr (divisor % 133 == 0 and math::is_power_of_two(divisor / 133)) {
                return dividend.mulh32(Vector(4133502361)) >> (7 + math::const_log2(divisor / 133));
            } else if constexpr (divisor % 135 == 0 and math::is_power_of_two(divisor / 135)) {
                return dividend.mulh32(Vector(4072265289)) >> (7 + math::const_log2(divisor / 135));
            } else if constexpr (divisor % 137 == 0 and math::is_power_of_two(divisor / 137)) {
                return dividend.mulh32(Vector(125400505)) >> (2 + math::const_log2(divisor / 137));
            } else if constexpr (divisor % 139 == 0 and math::is_power_of_two(divisor / 139)) {
                return dividend.mulh32(Vector(1977538899)) >> (6 + math::const_log2(divisor / 139));
            } else if constexpr (divisor == 140) {
                return (dividend >> 2).mulh32(Vector(981706811)) >> 3;
            } else if constexpr (divisor % 141 == 0 and math::is_power_of_two(divisor / 141)) {
                return dividend.mulh32(Vector(974744351)) >> (5 + math::const_log2(divisor / 141));
            } else if constexpr (divisor % 143 == 0 and math::is_power_of_two(divisor / 143)) {
                return dividend.mulh32(Vector(3844446251)) >> (7 + math::const_log2(divisor / 143));
            } else if constexpr (divisor % 145 == 0 and math::is_power_of_two(divisor / 145)) {
                return dividend.mulh32(Vector(3791419407)) >> (7 + math::const_log2(divisor / 145));
            } else if constexpr (divisor == 146) {
                return (dividend >> 1).mulh32(Vector(3765450781)) >> 6;
            } else if constexpr (divisor % 147 == 0 and math::is_power_of_two(divisor / 147)) {
                return dividend.mulh32(Vector(3739835469)) >> (7 + math::const_log2(divisor / 147));
            } else if constexpr (divisor == 148) {
                return (dividend >> 2).mulh32(Vector(464320789)) >> 2;
            } else if constexpr (divisor % 149 == 0 and math::is_power_of_two(divisor / 149)) {
                return dividend.mulh32(Vector(3689636335)) >> (7 + math::const_log2(divisor / 149));
            } else {
                throw std::runtime_error{"not supported: " + std::to_string(divisor)};
            }
        }
    }

    template<size_t divisor, typename Vector>
    forceinline
    static Vector mod(const Vector &dividend) {
        if constexpr (math::is_power_of_two(divisor)) {
            return dividend & Vector(divisor - 1);
        } else if constexpr (not Vector::avx) {
            return Vector(dividend.vector % divisor);
        } else {
            const Vector div = simd::div<divisor>(dividend);
            return dividend - div * Vector(divisor);
        }
    }

    template<size_t divisor, typename Vector>
    forceinline
    static void div_mod(const Vector &dividend, Vector &div, Vector &mod) {
        if constexpr (math::is_power_of_two(divisor)) {
            div = dividend >> math::const_log2(divisor);
            mod = dividend & Vector(divisor - 1);
        } else if constexpr (not Vector::avx) {
            div = Vector(dividend.vector / divisor);
            mod = Vector(dividend.vector % divisor);
        } else {
            div = simd::div<divisor>(dividend);
            mod = dividend - div * Vector(divisor);
        }
    }
}
#pragma once
#include <dobutsu/types.hpp>
#include <bit>
#include <cstdint>

namespace dobutsu{

    class Bitboard{
    private:
        std::uint16_t b_;   // 盤面本体
        static constexpr std::uint16_t BOARD_SCOPE_MASK = 0x0FFF;

    public:
        /* コンストラクタ */
        constexpr Bitboard() : b_(0) {}
        explicit constexpr Bitboard(std::uint16_t raw_b) : b_(raw_b & BOARD_SCOPE_MASK) {}

        /* 演算 */
        constexpr Bitboard& operator|=(Bitboard b){
            b_ = b_ | b.b_;
            return *this;
        }
        constexpr Bitboard& operator&=(Bitboard b){
            b_ = b_ & b.b_;
            return *this;
        }
        constexpr Bitboard& operator^=(Bitboard b){
            b_ = b_ ^ b.b_;
            return *this;
        }
        constexpr Bitboard operator~() const {
            std::uint16_t b = ~b_;
            return Bitboard(b);
        }
        constexpr bool operator==(const Bitboard&) const = default;

        explicit constexpr operator bool() const{ return b_ != 0; }


        /* 判定と要素操作 */
        // マス s を含むか
        constexpr bool test(Square s) const {
            return (b_ & std::uint16_t(1 << s)) != 0;
        }
        // マス s を追加
        // 呼び出し側が、is_ok(s) が真となることを保証する
        constexpr void set(Square s){
            b_ |= std::uint16_t(1 << s);
        }
        // マス s を除去
        // 呼び出し側が、is_ok(s) が真となることを保証する
        constexpr void reset(Square s){
            b_ &= std::uint16_t(~(1 << s));
        }
        // 要素数
        constexpr int popcount() const {
            return std::popcount(b_);
        }
        // 最小のマス番号を持つ要素
        // なお呼び出し側が、盤が空(b_=0)でないことを保証する
        constexpr Square lsb() const {
            return Square(std::countr_zero(b_));
        }
        // lsb() を返し、それを集合から除去する
        constexpr Square pop_lsb(){
            Square lsb_num = lsb();
            b_ &= b_ - 1;
            return lsb_num;
        }
    };

    // マス1個の状態からBitboardを生成する
    constexpr Bitboard square_bb(Square s){
        return Bitboard(std::uint16_t(1 << s));
    }

    /* 演算 */
    constexpr Bitboard operator|(Bitboard b0, Bitboard b1){
        b0 |= b1;
        return b0;
    }
    constexpr Bitboard operator&(Bitboard b0, Bitboard b1){
        b0 &= b1;
        return b0;
    }
    constexpr Bitboard operator^(Bitboard b0, Bitboard b1){
        b0 ^= b1;
        return b0;
    }
}

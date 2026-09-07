#pragma once
#include <array>
#include <dobutsu/types.hpp>
#include <dobutsu/bitboard.hpp>



namespace dobutsu {
    namespace detail {
        constexpr bool moves_like(PieceType pt, Color c, int df, int dr){
            switch(pt){
                case NO_PIECE_TYPE:
                    return false;
                case LION:
                    return true;
                case GIRAFFE:
                    if(df * dr != 0) return false;
                    return true;
                case ELEPHANT:
                    if(df * dr == 0) return false;
                    return true;
                case CHICK:
                    if(df == 0 && dr == (c == BLACK ? -1 : 1)) return true;
                    return false;
                case HEN:
                    if(df != 0 && dr == (c == BLACK ? 1 : -1)) return false;
                    return true;
                case PIECE_TYPE_NB:
                    return false;
            }
            return false;
        }
        constexpr Bitboard make_attack(PieceType pt, Color c, Square s){
            Bitboard bb;
            for(int df : {-1, 0, 1}){
                for(int dr : {-1, 0, 1}){
                    if(df == 0 && dr == 0) continue;
                    if(!moves_like(pt, c, df, dr)) continue;
                    const int f = int(file_of(s)) + df;
                    const int r = int(rank_of(s)) + dr;
                    if(f < 0 || f >= FILE_NB || r < 0 || r >= RANK_NB) continue;
                    bb.set(make_square(File(f), Rank(r)));
                }
            }
            return bb;
        }

        constexpr auto make_attack_table(){
            std::array<std::array<std::array<Bitboard, SQ_NB>, COLOR_NB>, PIECE_TYPE_NB> t{};
            for(int pt = 0; pt < PIECE_TYPE_NB; pt++){
                for(int c = 0; c < COLOR_NB; c++){
                    for(int s = 0; s < SQ_NB; s++){
                        t[pt][c][s] = make_attack(PieceType(pt), Color(c), Square(s));
                    }
                }
            }
            return t;
        }
    }
        
    // [駒種][手番][マス] -> そのマスから利くマスの集合
    inline constexpr auto PieceAttacks = detail::make_attack_table();

    /* 引く際の関数 */
    // 添え字の範囲は呼び出し側で保証する
    constexpr Bitboard attacks_from(PieceType pt, Color c, Square s){
        return PieceAttacks[pt][c][s];
    }
    constexpr Bitboard attacks_from(Piece pc, Square s){
        return PieceAttacks[type_of(pc)][color_of(pc)][s];
    }
}

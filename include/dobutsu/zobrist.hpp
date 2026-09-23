#pragma once
#include <dobutsu/types.hpp>
#include <cstdint>
#include <array>

namespace dobutsu{

    struct ZobristTable{
        std::array<std::array<Key, SQ_NB>, PIECE_NB> psq;
        std::array<std::array<std::array<Key, 3>, PIECE_TYPE_NB>, COLOR_NB> hand;
        Key side;
    };

    namespace detail {
        // 疑似乱数
        constexpr std::uint64_t splitmix64(std::uint64_t& state){
            state += 0x9E3779B97F4A7C15ULL;
            std::uint64_t z = state;
            z ^= z >> 30; z *= 0xBF58476D1CE4E5B9ULL;
            z ^= z >> 27; z *= 0x94D049BB133111EBULL;
            z ^= z >> 31;
            return z;
        }
        
        constexpr ZobristTable make_zobrist_table(){
            ZobristTable zt{};
            std::uint64_t state = 0;
            
            for(Color c : {BLACK, WHITE}){
                for(int pt = 0; pt < PIECE_TYPE_NB; pt++){
                    for(int s = 0; s < SQ_NB; s++){
                        zt.psq[make_piece(c, PieceType(pt))][s] = splitmix64(state);
                    }
                    for(int num = 0; num < 3; num++){
                        zt.hand[c][pt][num] = splitmix64(state);
                    }
                }
            }

            // 0埋め
            for(int s = 0; s < SQ_NB; s++){
                zt.psq[make_piece(BLACK, NO_PIECE_TYPE)][s] = 0;
                zt.psq[make_piece(WHITE, NO_PIECE_TYPE)][s] = 0;
            }
            for(Color c : {BLACK, WHITE}){
                for(int num = 0; num < 3; num++){
                    zt.hand[c][NO_PIECE_TYPE][num] = 0;
                    zt.hand[c][LION][num] = 0;
                    zt.hand[c][HEN][num] = 0;
                }
                for(int pt = 0; pt < PIECE_TYPE_NB; pt++){
                    zt.hand[c][pt][0] = 0;
                }
            }

            zt.side = splitmix64(state);

            return zt;
        }

    }

    inline constexpr ZobristTable Zobrist = detail::make_zobrist_table();
}


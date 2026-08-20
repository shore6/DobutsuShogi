#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <dobutsu/types.hpp>
#include <dobutsu/bitboard.hpp>
#include <optional>

namespace dobutsu {

    class Position{
        private:
            /* 変数 */
            Piece board_[SQ_NB]{};              // マスごとの駒
            Bitboard byType_[PIECE_TYPE_NB]{};  // 駒種ごとの駒の集合
            Bitboard byColor_[COLOR_NB]{};      // 手番ごとの駒の集合
            std::uint16_t hand_[COLOR_NB]{};    // 持ち駒
            std::uint16_t ply_{};               // 手数
            Color sideToMove_{};                // どちらの手番か
            
            /* 定数 */
            static constexpr std::uint8_t HAND_SHIFT[PIECE_TYPE_NB] = {0, 0, 0, 2, 4, 0};    // きりん([GIRAFFE]), ぞう([ELEPHANT]), ひよこ([CHICK]) 以外は引かない、使う側で制御すること
            static constexpr std::uint8_t HAND_MASK = 0b11;

            /* 盤を実際に書き換える操作 */
            constexpr void put_piece(Piece pc, Square s){
                board_[s] = pc;
                byType_[type_of(pc)].set(s);
                byColor_[color_of(pc)].set(s);
            }
            constexpr void remove_piece(Square s){
                Piece pc = board_[s];
                byType_[type_of(pc)].reset(s);
                byColor_[color_of(pc)].reset(s);
                board_[s] = NO_PIECE;
            }

            /* 持ち駒を実際に書き換える操作 */
            constexpr void add_hand(Color c, PieceType pt){ // 過剰に(3枚以上)増やさないのは呼び出し側で保証すること
                hand_[c] += (1 << HAND_SHIFT[pt]);
            }    
            constexpr void remove_hand(Color c, PieceType pt){ // 持っていない駒を減らさないのも呼び出し側で保証すること
                hand_[c] -= (1 << HAND_SHIFT[pt]);
            }
        
        public:
            /* コンストラクタ */
            constexpr Position() : ply_(1), sideToMove_(BLACK) {}

            /* 各種メソッド */
            // 名前付きコンストラクタ
            static Position startpos();
            static std::optional<Position> from_sfen(std::string_view);

            std::string sfen() const;
            std::string to_ascii() const;

            constexpr Color side_to_move() const { return sideToMove_; }
            constexpr int ply() const {return ply_; }
            constexpr Piece piece_on(Square s) const { return board_[s]; }    // 呼び出し側で s の範囲を検査すること
            constexpr int hand_count(Color c, PieceType pt) const { return (hand_[c] >> HAND_SHIFT[pt]) & HAND_MASK; }

            constexpr Bitboard pieces() const { return byColor_[BLACK] | byColor_[WHITE]; }
            constexpr Bitboard pieces(Color c) const { return byColor_[c]; }
            constexpr Bitboard pieces(PieceType pt) const { return byType_[pt]; }
            constexpr Bitboard pieces(Color c, PieceType pt) const { return byColor_[c] & byType_[pt]; }

            constexpr Square lion_square(Color c) const { return pieces(c, LION).lsb(); }   // 盤面に LION がいることを呼び出し側で保証すること

            // pos の整合性をチェック
            // 種類ごとの駒数、ライオンの色など。すべてではない。
            bool is_consistent() const;
    };
}

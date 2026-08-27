#pragma once
#include <cstdint>
#include <string>
#include <string_view>

namespace dobutsu {
    enum Color : std::uint8_t{
        BLACK = 0,
        WHITE = 1,
        COLOR_NB = 2
    };

    // 色を反転する演算子
    constexpr Color operator~(Color c){
        return Color(c ^ 1);
    }

    // 駒種
    enum PieceType : std::uint8_t{
        NO_PIECE_TYPE = 0,  // 空マス
        LION = 1,
        GIRAFFE = 2,
        ELEPHANT = 3,
        CHICK = 4,
        HEN = 5,
        PIECE_TYPE_NB = 6
    };

    // 手番+駒
    enum Piece : std::uint8_t{
        NO_PIECE = 0,
        B_LION = 1, B_GIRAFFE = 2, B_ELEPHANT = 3, B_CHICK = 4, B_HEN = 5,
        W_LION = 9, W_GIRAFFE = 10, W_ELEPHANT = 11, W_CHICK = 12, W_HEN = 13,
        PIECE_NB = 16
    };

    // 座標系統
    enum File : std::uint8_t{
        FILE_3 = 0, FILE_2 = 1, FILE_1 = 2, 
        FILE_NB = 3
    };
    enum Rank : std::uint8_t{
        RANK_A = 0,
        RANK_B = 1,
        RANK_C = 2,
        RANK_D = 3,
        RANK_NB = 4
    };
    enum Square : std::uint8_t{
        SQ_3A = 0, SQ_2A = 1, SQ_1A = 2,
        SQ_3B = 3, SQ_2B = 4, SQ_1B = 5,
        SQ_3C = 6, SQ_2C = 7, SQ_1C = 8,
        SQ_3D = 9, SQ_2D = 10, SQ_1D = 11,
        SQ_NB = 12,
        SQ_NONE = 15
    };

    // 打ち手
    enum class Move : std::uint16_t{ NONE = 0 };

    constexpr std::uint8_t COLOR_SHIFT = 3;       // 手番のシフト量
    constexpr std::uint8_t PT_MASK = 0b111;       // 駒種のマスク
    constexpr std::uint8_t SQ_PER_RANK = FILE_NB;

    constexpr std::uint16_t MOVES_DESTINATION_SHIFT = 0;    // 行先のマス
    constexpr std::uint16_t MOVES_SOURCE_SHIFT = 4;         // 元のマス
    constexpr std::uint16_t MOVES_PROMOTION_SHIFT = 8;      // 成りフラグ
    constexpr std::uint16_t MOVES_PIECETYPE_SHIFT = 9;      // 駒種
    constexpr std::uint16_t MOVES_SQUARE_MASK = 0b1111;     // マス抽出マスク
    constexpr std::uint16_t MOVES_PIECETYPE_MASK = 0b111;   // 駒種抽出マスク
    constexpr std::uint16_t MOVES_FLAG_MASK = 0b1;          // フラグ抽出マスク


    /* 変換関数 */
    constexpr Piece make_piece(Color c, PieceType pt){
        return Piece((c << COLOR_SHIFT) | pt);
    }
    
    constexpr PieceType type_of(Piece pc){
        return PieceType(PT_MASK & pc);
    }
    
    constexpr Color color_of(Piece pc){
        return Color(pc >> COLOR_SHIFT);
    }

    constexpr Square make_square(File f, Rank r){
        return Square(f + SQ_PER_RANK  * r);
    }

    constexpr File file_of(Square s){
        return File(s % SQ_PER_RANK );
    }

    constexpr Rank rank_of(Square s){
        return Rank(s / SQ_PER_RANK );
    }

    constexpr bool is_ok(Square s){
        return (s < SQ_NB);
    }

    /* 文字列変換 */
    std::string square_to_usi(Square s);
    Square square_from_usi(std::string_view usi);

    /* MOVE 関連 */
    constexpr Move make_move(Square from, Square to, bool promote = false){
        return Move((std::uint16_t(to) << MOVES_DESTINATION_SHIFT) | (std::uint16_t(from) << MOVES_SOURCE_SHIFT) | (std::uint16_t(promote) << MOVES_PROMOTION_SHIFT));
    }

    constexpr Move make_drop(PieceType pt, Square to){
        return Move((std::uint16_t(pt) << MOVES_PIECETYPE_SHIFT) | (std::uint16_t(SQ_NONE) << MOVES_SOURCE_SHIFT) | (std::uint16_t(to) << MOVES_DESTINATION_SHIFT)); // LION や HEN 等は呼び出し側で渡さないことを保証
    }

    constexpr Square to_sq(Move m){
        std::uint16_t move = std::uint16_t(m);
        return Square((move >> MOVES_DESTINATION_SHIFT) & MOVES_SQUARE_MASK);
    }

    constexpr Square from_sq(Move m){
        std::uint16_t move = std::uint16_t(m);
        return Square((move >> MOVES_SOURCE_SHIFT) & MOVES_SQUARE_MASK);
    }

    constexpr PieceType dropped_piece(Move m){
        std::uint16_t move = std::uint16_t(m);
        return PieceType((move >> MOVES_PIECETYPE_SHIFT) & MOVES_PIECETYPE_MASK);
    }

    constexpr bool is_drop(Move m){
        return (from_sq(m) == SQ_NONE);
    }

    constexpr bool is_promotion(Move m){
        std::uint16_t move = std::uint16_t(m);
        return ((move >> MOVES_PROMOTION_SHIFT) & MOVES_FLAG_MASK) > 0;
    }

    std::string move_to_usi(Move m);
    Move move_from_usi(std::string_view usi);
}

#include <dobutsu/types.hpp>
#include <dobutsu/evaluate.hpp>
#include <dobutsu/position.hpp>


namespace {
    constexpr int PIECE_VALUE[dobutsu::PIECE_TYPE_NB] = { 0, 0, 400, 300, 100, 500 };
    constexpr int HAND_VALUE[dobutsu::PIECE_TYPE_NB] = {0, 0, 400, 300, 100, 0};
}
namespace dobutsu {
    int evaluate(const Position& pos){
        int score = 0;
        for(int pt = 0; pt < PIECE_TYPE_NB; pt++){
            score += PIECE_VALUE[pt] * (pos.pieces(BLACK, PieceType(pt)).popcount() - pos.pieces(WHITE, PieceType(pt)).popcount());
        }
        for(PieceType pt : {GIRAFFE, ELEPHANT, CHICK}){
            score += HAND_VALUE[pt] * (pos.hand_count(BLACK, pt) - pos.hand_count(WHITE, pt));
        }
        return (pos.side_to_move() == BLACK ? score : -score);
    }
}


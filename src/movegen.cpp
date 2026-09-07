#include <dobutsu/types.hpp>
#include <dobutsu/bitboard.hpp>
#include <dobutsu/position.hpp>
#include <dobutsu/movegen.hpp>

namespace dobutsu {
    MoveList::MoveList(const Position& pos){
        const Color us = pos.side_to_move();
        // 移動の場合
        Bitboard bb_from = pos.pieces(us);
        while(bb_from){
            Square from = bb_from.pop_lsb();
            Piece pc = pos.piece_on(from);
            Bitboard bb_to = attacks_from(pc, from) & ~pos.pieces(us);
            while(bb_to){
                Square to = bb_to.pop_lsb();
                bool promote = type_of(pc) == CHICK && rank_of(to) == (us == BLACK ? RANK_A : RANK_D);
                add(make_move(from, to, promote));
                
            }
        }
        // 打ち手の場合
        for(PieceType pt: {GIRAFFE, ELEPHANT, CHICK}){
            if(pos.hand_count(us, pt) == 0) continue;
            Bitboard bb_to = ~pos.pieces();
            while(bb_to){
                Square to = bb_to.pop_lsb();
                add(make_drop(pt, to));
            }
        }
    }
}

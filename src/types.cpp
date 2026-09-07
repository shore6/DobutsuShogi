#include <dobutsu/types.hpp>
#include <string>
#include <string_view>

namespace {
    constexpr char DROP_PIECE[dobutsu::PIECE_TYPE_NB] = {
        ' ', ' ', 'G', 'E', 'C', ' '
    };
}

namespace dobutsu {
    
    std::string square_to_usi(Square s){
        if(!is_ok(s)) return "";
        const char c1 = char('3' - file_of(s));
        const char c2 = char('a' + rank_of(s));
        return {c1, c2};
    }

    Square square_from_usi(std::string_view usi){
        if(usi.size() != 2 || usi[0] < '1' || usi[0] > '3' || usi[1] < 'a' || usi[1] > 'd') return SQ_NONE;
        return make_square(File('3' - usi[0]), Rank(usi[1] - 'a'));
    }

    std::string move_to_usi(Move m){
        if(m == Move::NONE) return "";
        std::string out;
        if(is_drop(m)){
            // 打ち手
            out += DROP_PIECE[dropped_piece(m)];
            out += '*';
            out += square_to_usi(to_sq(m));
        }else{
            // 移動
            out += square_to_usi(from_sq(m));
            out += square_to_usi(to_sq(m));
            if(is_promotion(m)){
                out += '+';
            }
        }
        return out;
    }

    Move move_from_usi(std::string_view usi){
        if(usi.size() != 4 && usi.size() != 5) return Move::NONE;
        const Square from = square_from_usi(usi.substr(0, 2));
        const Square to = square_from_usi(usi.substr(2, 2));
        if(from == to) return Move::NONE;
        if(usi.size() == 4){
            if(from != SQ_NONE && to != SQ_NONE){
                return make_move(from, to, false);
            }else if((usi[0] == DROP_PIECE[GIRAFFE] || usi[0] == DROP_PIECE[ELEPHANT] || usi[0] == DROP_PIECE[CHICK]) && usi[1] == '*' && to != SQ_NONE){
                PieceType pt = usi[0] == DROP_PIECE[GIRAFFE] ? GIRAFFE : (usi[0] == DROP_PIECE[ELEPHANT] ? ELEPHANT : CHICK);
                return make_drop(pt, to);
            }
        }else if(from != SQ_NONE && to != SQ_NONE && usi[4] == '+'){
            return make_move(from, to, true);
        }
        return Move::NONE;
    }
}

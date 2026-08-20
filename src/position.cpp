#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <charconv>
#include <array>
#include <utility>
#include <limits>
#include <dobutsu/types.hpp>
#include <dobutsu/bitboard.hpp>
#include <dobutsu/position.hpp>

namespace {
    constexpr char PIECE_CHAR[dobutsu::PIECE_NB] = {
    ' ', 'L', 'G', 'E', 'C', 'H', ' ', ' ',
    ' ', 'l', 'g', 'e', 'c', 'h', ' ', ' ',
    };

    constexpr std::pair<std::array<std::string_view, 4>, std::size_t> split(std::string_view s) noexcept {
        std::array<std::string_view, 4> out{};
        const std::string_view delims = " ";
        std::size_t idx = 0;
        for(std::size_t pos = 0; pos < s.size(); ){
            pos = s.find_first_not_of(delims, pos);
            if(pos == std::string_view::npos) break;
            const std::size_t end = s.find_first_of(delims, pos);
            if(idx < out.size()) out[idx] = s.substr(pos, end - pos);
            idx++;
            if(end == std::string_view::npos) break;
            pos = end;
        }
        return {out, idx};
    }
    
    dobutsu::Piece piece_from_char(char c){
        for(int i = 0; i < dobutsu::PIECE_NB; i++){
            if(c == PIECE_CHAR[dobutsu::Piece(i)]){
                return dobutsu::Piece(i);
            }
        }
        return dobutsu::NO_PIECE;
    }

    char piece_to_char(dobutsu::Piece pc){
        return PIECE_CHAR[pc];
    }
}

namespace dobutsu {

        // 初期局面の生成
        Position Position::startpos(){
            Position pos;
            pos.put_piece(B_LION, SQ_2D);
            pos.put_piece(B_GIRAFFE, SQ_1D);
            pos.put_piece(B_ELEPHANT, SQ_3D);
            pos.put_piece(B_CHICK, SQ_2C);
            pos.put_piece(W_LION, SQ_2A);
            pos.put_piece(W_GIRAFFE, SQ_3A);
            pos.put_piece(W_ELEPHANT, SQ_1A);
            pos.put_piece(W_CHICK, SQ_2B);
             
            return pos;
        }


        // 盤面を図で表示
        std::string Position::to_ascii() const {
            std::string out;
            const std::string line = "+---+---+---+\n";
            out += "  3   2   1\n";
            out += line;
            for(int rank = 0; rank < RANK_NB; rank++){
                out += "| ";
                out += piece_to_char(board_[make_square(FILE_3, Rank(rank))]);
                out += " | ";
                out += piece_to_char(board_[make_square(FILE_2, Rank(rank))]);
                out += " | ";
                out += piece_to_char(board_[make_square(FILE_1, Rank(rank))]);
                out += " | ";
                out += char('a' + rank);
                out += '\n';
                out += line;
            }

            // 持ち駒
            out += "hand:";
            int cnt = 0;
            for(Color c : {BLACK, WHITE}){
                for(PieceType pt : {GIRAFFE, ELEPHANT, CHICK}){
                    if(hand_count(c, pt) != 0){
                        out += ' ';
                        out += piece_to_char(make_piece(c, pt));
                        out += '*';
                        out += std::to_string(hand_count(c, pt));
                        cnt++;
                    }
                }
            }
            if(cnt == 0) out += " -";
            out += '\n';
            out += "turn: ";
            out += side_to_move() == BLACK ? 'b' : 'w';
            out += '\n';
            out += "ply: ";
            out += std::to_string(ply());
            out += '\n';
            return out;
        }

        std::string Position::sfen() const {
            std::string out = "";
            // 盤面
            for(int r = RANK_A; r < RANK_NB; r++){
                int empty = 0;
                auto flush = [&]{ if(empty){ out += char('0' + empty); empty = 0;}};
                for(int f = FILE_3; f < FILE_NB; f++){
                    Piece pc = board_[make_square(File(f), Rank(r))];
                    if(pc == NO_PIECE){
                        empty++;
                    }else{
                        flush();
                        if(type_of(pc) == HEN){
                            out += '+';
                            out += piece_to_char(make_piece(color_of(pc), CHICK));
                            continue;
                        }
                        out += piece_to_char(board_[make_square(File(f), Rank(r))]);
                    }
                }
                flush();
                if(r != RANK_D){
                    out += '/';
                }
            }
            out += ' ';

            // 手番
            out += side_to_move() == BLACK ? 'b' : 'w';
            out += ' ';

            // 持ち駒
            int cnt = 0;
            for(Color c : {BLACK, WHITE}){
                for(PieceType pt : {GIRAFFE, ELEPHANT, CHICK}){
                    if(hand_count(c, pt) != 0){
                        out += hand_count(c, pt) == 1 ? "" : std::to_string(hand_count(c, pt));
                        out += piece_to_char(make_piece(c, pt));
                        cnt++;
                    }
                }
            }
            if(cnt == 0) out += '-';
            out += ' ';

            // 手数
            out += std::to_string(ply());
            
            return out;
        }

        // sfen からposを生成、できなければ std::nullopt
        std::optional<Position> Position::from_sfen(std::string_view sfen){
            auto [fields, cnt] = split(sfen);

            Position pos;

            if(cnt != 4) return std::nullopt;   // フィールド数の確認

            // 盤面を解析
            int sq_idx = 0, rank_idx = 0;
            for(std::size_t char_idx = 0; char_idx < fields[0].size(); char_idx++){
                char c = fields[0][char_idx];
                if(sq_idx >= SQ_NB) return std::nullopt;     // 盤面のマス数を確認
                if(c == '/'){
                    if(sq_idx == 3 * (rank_idx + 1)){
                        rank_idx++;
                        continue;
                    }
                    return std::nullopt;
                }
                if(c >= '1' && c <= '3'){
                    sq_idx += int(c - '0');
                    continue;
                }
                if(c == '+'){
                    if(char_idx + 1 >= fields[0].size()) return std::nullopt;
                    char nxt = fields[0][char_idx+1];
                    Piece base = piece_from_char(nxt);
                    if(type_of(base) != CHICK) return std::nullopt;
                    pos.put_piece(make_piece(color_of(base), HEN), Square(sq_idx));
                    char_idx++; sq_idx++;
                    continue;
                }
                Piece p = piece_from_char(c);   // 空白が区切り文字なのでよけいな空白は消えてる
                if(p == NO_PIECE) return std::nullopt;
                pos.put_piece(p, Square(sq_idx));
                sq_idx++;
            }
            if(rank_idx != 3 || sq_idx != SQ_NB) return std::nullopt;    // 盤面のマス数合計を確認
            
            
            // 手番を取得
            if(fields[1] == "b" || fields[1] == "w"){
                pos.sideToMove_ = (fields[1] == "b" ? BLACK : WHITE);
            }else return std::nullopt;

            // 持ち駒の解析
            if(fields[2] != "-"){
                for(std::size_t char_idx = 0; char_idx < fields[2].size(); char_idx++){
                    char c = fields[2][char_idx];
                    if(c == '2'){
                        if(char_idx + 1 >= fields[2].size()) return std::nullopt;
                        char nxt = fields[2][char_idx+1];
                        Piece p = piece_from_char(nxt);
                        PieceType pt = type_of(p);
                        if(pt == GIRAFFE || pt == ELEPHANT || pt == CHICK){
                            if(pos.hand_count(color_of(p), pt) + 2 > 2) return std::nullopt;
                            pos.add_hand(color_of(p), pt);
                            pos.add_hand(color_of(p), pt);
                            char_idx++;
                            continue;
                        }
                        return std::nullopt;
                    }
                    Piece p = piece_from_char(c);
                    PieceType pt = type_of(p);
                    if(pt == GIRAFFE || pt == ELEPHANT || pt == CHICK){
                        if(pos.hand_count(color_of(p), pt) + 1 > 2) return std::nullopt;
                        pos.add_hand(color_of(p), type_of(p));
                        continue;
                    }
                    return std::nullopt;
                }
            }

            // 手数の解析
            int n = 0;
            auto [ptr, ec] = std::from_chars(fields[3].data(), fields[3].data() + fields[3].size(), n);
            if(ec != std::errc{} || ptr != fields[3].data() + fields[3].size()) return std::nullopt;
            if(n <= 0 || n > std::numeric_limits<std::uint16_t>::max()) return std::nullopt;
            pos.ply_ = n;

            // 駒種ごとの総数およびライオンの色を確認
            if(!pos.is_consistent()) return std::nullopt;
            return pos; // すべてパス
        }

        bool Position::is_consistent() const {
            // board_ の操作
            Bitboard bb_color[COLOR_NB]{};
            Bitboard bb_type[PIECE_TYPE_NB]{};
            int pt_count[PIECE_TYPE_NB]{};
            int lion_count[COLOR_NB]{};
            for(int sq = 0; sq < SQ_NB; sq++){
                Piece p = board_[Square(sq)];
                if(p != NO_PIECE){
                    bb_color[color_of(p)].set(Square(sq));
                    bb_type[type_of(p)].set(Square(sq));
                    pt_count[type_of(p)]++;
                    if(type_of(p) == LION) lion_count[color_of(p)]++;
                    if(type_of(p) == HEN) pt_count[CHICK]++;
                }
            }
            // 持ち駒
            for(Color c : {BLACK, WHITE}){
                for(PieceType pt : {GIRAFFE, ELEPHANT, CHICK}){
                    pt_count[pt] += hand_count(c, pt);
                }
            }

            // チェック
            for(Color c : {BLACK, WHITE}){
                if(bb_color[c] != byColor_[c]) return false;
            }
            for(PieceType pt : {LION, GIRAFFE, ELEPHANT, CHICK}){
                if(pt_count[pt] != 2) return false;
            }
            for(int pt = NO_PIECE_TYPE; pt < PIECE_TYPE_NB; pt++){
                if(bb_type[PieceType(pt)] != byType_[PieceType(pt)]) return false;
            }
            if(lion_count[BLACK] != 1 || lion_count[WHITE] != 1) return false;

            return true;
        }

}


#include <algorithm>
#include <iostream>
#include <gtest/gtest.h>
#include <dobutsu/types.hpp>
#include <dobutsu/position.hpp>
#include <dobutsu/movegen.hpp>
#include <string>
#include <vector>

using namespace dobutsu;

std::uint64_t perft(Position& pos, int depth){
    if(pos.is_terminal()) return 1;
    if(depth == 0) return 1;
    MoveList ml(pos);
    std::uint64_t cnt = 0;
    for(Move m : ml){
        StateInfo st;
        pos.do_move(m, st);
        cnt += perft(pos, depth - 1);
        pos.undo_move(m);
    }
    return cnt;
}

bool in_board(Square from, int dr, int df){
    if((rank_of(from) == RANK_A && dr == -1) || (rank_of(from) == RANK_D && dr == 1)) return false;
    if((file_of(from) == FILE_3 && df == -1) || (file_of(from) == FILE_1 && df == 1)) return false;
    return true;
}
bool to_ok(Position pos, Square to, Color us){
    if(pos.piece_on(to) == NO_PIECE) return true;
    if(color_of(pos.piece_on(to)) != us) return true;
    return false;
}

std::vector<Move> movelist_custom(const Position& pos){
    std::vector<Move> move_list;
    const Color us = pos.side_to_move();
    // 移動
    for(int sq = 0; sq < SQ_NB; sq++){
        Square from = Square(sq);
        Piece pc = pos.piece_on(from);
        PieceType pt = type_of(pc);
        if(pt == NO_PIECE_TYPE) continue;
        Color c = color_of(pc);
        if(c != us) continue;
        
        switch(pt){
            case LION:{
                for(int dr : {-1, 0, 1}){
                    for(int df : {-1, 0, 1}){
                        if(dr == 0 && df == 0) continue;
                        if(!in_board(from, dr, df)) continue;
                        Square to = make_square(File(int(file_of(from)) + df), Rank(int(rank_of(from)) + dr));
                        if(!to_ok(pos, to, us)) continue;
                        move_list.push_back(make_move(from, to));
                    }
                }
                break;
            }

            case GIRAFFE:{
                for(int dr : {-1, 0, 1}){
                    for(int df : {-1, 0, 1}){
                        if(df * dr != 0) continue;
                        if(!in_board(from, dr, df)) continue;
                        Square to = make_square(File(int(file_of(from)) + df), Rank(int(rank_of(from)) + dr));
                        if(!to_ok(pos, to, us)) continue;
                        move_list.push_back(make_move(from, to));
                    }
                }
                break;
            }

            case ELEPHANT:{
                for(int dr : {-1, 0, 1}){
                    for(int df : {-1, 0, 1}){
                        if(df * dr == 0) continue;
                        if(!in_board(from, dr, df)) continue;
                        Square to = make_square(File(int(file_of(from)) + df), Rank(int(rank_of(from)) + dr));
                        if(!to_ok(pos, to, us)) continue;
                        move_list.push_back(make_move(from, to));
                    }
                }
                break;
            }

            case CHICK:{
                int dr = (us == BLACK ? -1 : 1);
                if(!in_board(from, dr, 0)) continue;
                Square to = make_square(file_of(from), Rank(int(rank_of(from)) + dr));
                if(!to_ok(pos, to, us)) continue;
                bool promote = (rank_of(to) == (us == BLACK ? RANK_A : RANK_D));
                move_list.push_back(make_move(from, to, promote));
                break;
            }
            case HEN:{
                for(int dr : {-1, 0, 1}){
                    for(int df : {-1, 0, 1}){
                        if(df != 0 && dr == (us == BLACK ? 1 : -1)) continue;
                        if(!in_board(from, dr, df)) continue;
                        Square to = make_square(File(int(file_of(from)) + df), Rank(int(rank_of(from)) + dr));
                        if(!to_ok(pos, to, us)) continue;
                        move_list.push_back(make_move(from, to));
                    }
                }
                break;
            }
        }
    }
    
    // 打ち駒
    for(PieceType pt : {GIRAFFE, ELEPHANT, CHICK}){
        if(pos.hand_count(us, pt) == 0) continue;
        for(int sq = 0; sq < SQ_NB; sq++){
            Square to = Square(sq);
            if(pos.piece_on(to) != NO_PIECE) continue;
            move_list.push_back(make_drop(pt, to));
        }
    }
    
    return move_list;
}
bool in_check(const Position pos){
    bool can_catch = false;
    std::vector<Move> ml = movelist_custom(pos);
    for(Move m : ml){
        if(to_sq(m) == pos.lion_square(~pos.side_to_move())){
            can_catch = true; break;
        }
    }
    bool is_tried = rank_of(pos.lion_square(~pos.side_to_move())) == (pos.side_to_move() == BLACK ? RANK_D : RANK_A);
    return can_catch || is_tried;
}


std::uint64_t perft_custom(Position& pos, int depth){
    if(in_check(pos)) return 1;
    if(depth == 0) return 1;
    std::vector<Move> ml = movelist_custom(pos);
    std::uint64_t cnt = 0;
    for(Move m : ml){
        StateInfo st;
        pos.do_move(m, st);
        cnt += perft_custom(pos, depth -1);
        pos.undo_move(m);
    }
    return cnt;
}


// int main(){
//     Position pos = Position::startpos();
//     auto move_list = movelist_custom(pos);
//     std::cout << move_list.size() << std::endl;
//     for(Move m : move_list){
//         std::cout << move_to_usi(m) << std::endl;
//     }
//     std::cout << "------" << std::endl;
//     MoveList ml(pos);
//     for(Move m : ml){
//         std::cout << move_to_usi(m) << std::endl;
//     }
//     std::cout << "======" << std::endl;
//
//     int depth = 3;
//     Position posA = Position::startpos();
//     Position posB = Position::startpos();
//     std::uint64_t custom = perft_custom(posA, depth);
//     std::uint64_t origin = perft(posB, depth);
//     std::cout << custom << ":" << origin << std::endl;
//     return 0;
// }


TEST(Perft, CheckConsistencyOfMoveListBetweenCustomLogicAtStartpos){
    Position pos = Position::startpos();
    auto ml_custom = movelist_custom(pos);
    MoveList ml(pos);
    std::vector<std::string> usi_custom;
    std::vector<std::string> usi;
    for(Move m : ml_custom){
        usi_custom.push_back(move_to_usi(m));
    }
    for(Move m : ml){
        usi.push_back(move_to_usi(m));
    }
    std::sort(usi_custom.begin(), usi_custom.end());
    std::sort(usi.begin(), usi.end());
    EXPECT_EQ(usi, usi_custom);
}


TEST(Perft, CheckConsistencyOfMoveListBetweenCustomLogicAtSampleSFEN){
    const std::string sfen_list[] = {
        "l2/1CC/3/ELG b eg 3",
        "l2/1cc/3/ELG w EG 10",
        "l2/3/1CC/ELG b eg 13",
        "l2/3/1cc/LEG w eg 22",
        "gle/1c1/1C1/ELG b - 1",
        "+Cle/3/3/EL+c b 2G 7",
        "l2/1C1/3/2L b 2G2ec 5",
        "gle/1c1/1C1/ELG w - 101",
        "gle/3/3/ELG w 2c 170",
        "gle/1c1/1+C1/ELG b - 1",
        "gl1/1c1/1C1/1LG b Ee 1",
        "+Cle/3/3/EL+c w 2G 7",
        "l2/3/3/2L b 2GC2ec 5"
    };
    for(const std::string& sfen : sfen_list){
        auto pos = Position::from_sfen(sfen);
        auto ml_custom = movelist_custom(pos.value());
        MoveList ml(pos.value());
        std::vector<std::string> usi_custom;
        std::vector<std::string> usi;
        for(Move m : ml_custom){
            usi_custom.push_back(move_to_usi(m));
        }
        for(Move m : ml){
            usi.push_back(move_to_usi(m));
        }
        std::sort(usi_custom.begin(), usi_custom.end());
        std::sort(usi.begin(), usi.end());
        EXPECT_EQ(usi, usi_custom) << sfen;
    }
}

TEST(Perft, CheckConsistencyOfPerftCounts){
    for(int depth = 1; depth < 9; depth++){
        Position posA = Position::startpos();
        Position posB = Position::startpos();
        std::uint64_t perft_num = perft(posA, depth);
        std::uint64_t perft_num_custom = perft_custom(posB, depth);
        EXPECT_EQ(perft_num, perft_num_custom) << depth;
    }
}

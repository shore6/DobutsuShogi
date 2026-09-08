#include <algorithm>
#include <iostream>
#include <gtest/gtest.h>
#include <dobutsu/types.hpp>
#include <dobutsu/position.hpp>
#include <dobutsu/movegen.hpp>
#include <string>
#include <vector>

using namespace dobutsu;

bool in_board(Square from, int dr, int df){
    if((rank_of(from) == RANK_A && dr == -1) || (rank_of(from) == RANK_D && dr == 1)) return false;
    if((file_of(from) == FILE_3 && df == -1) || (file_of(from) == FILE_1 && df == 1)) return false;
    return true;
}
bool to_ok(const Position& pos, Square to, Color us){
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
            case NO_PIECE_TYPE:
            case PIECE_TYPE_NB: break;
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
bool is_terminal_custom(const Position& pos){
    bool can_catch = false;
    std::vector<Move> ml = movelist_custom(pos);
    Square lion_sq;
    for(int sq = 0; sq < SQ_NB; sq++){
        Piece pc = pos.piece_on(Square(sq));
        if(pc == NO_PIECE) continue;
        if(type_of(pc) == LION && color_of(pc) == ~pos.side_to_move()){
            lion_sq = Square(sq); break;
        }
    }
    for(Move m : ml){
        if(to_sq(m) == lion_sq){
            can_catch = true; break;
        }
    }
    bool is_tried = rank_of(pos.lion_square(~pos.side_to_move())) == (pos.side_to_move() == BLACK ? RANK_D : RANK_A);
    return can_catch || is_tried;
}


std::uint64_t perft_custom(Position& pos, int depth){
    if(is_terminal_custom(pos)) return 1;
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
    for(int depth = 1; depth < 7; depth++){
        Position posA = Position::startpos();
        Position posB = Position::startpos();
        std::uint64_t perft_num = perft(posA, depth);
        std::uint64_t perft_num_custom = perft_custom(posB, depth);
        EXPECT_EQ(perft_num, perft_num_custom) << depth;
    }
}

TEST(Perft, CompareBaselineAndPerft){
    Position pos = Position::startpos();
    EXPECT_EQ(perft(pos, 1), 4ULL);
    EXPECT_EQ(perft(pos, 2), 17ULL);
    EXPECT_EQ(perft(pos, 3), 100ULL);
    EXPECT_EQ(perft(pos, 4), 610ULL);
    EXPECT_EQ(perft(pos, 5), 3411ULL);
    EXPECT_EQ(perft(pos, 6), 19988ULL);
    EXPECT_EQ(perft(pos, 7), 122546ULL);
}

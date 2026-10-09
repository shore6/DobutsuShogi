#include <gtest/gtest.h>
#include <dobutsu/types.hpp>
#include <dobutsu/position.hpp>
#include <dobutsu/zobrist.hpp>
#include <dobutsu/movegen.hpp>
#include <set>
#include <array>
#include <algorithm>
#include <vector>
#include <string>

using namespace dobutsu;

static_assert(Zobrist.psq[B_LION][SQ_2D] == 0x935e82f1db4c4f7bULL); // state初期値(seed) = 0 

namespace {
    std::vector<Key> to_vec(){
        std::vector<Key> all;
        for(const auto& row : Zobrist.psq){
            for(Key k : row) if(k != 0) all.push_back(k);
        }
        for(const auto& mat : Zobrist.hand){
            for(const auto & row : mat){
                for(Key k : row) if(k != 0) all.push_back(k);
            }
        }
        all.push_back(Zobrist.side);

        return all;
    }

    std::uint64_t perft_collect(Position& pos, int depth, std::vector<std::pair<std::string, Key>>& vec){
        if(pos.is_terminal()) return 1;
        if(depth == 0) return 1;
        MoveList ml(pos);
        std::uint64_t cnt = 0;
        for(Move m : ml){
            StateInfo st;
            pos.do_move(m, st);
            std::string s = pos.sfen(); std::string sfen = s.substr(0, s.rfind(' '));
            vec.push_back({sfen, pos.key()});
            cnt += perft_collect(pos, depth - 1, vec);
            pos.undo_move(m);
        }
        return cnt;
    }
}


TEST(zobrist, CheckAllNumsAreUnique){
    int cnt = 0;
    std::set<Key> zobrist_num_sets;

    for(Color c : {BLACK, WHITE}){
        for(int pt = 0; pt < PIECE_TYPE_NB; pt++){
            for(int s = 0; s < SQ_NB; s++){
                Key k = Zobrist.psq[make_piece(c, PieceType(pt))][s];
                if(k == 0) continue;
                zobrist_num_sets.insert(k); cnt++;
            }
            for(int num = 0; num < 3; num++){
                Key k = Zobrist.hand[c][pt][num];
                if(k == 0) continue;
                zobrist_num_sets.insert(k); cnt++;
            }
        }
    }
    zobrist_num_sets.insert(Zobrist.side); cnt++;

    EXPECT_EQ(zobrist_num_sets.size(), size_t(cnt));
    EXPECT_EQ(zobrist_num_sets.size(), 133u);
}

TEST(zobrist, WhetherEachNumIsZeroOrNot){
    std::array<Piece, 10> pcs = {
        B_LION, B_GIRAFFE, B_ELEPHANT, B_CHICK, B_HEN, 
        W_LION, W_GIRAFFE, W_ELEPHANT, W_CHICK, W_HEN
    };
    std::array<PieceType, 3> pts = {
        GIRAFFE, ELEPHANT, CHICK
    };

    for(int pc = 0; pc < PIECE_NB; pc++){
        auto it = std::find(pcs.begin(), pcs.end(), Piece(pc));
        if(it != pcs.end()){
            for(int s = 0; s < SQ_NB; s++) EXPECT_NE(Zobrist.psq[pc][s], Key{0});
        }else{
            for(int s = 0; s < SQ_NB; s++) EXPECT_EQ(Zobrist.psq[pc][s], Key{0});
        }
    }

    for(int pt = 0; pt < PIECE_TYPE_NB; pt++){
        for(int n = 0; n < 3; n++){
            auto it = std::find(pts.begin(), pts.end(), PieceType(pt));
            if(it == pts.end() || n == 0){
                EXPECT_EQ(Zobrist.hand[BLACK][pt][n], Key{0});
                EXPECT_EQ(Zobrist.hand[WHITE][pt][n], Key{0});
            }else{
                EXPECT_NE(Zobrist.hand[BLACK][pt][n], Key{0});
                EXPECT_NE(Zobrist.hand[WHITE][pt][n], Key{0});
            }
        }
    }

    EXPECT_NE(Zobrist.side, Key{0});
}

TEST(zobrist, NoTwoXORsAreEqual){
    
    std::vector<Key> all = to_vec();

    ASSERT_EQ(all.size(), 133u);
    for(int idx1 = 0; idx1 < all.size(); idx1++){
        for(int idx2 = idx1 + 1; idx2 < all.size(); idx2++){
            Key k1 = all[idx1], k2 = all[idx2];
            for(int idx3 = 0; idx3 < all.size(); idx3++){
                Key xors = k1 ^ k2;
                EXPECT_NE(all[idx3], xors);
            }
        }
    }
}

/* 以下は Key への組み込みテスト */

TEST(zobrist, CompareStartPosKeyWithComputedKey){
    Position pos = Position::startpos();
    EXPECT_EQ(pos.key(), pos.compute_key());
}


TEST(zobrist, CompareKeyOfPosFromSampleSFEN){
    const std::string sfen_list[] = {
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
        EXPECT_EQ(pos->key(), pos->compute_key()) << sfen;
    }
}

TEST(zobrist, CheckKeyConsistencyBetweenDoAndUndoMoves){
    const std::string sfen_list[] = {
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
        for(Move m : MoveList(pos.value())){
            StateInfo st;
            Key before = pos->key();
            pos->do_move(m, st); pos->undo_move(m);
            EXPECT_EQ(before, pos->key());
        }
    }
}

TEST(zobrist, CheckKeyConsistencyAfterRoundTrip){
    auto pos1 = Position::startpos(); // gle/1c1/1C1/ELG b - 1
    auto pos2 = Position::startpos();

    StateInfo st1[10], st2[10];
    pos1.do_move(make_move(SQ_2C, SQ_2B), st1[0]);
    pos1.do_move(make_move(SQ_2A, SQ_2B), st1[1]);


    pos2.do_move(make_move(SQ_2C, SQ_2B), st2[0]);
    pos2.do_move(make_move(SQ_2A, SQ_2B), st2[1]);
    pos2.do_move(make_move(SQ_3D, SQ_2C), st2[2]);
    pos2.do_move(make_move(SQ_2B, SQ_2A), st2[3]);
    pos2.do_move(make_move(SQ_2C, SQ_3D), st2[4]);
    pos2.do_move(make_move(SQ_2A, SQ_2B), st2[5]);

    EXPECT_EQ(pos1.key(), pos2.key());

}

TEST(zobrist, DifferentKeyFromDifferentBoards){

    auto pos = Position::startpos();
    std::vector<std::pair<std::string, Key>> all_state;
    std::uint64_t n = perft_collect(pos, 6, all_state);
    EXPECT_EQ(n, 19988ULL);

    std::map<std::string, Key> str2key;
    std::map<Key, std::string> key2str;
    for(auto p : all_state){
        if(str2key.contains(p.first)){
            ASSERT_EQ(str2key.at(p.first), p.second) << p.first;
        }else{
            str2key.insert(p);
        }

        if(key2str.contains(p.second)){
            ASSERT_EQ(key2str.at(p.second), p.first) << p.first;
        }else{
            key2str.emplace(p.second, p.first);
        }
    }

}

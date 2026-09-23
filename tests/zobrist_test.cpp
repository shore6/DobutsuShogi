#include <gtest/gtest.h>
#include <dobutsu/types.hpp>
#include <dobutsu/zobrist.hpp>
#include <set>
#include <array>
#include <algorithm>
#include <vector>

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

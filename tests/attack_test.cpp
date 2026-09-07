#include <bit>
#include <cstdint>
#include <gtest/gtest.h>
#include <dobutsu/types.hpp>
#include <dobutsu/bitboard.hpp>
#include <dobutsu/attack.hpp>

using namespace dobutsu;

TEST(Attacks, CountBitsOfAllBitboardFromAttackTable){
    for(int pt = 0; pt < PIECE_TYPE_NB; pt++){
        for(int c = 0; c < COLOR_NB; c++){
            for(int s = 0; s < SQ_NB; s++){
                PieceType pp = PieceType(pt); Color cc = Color(c); Square ss = Square(s);
                Bitboard bb = attacks_from(pp, cc, ss);
                if(pp == LION){
                    if(ss == SQ_2B || ss == SQ_2C){
                        EXPECT_EQ(bb.popcount(), 8) << "LION : center"; 
                    }else if(ss == SQ_3A || ss == SQ_1A || ss == SQ_3D || ss == SQ_1D){
                        EXPECT_EQ(bb.popcount(), 3) << "LION : corner";
                    }else{
                        EXPECT_EQ(bb.popcount(), 5) << "LION";
                    }
                }else if(pp == GIRAFFE){
                    if(ss == SQ_2B || ss == SQ_2C){
                        EXPECT_EQ(bb.popcount(), 4) << "GIRAFFE : center"; 
                    }else if(ss == SQ_3A || ss == SQ_1A || ss == SQ_3D || ss == SQ_1D){
                        EXPECT_EQ(bb.popcount(), 2) << "GIRAFFE : corner";
                    }else{
                        EXPECT_EQ(bb.popcount(), 3) << "GIRAFFE";
                    }
                }else if(pp == ELEPHANT){
                    if(ss == SQ_2B || ss == SQ_2C){
                        EXPECT_EQ(bb.popcount(), 4) << "ELEPHANT : center"; 
                    }else if(ss == SQ_3A || ss == SQ_1A || ss == SQ_3D || ss == SQ_1D){
                        EXPECT_EQ(bb.popcount(), 1) << "ELEPHANT : corner";
                    }else{
                        EXPECT_EQ(bb.popcount(), 2) << "ELEPHANT";
                    }
                }else if(pp == CHICK){
                    if(cc == BLACK && rank_of(ss) == RANK_A){
                        EXPECT_EQ(bb.popcount(), 0) << "CHICK : BLACK : FRONT";
                    }else if(cc == BLACK){
                        EXPECT_EQ(bb.popcount(), 1) << "CHICK : BLACK";
                    }else if(cc == WHITE && rank_of(ss) == RANK_D){
                        EXPECT_EQ(bb.popcount(), 0) << "CHICK : WHITE : BACK";
                    }else if(cc == WHITE){
                        EXPECT_EQ(bb.popcount(), 1) << "CHICK : WHITE";
                    }
                }else if(pp == HEN){
                    if(cc == BLACK){
                        if(ss == SQ_2B || ss == SQ_2C){
                            EXPECT_EQ(bb.popcount(), 6) << "HEN : BLACK : center"; 
                        }else if(ss == SQ_3A || ss == SQ_1A){
                            EXPECT_EQ(bb.popcount(), 2) << "HEN : BLACK : A corner";
                        }else if(ss == SQ_2A){
                            EXPECT_EQ(bb.popcount(), 3) << "HEN : BLACK : A center";
                        }else if(ss == SQ_3D || ss == SQ_1D){
                            EXPECT_EQ(bb.popcount(), 3) << "HEN : BLACK : D corner";
                        }else if(ss == SQ_2D){
                            EXPECT_EQ(bb.popcount(), 5) << "HEN : BLACK : D center";
                        }else{
                            EXPECT_EQ(bb.popcount(), 4) << "HEN : BLACK";
                        }
                    }else if(cc == WHITE){
                        if(ss == SQ_2B || ss == SQ_2C){
                            EXPECT_EQ(bb.popcount(), 6) << "HEN : WHITE : center"; 
                        }else if(ss == SQ_3A || ss == SQ_1A){
                            EXPECT_EQ(bb.popcount(), 3) << "HEN : WHITE : A corner";
                        }else if(ss == SQ_2A){
                            EXPECT_EQ(bb.popcount(), 5) << "HEN : WHITE : A center";
                        }else if(ss == SQ_3D || ss == SQ_1D){
                            EXPECT_EQ(bb.popcount(), 2) << "HEN : WHITE : D corner";
                        }else if(ss == SQ_2D){
                            EXPECT_EQ(bb.popcount(), 3) << "HEN : WHITE : D center";
                        }else{
                            EXPECT_EQ(bb.popcount(), 4) << "HEN : WHITE";
                        }
                    }
                }else if(pp == NO_PIECE_TYPE){
                    EXPECT_EQ(bb.popcount(), 0);
                }
            }
        }
    }
}

TEST(Attacks, CheckSampleBitboardFromAttackTable){
    Bitboard bb1 = attacks_from(ELEPHANT, BLACK, SQ_2D);
    for(int sq = 0; sq < SQ_NB; sq++){
        Square s = Square(sq);
        if(s == SQ_3C || s == SQ_1C){
            EXPECT_TRUE(bb1.test(s));
        }else{
            EXPECT_FALSE(bb1.test(s));
        }
    }

    Bitboard bb2 = attacks_from(HEN, WHITE, SQ_2B);
    for(int sq = 0; sq < SQ_NB; sq++){
        Square s = Square(sq);
        if(s == SQ_2A || s == SQ_3B || s == SQ_1B || rank_of(s) == RANK_C){
            EXPECT_TRUE(bb2.test(s));
        }else{
            EXPECT_FALSE(bb2.test(s));
        }
    }
}

TEST(Attacks, OrientationOfChick){
    EXPECT_FALSE(attacks_from(B_CHICK, SQ_3A));
    EXPECT_FALSE(attacks_from(B_CHICK, SQ_2A));
    EXPECT_FALSE(attacks_from(B_CHICK, SQ_1A));
    EXPECT_EQ(attacks_from(B_CHICK, SQ_3B), square_bb(SQ_3A));
    EXPECT_EQ(attacks_from(B_CHICK, SQ_2B), square_bb(SQ_2A));
    EXPECT_EQ(attacks_from(B_CHICK, SQ_1B), square_bb(SQ_1A));
    EXPECT_EQ(attacks_from(B_CHICK, SQ_3C), square_bb(SQ_3B));
    EXPECT_EQ(attacks_from(B_CHICK, SQ_2C), square_bb(SQ_2B));
    EXPECT_EQ(attacks_from(B_CHICK, SQ_1C), square_bb(SQ_1B));
    EXPECT_EQ(attacks_from(B_CHICK, SQ_3D), square_bb(SQ_3C));
    EXPECT_EQ(attacks_from(B_CHICK, SQ_2D), square_bb(SQ_2C));
    EXPECT_EQ(attacks_from(B_CHICK, SQ_1D), square_bb(SQ_1C));

    EXPECT_EQ(attacks_from(W_CHICK, SQ_3A), square_bb(SQ_3B));
    EXPECT_EQ(attacks_from(W_CHICK, SQ_2A), square_bb(SQ_2B));
    EXPECT_EQ(attacks_from(W_CHICK, SQ_1A), square_bb(SQ_1B));
    EXPECT_EQ(attacks_from(W_CHICK, SQ_3B), square_bb(SQ_3C));
    EXPECT_EQ(attacks_from(W_CHICK, SQ_2B), square_bb(SQ_2C));
    EXPECT_EQ(attacks_from(W_CHICK, SQ_1B), square_bb(SQ_1C));
    EXPECT_EQ(attacks_from(W_CHICK, SQ_3C), square_bb(SQ_3D));
    EXPECT_EQ(attacks_from(W_CHICK, SQ_2C), square_bb(SQ_2D));
    EXPECT_EQ(attacks_from(W_CHICK, SQ_1C), square_bb(SQ_1D));
    EXPECT_FALSE(attacks_from(W_CHICK, SQ_3D));
    EXPECT_FALSE(attacks_from(W_CHICK, SQ_2D));
    EXPECT_FALSE(attacks_from(W_CHICK, SQ_1D));
}

TEST(Attacks, CheckingDoNotWrapSquare){
    for(int pt_ = 0; pt_ < PIECE_TYPE_NB; pt_++){
        for(int c_ = 0; c_ < COLOR_NB; c_++){
            for(int s_ = 0; s_ < SQ_NB; s_++){
                PieceType pt = PieceType(pt_); Color c = Color(c_); Square s = Square(s_);
                Bitboard bb = attacks_from(pt, c, s);
                while(bb){
                    Square t = bb.pop_lsb();
                    EXPECT_LE(std::abs(int(file_of(t)) - int(file_of(s))), 1);
                    EXPECT_LE(std::abs(int(rank_of(t)) - int(rank_of(s))), 1);
                }
            }
        }
    }
}

TEST(Attacks, CheckSymmetryOfBlackAndWhite){
    for(int pt_ = 0; pt_ < PIECE_TYPE_NB; pt_++){
        for(int s_ = 0; s_ < SQ_NB; s_++){
            PieceType pt = PieceType(pt_); Square s = Square(s_);
            auto reverse = [](Square sq) { return Square(SQ_1D - sq); };
            Bitboard bb_b = attacks_from(pt, BLACK, s);
            Bitboard bb_w = attacks_from(pt, WHITE, reverse(s));
            Bitboard bb_reverse;
            while(bb_w){
                Square sq = bb_w.pop_lsb();
                bb_reverse.set(reverse(sq));
            }
            EXPECT_EQ(bb_b, bb_reverse);
        }
    }
}

#include <gtest/gtest.h>
#include <dobutsu/types.hpp>

using namespace dobutsu;

// 駒のパック/アンパック
TEST(Types, PackUnpack){
    for(int c = 0; c < COLOR_NB; c++){
        for(int pt = 1; pt < PIECE_TYPE_NB; pt++){
            Piece p = make_piece(Color(c), PieceType(pt));
            PieceType pt_target = type_of(p);
            Color c_target = color_of(p);

            EXPECT_EQ(c_target, c) << "c = " << c;
            EXPECT_EQ(pt_target, pt) << "pt = " << pt;
        }
    }
}

TEST(Types, ConstPiece){
    EXPECT_EQ(make_piece(WHITE, LION), W_LION);
    EXPECT_EQ(make_piece(WHITE, GIRAFFE), W_GIRAFFE);
    EXPECT_EQ(make_piece(WHITE, ELEPHANT), W_ELEPHANT);
    EXPECT_EQ(make_piece(WHITE, CHICK), W_CHICK);
    EXPECT_EQ(make_piece(WHITE, HEN), W_HEN);
    EXPECT_EQ(make_piece(BLACK, LION), B_LION);
    EXPECT_EQ(make_piece(BLACK, GIRAFFE), B_GIRAFFE);
    EXPECT_EQ(make_piece(BLACK, ELEPHANT), B_ELEPHANT);
    EXPECT_EQ(make_piece(BLACK, CHICK), B_CHICK);
    EXPECT_EQ(make_piece(BLACK, HEN), B_HEN);
}

TEST(Types, ReverseColor){
    EXPECT_EQ(~BLACK, WHITE);
    EXPECT_EQ(~WHITE, BLACK);
}

TEST(Types, RoundSquare){
    for(int s = 0; s < SQ_NB; s++){
        EXPECT_EQ(make_square(file_of(Square(s)), rank_of(Square(s))), s) << "s = " << s;
    }
}

TEST(Types, ConstSquare){
    EXPECT_EQ(make_square(FILE_3, RANK_D), SQ_3D);
    EXPECT_EQ(make_square(FILE_2, RANK_D), SQ_2D);
    EXPECT_EQ(make_square(FILE_1, RANK_C), SQ_1C);
    EXPECT_EQ(make_square(FILE_3, RANK_C), SQ_3C);
    EXPECT_EQ(make_square(FILE_2, RANK_B), SQ_2B);
    EXPECT_EQ(make_square(FILE_1, RANK_B), SQ_1B);
    EXPECT_EQ(make_square(FILE_3, RANK_A), SQ_3A);
    EXPECT_EQ(make_square(FILE_2, RANK_A), SQ_2A);
}


TEST(Types, IsOK){
    for(int s = 0; s < 16; s++){
        EXPECT_EQ(is_ok(Square(s)), s < 12) << "s = " << s;
    }
    EXPECT_EQ(is_ok(SQ_NONE), false);
}

TEST(Types, TransUSI){
    EXPECT_EQ(square_to_usi(SQ_3A), "3a");
    EXPECT_EQ(square_to_usi(SQ_2A), "2a");
    EXPECT_EQ(square_to_usi(SQ_1B), "1b");
    EXPECT_EQ(square_to_usi(SQ_3B), "3b");
    EXPECT_EQ(square_to_usi(SQ_2C), "2c");
    EXPECT_EQ(square_to_usi(SQ_1C), "1c");
    EXPECT_EQ(square_to_usi(SQ_3D), "3d");
    EXPECT_EQ(square_to_usi(SQ_2D), "2d");

    EXPECT_EQ(square_from_usi("3d"), SQ_3D);
    EXPECT_EQ(square_from_usi("2c"), SQ_2C);
    EXPECT_EQ(square_from_usi("1b"), SQ_1B);
    EXPECT_EQ(square_from_usi("3a"), SQ_3A);
    EXPECT_EQ(square_from_usi("2d"), SQ_2D);
    EXPECT_EQ(square_from_usi("1c"), SQ_1C);
    EXPECT_EQ(square_from_usi("3b"), SQ_3B);
    EXPECT_EQ(square_from_usi("2a"), SQ_2A);
}


TEST(Types, AllUSI){
    for(int s = 0; s < 12; s++){
        EXPECT_EQ(square_from_usi(square_to_usi(Square(s))), s) << "s = " << s;
    }
}

TEST(Types, InvalidInput){
    EXPECT_EQ(square_from_usi("abc"), SQ_NONE);
    EXPECT_EQ(square_from_usi("3f"), SQ_NONE);
    EXPECT_EQ(square_from_usi("2A"), SQ_NONE);
    EXPECT_EQ(square_from_usi(""), SQ_NONE);
    EXPECT_EQ(square_to_usi(SQ_NONE), "");
}

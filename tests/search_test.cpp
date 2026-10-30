#include <cstdint>
#include <gtest/gtest.h>
#include <dobutsu/types.hpp>
#include <dobutsu/position.hpp>
#include <dobutsu/movegen.hpp>
#include <dobutsu/evaluate.hpp>
#include <dobutsu/search.hpp>
#include <string>

using namespace dobutsu;

namespace {
    int minimax(Position& pos, int depth, int ply, std::uint64_t& nodes){
        nodes++;
        GameResult gr = pos.result();
        if(gr == GameResult::CATCH) return VALUE_MATE - ply;
        else if(gr == GameResult::TRY) return -(VALUE_MATE - ply);
        else if(gr == GameResult::REPETITION) return 0;

        if(depth == 0){
            return evaluate(pos);
        }

        int best = -VALUE_INFINITE;
        for(const Move m : MoveList(pos)){
            StateInfo st;
            pos.do_move(m, st);
            const int v =-minimax(pos, depth - 1, ply + 1, nodes);
            pos.undo_move(m);
            if(v > best) best = v;
        }
        return best;
    }

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

    const std::string sfen_list_mate_by_try[] = {
        "2l/L2/3/3 b 2G2E2C 3",
        "3/L2/2l/eeg b G2c 17",
        "3/3/l2/2L w 2g2e2c 22",
        "EEG/2L/l2/3 w G2C 36"
    };

    const std::string sfen_list_mate_by_catch[] = {
        "2l/1L1/3/3 b 2G2E2C 47",
        "1EG/2L/1lG/3 b e2c 51",
        "1l1/3/g+cg/eLE w C 62",
        "1l1/2L/3/3 w GECgec 78"
    };
}

TEST(Search, VerifyThatAlphabetaAndMinimaxReturnTheSameResult){
    Searcher s{};
    for(const std::string& sfen : sfen_list){
        for(int depth = 1; depth < 5; depth++){
            Position pos = Position::from_sfen(sfen).value();
            std::uint64_t nodes = 0;
            SearchResult result = s.search(pos, SearchLimits{depth});
            int score_ab = result.score;
            int score_mm = minimax(pos, depth, 0, nodes);
            EXPECT_EQ(score_ab, score_mm);
            if(result.bestMove != Move::NONE){
                StateInfo st; std::uint64_t n = 0;
                pos.do_move(result.bestMove, st);
                int score = -minimax(pos, depth - 1, 1, n);
                EXPECT_EQ(score, score_ab);
            }
        }
    }
}

TEST(Search, CheckThatCanChooseCheckmateMove){
    Searcher s{};
    for(const std::string& sfen : sfen_list_mate_by_try){
        Position pos = Position::from_sfen(sfen).value();
        EXPECT_EQ(VALUE_MATE - 1, s.search(pos, SearchLimits{2}).score);
    }
    for(const std::string& sfen : sfen_list_mate_by_catch){
        Position pos = Position::from_sfen(sfen).value();
        SearchResult result = s.search(pos, SearchLimits{2});
        EXPECT_EQ(VALUE_MATE, result.score);
        EXPECT_EQ(Move::NONE, result.bestMove);
    }
}

TEST(Search, NodesOfAlphabetaAreLessThanNodesOfMinimax){
    Searcher s{};
    int depth = 4;
    for(const std::string& sfen : sfen_list){
        std::uint64_t nodes_mm = 0;
        Position pos = Position::from_sfen(sfen).value();
        SearchResult result = s.search(pos, SearchLimits{depth});
        int score_mm = minimax(pos, depth, 0, nodes_mm);
        EXPECT_EQ(score_mm, result.score);
        EXPECT_LT(result.nodes, nodes_mm);
    }
    
    for(int depth : {4, 5, 6}){
        Position pos = Position::startpos();
        SearchResult result = s.search(pos, SearchLimits{depth});
        std::uint64_t nodes_mm = 0;
        int score_mm = minimax(pos, depth, 0, nodes_mm);
        EXPECT_EQ(score_mm, result.score);
        EXPECT_LT(result.nodes, nodes_mm);
        if(depth == 4) EXPECT_EQ(result.nodes, 114);
        else if(depth == 5) EXPECT_EQ(result.nodes, 503);
        else if(depth == 6) EXPECT_EQ(result.nodes, 2616);
    }
}

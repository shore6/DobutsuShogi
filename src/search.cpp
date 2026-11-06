#include <dobutsu/types.hpp>
#include <dobutsu/position.hpp>
#include <dobutsu/movegen.hpp>
#include <dobutsu/evaluate.hpp>
#include <dobutsu/search.hpp>

namespace dobutsu {
    
    int Searcher::alphabeta(Position& pos, int depth, int ply, int alpha, int beta){
        nodes_++;
        GameResult gr = pos.result();
        if(gr == GameResult::CATCH) return VALUE_MATE - ply;
        else if(gr == GameResult::TRY) return -(VALUE_MATE - ply);
        else if(gr == GameResult::REPETITION) return 0;

        if(depth <= 0){
            return evaluate(pos);
        }

        int best = -VALUE_INFINITE;
        for(const Move m : MoveList(pos)){
            StateInfo st;
            pos.do_move(m, st);
            const int v =-alphabeta(pos, depth - 1, ply + 1, -beta, -alpha);
            pos.undo_move(m);
            if(v > best) best = v;
            if(v > alpha) alpha = v;
            if(alpha >= beta) break;
        }
        
        return best;
    }


    SearchResult Searcher::search(Position& pos, const SearchLimits& limits){
        SearchResult result{}; result.score = -VALUE_INFINITE;
        nodes_ = 0;
        nodes_++;
        
        GameResult gr = pos.result();
        if(gr == GameResult::CATCH){
            result.score = VALUE_MATE;
            return result;
        }else if(gr == GameResult::TRY){
            result.score = -VALUE_MATE;
            return result;
        }else if(gr == GameResult::REPETITION){
            return result;
        }
        
        for(const Move m : MoveList(pos)){
            StateInfo st;
            pos.do_move(m, st);
            int score = -alphabeta(pos, limits.depth - 1, 1, -VALUE_INFINITE, -result.score);
            if(score > result.score){
                result.score = score; result.bestMove = m;
            }
            pos.undo_move(m);
        }

        result.depth = limits.depth;
        result.nodes = nodes_;
        return result;
    }
}

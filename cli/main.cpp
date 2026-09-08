#include <cstdint>
#include <iostream>
#include <iomanip>
#include <ostream>
#include <string>
#include <system_error>
#include <vector>
#include <chrono>
#include <charconv>
#include <utility>
#include <optional>
#include <dobutsu/types.hpp>
#include <dobutsu/bitboard.hpp>
#include <dobutsu/position.hpp>
#include <dobutsu/movegen.hpp>

void help(){
    std::cout << "dobutsu Version 0.3.0\n";
    std::cout << "Usage: dobutsu [options]\n";
    std::cout << "Options:\n";
    std::cout << "  help                    Display this information.\n";
    std::cout << "  show <sfen>             Dump board generated from SFEN.\n";
    std::cout << "                          (if you enter \"startpos\" instead, it will output board of Starting position according to general rules.)\n";
    std::cout << "  perft <depth> [sfen]    Run perft at the specified depth.\n";
    std::cout << "                          If SFEN is specified, it runs from that board position; if not, it runs from the initial board position.\n";

    std::cout << std::endl;
}

int show(int argc, std::vector<std::string> args){
    if(argc < 3){
        std::cerr << "show requires an SFEN or \"startpos\"." << std::endl;
        return 1;
    }
    if(args[2] == "startpos"){
        std::cout << dobutsu::Position::startpos().to_ascii();
        return 0;
    }
    std::string sfen = "";
    for(int i = 2; i < argc; i++){
        sfen += args[i];
        sfen += " ";
    }
    auto p = dobutsu::Position::from_sfen(sfen);
    if(!p.has_value()){
        std::cerr << "Unrecognized SFEN." << std::endl;
        return 1;
    }else{
        std::cout << p->to_ascii();
    }

    return 0;
}

int perft(int argc, std::vector<std::string> args){
    if(argc < 3){
        std::cerr << "perft requires depth." << std::endl;
        return 1;
    }

    int depth;
    auto [ptr, ec] = std::from_chars(args[2].data(), args[2].data() + args[2].size(), depth);
    if(ec != std::errc{} || ptr != args[2].data() + args[2].size()){
        std::cerr << "Unrecognized depth input." << std::endl;
        return 1;
    }
    if(depth < 1){
        std::cerr << "Invalid depth value." << std::endl;
        return 1;
    }
    if(depth > 10){
        std::cerr << "warning : Too large value of depth. Processing may take a long time\n";
    }

    std::optional<dobutsu::Position> pos;
    std::string sfen = "";
    if(argc > 3){
        if(args[3] == "startpos"){
            pos = dobutsu::Position::startpos();
        }else{
            for(int i = 3; i < argc; i++){
                sfen += args[i];
                sfen += " ";
            }
            pos = dobutsu::Position::from_sfen(sfen);
            if(!pos.has_value()){
                std::cerr << "Unrecognized SFEN." << std::endl;
                return 1;
            }
        }
    }else{
        pos = dobutsu::Position::startpos();
    }

    if(pos->is_terminal()){
        std::cout << "nodes: 1" << std::endl;
        return 0;
    }

    dobutsu::MoveList ml(pos.value());
    std::vector<std::pair<dobutsu::Move, std::uint64_t>> moves;
    for(dobutsu::Move m : ml){
        moves.push_back(std::pair<dobutsu::Move, std::uint64_t>(m, 0));
    }
    const auto start = std::chrono::steady_clock::now();
    for(auto& [m, nodes] : moves){
        dobutsu::StateInfo st;
        pos->do_move(m, st);
        nodes = dobutsu::perft(pos.value(), depth-1);
        pos->undo_move(m);
    }
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
    
    std::uint64_t total_nodes = 0;
    for(const auto& [m, nodes] : moves){
        std::cout << dobutsu::move_to_usi(m) << ": " << nodes << "\n";
        total_nodes += nodes;
    }
    std::cout << "nodes: " << total_nodes << "\n";
    std::cout << "elapsed time: " << ms << "ms\n";
    if(ms == 0){
        std::cout << "NPS: -\n";
    }else{
        std::cout << "NPS: " << std::fixed << std::setprecision(1) << (total_nodes / (ms / 1000.0)) << "\n";
    }

    return 0;
}


int main(int argc, char* argv[]){
    std::vector<std::string> args(argv, argv + argc);

    // 引数をサブコマンドとして処理
    if(argc < 2 || args[1] == "help"){
        help();
        return 0;
    }
    if(args[1] == "show"){
        return show(argc, args);
    }else if(args[1] == "perft"){
        return perft(argc, args);
    }else{
        std::cerr << "Unrecognized command \"" << args[1] << "\". Use \"help\"." << std::endl;
        return 1;
    }


    return 0;
}

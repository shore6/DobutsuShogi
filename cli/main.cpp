#include <iostream>
#include <string>
#include <vector>
#include <dobutsu/types.hpp>
#include <dobutsu/bitboard.hpp>
#include <dobutsu/position.hpp>

void dump_help(){
    std::cout << "dobutsu Version 0.2.0\n";
    std::cout << "Usage: dobutsu [options]\n";
    std::cout << "Options:\n";
    std::cout << "  show <sfen>\tDump board generated from SFEN.\n";
    std::cout << "             \t(if you enter \"startpos\" instead, it will output board of Starting position according to general rules.)" << std::endl;
}


int main(int argc, char* argv[]){
    std::vector<std::string> args(argv, argv + argc);

    // 引数をサブコマンドとして処理
    if(argc < 2 || args[1] == "help"){
        dump_help();
        return 0;
    }
    if(args[1] == "show"){
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
    }else{
        std::cerr << "Unrecognized command \"" << args[1] << "\". Use \"help\"." << std::endl;
        return 1;
    }


    return 0;
}

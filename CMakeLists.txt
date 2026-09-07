cmake_minimum_required(VERSION 3.20)
project(dobutsu VERSION 0.2 LANGUAGES CXX)

# オプション
option(DOBUTSU_BUILD_TESTS "Build unit tests" ON)

# コアライブラリ
add_library(dobutsu_core STATIC src/types.cpp src/position.cpp src/movegen.cpp)
target_include_directories(dobutsu_core PUBLIC include)
target_compile_features(dobutsu_core PUBLIC cxx_std_20)

# CLI 実行ファイル
add_executable(dobutsu_cli cli/main.cpp)
target_link_libraries(dobutsu_cli PRIVATE dobutsu_core)

# テスト
if(DOBUTSU_BUILD_TESTS)
    enable_testing()            # CTest を有効化
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.15.2
        GIT_SHALLOW TRUE
    )

    # MSVC 対策
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(googletest)
    add_executable(dobutsu_tests tests/types_test.cpp tests/bitboard_test.cpp tests/position_test.cpp tests/types_move_test.cpp tests/attack_test.cpp tests/movegen_test.cpp tests/perft.cpp)
    target_link_libraries(dobutsu_tests PRIVATE dobutsu_core GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(dobutsu_tests)
endif()

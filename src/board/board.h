#pragma once 

#include <iostream>
#include <numeric>
#include <algorithm>
#include <array>
#include <string>
#include <map>
#include <vector>
#include <thread>
#include <functional>
#include <queue>
#include <cmath>

#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/component/event.hpp"               // for Event

extern ftxui::ScreenInteractive screen ;
extern std::chrono::milliseconds drop_time;

struct board {
    int lines_cleared = 0, score = 0, level = 0;
    static constexpr size_t width = 10, height = 22;    
    std::vector<size_t> data;
    std::vector<size_t> color_data;
    bool can_save = true;

    struct tetromino {
        size_t cnt = 0;
        size_t width, height;
        int brd_x = 0, brd_y = 0;
        size_t piece_type = 0;
        std::vector<size_t> data{};
        tetromino();
        void move_left();
        void move_right();
        void move_down();
        void rotate_right();
        void rotate_left();
    };

    void drop_tetr(std::unique_ptr<tetromino>&);
    bool is_correct(tetromino);
    std::unique_ptr<tetromino> cur_tetr = nullptr, saved_tetr = nullptr;
    std::queue<std::unique_ptr<tetromino>> tetr_queue;
    board();
    tetromino* get_new_tetromino();
    std::vector<size_t> get_view();
    std::vector<size_t> get_color_view();
    void remove_lines();
    void save_tetr();
    void place_tetr();
    bool try_move(void(board::tetromino::*func)(), std::unique_ptr<tetromino>&);
    void process_input(const std::string& key);
};

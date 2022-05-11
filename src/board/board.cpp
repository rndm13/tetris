#include "board.h"


board::tetromino::tetromino(void) {
    std::vector<std::vector<int>> tetrominoes = {
    {0, 0, 0, 0,
     0, 1, 1, 0,
     0, 1, 1, 0,
     0, 0, 0, 0 },
    {0, 1, 0, 0,
     0, 1, 0, 0,
     0, 1, 0, 0,
     0, 1, 0, 0 },
    {0, 0, 0, 
     0, 1, 1, 
     1, 1, 0 },
    {0, 0, 0,
     1, 1, 0,
     0, 1, 1,},
    {0, 1, 0, 
     1, 1, 1,  
     0, 0, 0,},
    {1, 0, 0, 
     1, 1, 1,  
     0, 0, 0,},
    {0, 0, 1, 
     1, 1, 1,  
     0, 0, 0,}
    };

    static std::vector<int> bag(7);
    static size_t c = 0;
    cnt = ++c;

    static auto it = bag.begin();

    if (it == bag.end())
        it = bag.begin();

    if (it == bag.begin()) {
        std::iota(bag.begin(),bag.end(),0);
        std::random_shuffle(bag.begin(),bag.end());
    }
    
    piece_type = *it;

    tetromino::data.resize(tetrominoes.at(*it).size());

    width = height = sqrt(tetrominoes.at(*it).size());

    for (size_t i = 0; i < tetrominoes.at(*it).size(); ++i)
        tetromino::data[i] = tetrominoes.at(*it).at(i) * c ;

    ++it;
}

void board::tetromino::move_left(void) { --brd_x; }
void board::tetromino::move_right(void) { ++brd_x; }
void board::tetromino::move_down(void) { ++brd_y; }

void board::tetromino::rotate_right(void) {
    std::vector<size_t> new_data(data.size());
    for (size_t y = 0 ;y < height;++y) 
        for (size_t x = 0; x < width ;++x) 
            new_data[(width - y - 1) + x*width] = data[x + y*width];
    std::copy(new_data.begin(),new_data.end(),data.begin());
}

void board::tetromino::rotate_left(void) {
    std::vector<size_t> new_data(data.size());
    for (size_t y = 0 ;y < height; ++y) 
        for (size_t x = 0; x < width; ++x) 
            new_data[y + (height - x - 1)*width] = data[x + y*width];
    std::copy(new_data.begin(),new_data.end(),data.begin());
}

bool board::is_correct(tetromino tetr) {
    for (size_t y = 0;y < tetr.height; ++y) {
        for (size_t x = 0;x < tetr.width; ++x) {
            if (tetr.data.at(x + y * tetr.width) > 0 && 
                (static_cast<int>(x) + tetr.brd_x < 0     ||
                                 x + tetr.brd_x >= width  ||
                                 y + tetr.brd_y >= height || 
                data.at(x + tetr.brd_x + (y + tetr.brd_y)*width) > 0))
                return false;
        }
    }
    return true;
}

board::board() {
    srand(time(NULL));
    data.resize(board::width*board::height, 0);
    color_data.resize(board::width*board::height, 0);
    for (int i = 0;i<3;++i) 
        tetr_queue.push(std::unique_ptr<tetromino>(new tetromino()));
    cur_tetr.reset(get_new_tetromino());
}

board::tetromino* board::get_new_tetromino(void) {
    std::unique_ptr<tetromino> ret(tetr_queue.front().release());
    tetr_queue.pop();
    tetr_queue.push(std::unique_ptr<tetromino>(new tetromino()));
    return ret.release();
}

std::vector<size_t> board::get_view(void) {
    auto data_view = data;
    for (size_t x = 0; x < cur_tetr -> width; ++x)
        for (size_t y = 0; y < cur_tetr -> height; ++y)
            if (cur_tetr -> data.at(x + y*cur_tetr -> width))
                data_view.at(x + cur_tetr -> brd_x + (y + cur_tetr -> brd_y)*width) = cur_tetr -> data.at(x + y*cur_tetr -> width);
    return data_view;
}

std::vector<size_t> board::get_color_view(void) {
    auto data_view = color_data;
    for (size_t x = 0; x < cur_tetr -> width; ++x)
        for (size_t y = 0; y < cur_tetr -> height; ++y)
            if (cur_tetr -> data.at(x + y*cur_tetr -> width))
                data_view.at(x + cur_tetr -> brd_x + (y + cur_tetr -> brd_y)*width) = cur_tetr -> piece_type;
    return data_view;
}

void board::remove_lines(void) {
    size_t offset_by = 0;
    for (int y = height-1; y >= 0; --y) {
        if (std::find(data.begin() + y*width, data.begin() + (y+1)*width, 0) == data.begin() + (y+1)*width)  {
            std::fill_n(data.begin() + (y+offset_by)*width, width, 0);
            std::fill_n(color_data.begin() + (y+offset_by)*width, width, 0);
            ++offset_by;
        }
        else if (offset_by > 0) {
            std::copy(data.begin() + y*width, data.begin() + (y+1)*width, data.begin() + (y+offset_by)*width);
            std::copy(color_data.begin() + y*width, color_data.begin() + (y+1)*width, color_data.begin() + (y+offset_by)*width);
        }
    }
    score += offset_by*offset_by;
    lines_cleared += offset_by;
    level = std::min(9,lines_cleared/15);
    drop_time = std::chrono::milliseconds(1300 - level*115);
}

void board::save_tetr(void) {
    if (!can_save) return;
    can_save = false;
    saved_tetr.swap(cur_tetr);
    if (!cur_tetr) 
        cur_tetr.reset(get_new_tetromino());
    saved_tetr -> brd_y = 0;
}

void board::place_tetr(void) {
    can_save = true;
    for (size_t x = 0; x < cur_tetr -> width; ++x) {
        for (size_t y = 0; y < cur_tetr -> height; ++y) {
            if (cur_tetr -> data.at(x + y*cur_tetr -> width) > 0 ){
                if (data.at(x + cur_tetr -> brd_x + (y + cur_tetr -> brd_y)*width) > 0 ) {
                    screen.PostEvent(ftxui::Event::Escape);
                }
                else {
                    color_data.at(x + cur_tetr -> brd_x + (y + cur_tetr -> brd_y)*width) = cur_tetr -> piece_type;
                    data.at(x + cur_tetr -> brd_x + (y + cur_tetr -> brd_y)*width) = cur_tetr -> data.at(x + y*cur_tetr -> width);
                }
            }
        }
    }
    cur_tetr.reset(get_new_tetromino());
    remove_lines();
}

bool board::try_move(void(board::tetromino::*func)(void)) {
    tetromino tetr = *cur_tetr;
    (tetr.*func)();
    if (is_correct(tetr)) {
        cur_tetr.reset(new tetromino(tetr));
        return true;
    }
    else {
        if (func == &board::tetromino::move_down) {
            place_tetr();
            return false;
        }
        if (func == &board::tetromino::rotate_left || func == &board::tetromino::rotate_right) {
            for (int dx = 0; dx <= 2;++dx)
                for (int dy = 0;dy <= 2;++dy) {
                    tetr.brd_x += dx;
                    tetr.brd_y += dy;
                    if (is_correct(tetr)) {
                        cur_tetr.reset(new tetromino(tetr));
                        return true;
                    }
                    tetr.brd_x -= 2*dx;
                    tetr.brd_y -= 2*dy;

                    if (is_correct(tetr)) {
                        cur_tetr.reset(new tetromino(tetr));
                        return true;
                    }

                    tetr.brd_x += dx;
                    tetr.brd_y += dy;
                }
        }
    }
    return true;
}

void board::drop_tetr(void) {
    while(try_move(&board::tetromino::move_down));
}

void board::process_input(const std::string& key) {
    static std::unordered_map<std::string, void (board::tetromino::*)(void)> key_lookup_tetr {
        {"a", &board::tetromino::move_left},
        {"s", &board::tetromino::move_down},
        {"d", &board::tetromino::move_right},
        {"q", &board::tetromino::rotate_left},
        {"e", &board::tetromino::rotate_right},
    };
    {
    auto it = key_lookup_tetr.find(key);
    if (it != key_lookup_tetr.end()) {
        try_move(it -> second);
        return;
    }
    }

    if (key == "c") save_tetr();
    else if (key == "w") drop_tetr();

}

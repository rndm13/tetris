#include "board.h"

std::ostream& operator<<(std::ostream& os, const board::tetromino& tetr) {
    os << tetr.cnt << '\n';
    os << tetr.height << ' ' << tetr.width <<'\n';
    os << tetr.brd_x  << ' ' << tetr.brd_y <<'\n';
    os << tetr.piece_type << '\n';
    for (size_t y = 0;y<tetr.height;++y) {
        for (size_t x = 0;x<tetr.width;++x) {
            os << tetr.data.at(x + y*tetr.width);
        }
        os << '\n';
    }
    return os;
}

static size_t max_cnt = 0;

board::tetromino::tetromino() {
    cnt = ++max_cnt;
}

board::tetromino::tetromino(const tetromino& _tetr) {
    cnt = ++max_cnt;
    data.resize(_tetr.data.size());
    std::copy(_tetr.data.begin(), _tetr.data.end(), data.begin());
    std::for_each(data.begin(), data.end(), [v = cnt](size_t& x){if (x>0) x = v;});

    width = _tetr.width;
    height = _tetr.height;
    brd_x = _tetr.brd_x;
    brd_y = _tetr.brd_y;
    piece_type = _tetr.piece_type;
}

void board::tetromino::move_left() { --brd_x; }
void board::tetromino::move_right() { ++brd_x; }
void board::tetromino::move_down() { ++brd_y; }

void board::tetromino::rotate_right() {
    std::vector<size_t> new_data(data.size());
    for (size_t y = 0 ;y < height;++y) 
        for (size_t x = 0; x < width ;++x) 
            new_data[(width - y - 1) + x*width] = data[x + y*width];
    std::copy(new_data.begin(),new_data.end(),data.begin());
}

void board::tetromino::rotate_left() {
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

constexpr size_t queue_size = 3;

board::board() {
    srand(time(NULL));
    data.resize(board::width*board::height, 0);
    color_data.resize(board::width*board::height, 0);

    for (size_t i;i<queue_size;++i)
        tetr_queue.push(std::unique_ptr<tetromino>(get_bag_tetromino()));

    cur_tetr.reset(get_queue_tetromino());
    std::cout <<"cur_tetr: " <<*cur_tetr <<'\n';
}

board::tetromino* board::get_queue_tetromino() {
//    std::cout << "get queue begin\n";
    tetromino *ret = tetr_queue.front().release();
    tetr_queue.pop();
    tetr_queue.emplace(get_bag_tetromino());
//    std::cout << "get queue end\n";
    return ret;
}


board::tetromino* board::get_bag_tetromino() {
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
//    std::cout << "get bag start\n";
    
    tetromino* _tetr = new tetromino();
    static std::vector<int> bag(7);
    static auto it = bag.begin();
    
    if (it == bag.end())
        it = bag.begin();

    if (it == bag.begin()) {
        std::iota(bag.begin(),bag.end(),0);
        std::random_shuffle(bag.begin(),bag.end());
    }
    
    _tetr -> piece_type = *it;

    _tetr -> data.resize(tetrominoes.at(*it).size());

    _tetr -> width = _tetr -> height = sqrt(tetrominoes.at(*it).size());

    std::copy(tetrominoes.at(*it).begin(),tetrominoes.at(*it).end(),_tetr -> data.begin());
    std::for_each(_tetr -> data.begin(), _tetr -> data.end(), [](size_t& x){x*=max_cnt;});

    ++it;
//    std::cout << "get bag end\n";
    return _tetr;
}

std::vector<size_t> board::get_view() {
    auto data_view = data;
    std::unique_ptr<tetromino> dropped_tetr(new tetromino(*cur_tetr));

    drop_tetr(dropped_tetr);

    for (size_t x = 0; x < dropped_tetr -> width; ++x)
        for (size_t y = 0; y < dropped_tetr -> height; ++y) 
            if (dropped_tetr -> data.at(x + y*dropped_tetr -> width)) 
                data_view.at(x + dropped_tetr -> brd_x + (y + dropped_tetr -> brd_y)*width) = dropped_tetr -> data.at(x + y*dropped_tetr -> width);

    for (size_t x = 0; x < cur_tetr -> width; ++x)
        for (size_t y = 0; y < cur_tetr -> height; ++y) 
            if (cur_tetr -> data.at(x + y*cur_tetr -> width))
                data_view.at(x + cur_tetr -> brd_x + (y + cur_tetr -> brd_y)*width) = cur_tetr -> data.at(x + y*cur_tetr -> width);

    return data_view;
}

std::vector<size_t> board::get_color_view() {
    auto data_view = color_data;
    std::unique_ptr<tetromino> dropped_tetr(new tetromino(*cur_tetr));
    drop_tetr(dropped_tetr);

    for (size_t x = 0; x < dropped_tetr -> width; ++x)
        for (size_t y = 0; y < dropped_tetr -> height; ++y)
            if (dropped_tetr -> data.at(x + y*dropped_tetr -> width))
                data_view.at(x + dropped_tetr -> brd_x + (y + dropped_tetr -> brd_y)*width) = 0;

    for (size_t x = 0; x < cur_tetr -> width; ++x)
        for (size_t y = 0; y < cur_tetr -> height; ++y)
            if (cur_tetr -> data.at(x + y*cur_tetr -> width))
                data_view.at(x + cur_tetr -> brd_x + (y + cur_tetr -> brd_y)*width) = cur_tetr -> piece_type + 1;

    return data_view;
}

void board::remove_lines() {
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

void board::save_tetr() {
    if (!can_save) return;
    can_save = false;
    saved_tetr.swap(cur_tetr);
    if (!cur_tetr) 
        cur_tetr.reset(get_queue_tetromino());
    saved_tetr -> brd_y = 0;
}

void board::place_tetr() {
    can_save = true;
    for (size_t x = 0; x < cur_tetr -> width; ++x) {
        for (size_t y = 0; y < cur_tetr -> height; ++y) {
            if (cur_tetr -> data.at(x + y*cur_tetr -> width) > 0 ){
                if (data.at(x + cur_tetr -> brd_x + (y + cur_tetr -> brd_y)*width) > 0 ) {
                    screen.PostEvent(ftxui::Event::Escape);
                }
                else {
                    color_data.at(x + cur_tetr -> brd_x + (y + cur_tetr -> brd_y)*width) = cur_tetr -> piece_type + 1;
                    data.at(x + cur_tetr -> brd_x + (y + cur_tetr -> brd_y)*width) = cur_tetr -> data.at(x + y*cur_tetr -> width);
                }
            }
        }
    }
    cur_tetr.reset(get_queue_tetromino());
    remove_lines();
}

bool board::try_move(void(board::tetromino::*func)(),std::unique_ptr<tetromino>& move_tetr) {
    tetromino tetr = *move_tetr;
    (tetr.*func)();
    if (is_correct(tetr)) {
        move_tetr.reset(new tetromino(tetr));
        return true;
    }
    else {
        if (func == &board::tetromino::move_down) {
            if (move_tetr == cur_tetr)
                place_tetr();
            return false;
        }
        if (func == &board::tetromino::rotate_left || func == &board::tetromino::rotate_right) {
            for (int dx = 0; dx <= 2;++dx)
                for (int dy = 0;dy <= 2;++dy) {
                    tetr.brd_x += dx;
                    tetr.brd_y += dy;
                    if (is_correct(tetr)) {
                        move_tetr.reset(new tetromino(tetr));
                        return true;
                    }
                    tetr.brd_x -= 2*dx;
                    tetr.brd_y -= 2*dy;

                    if (is_correct(tetr)) {
                        move_tetr.reset(new tetromino(tetr));
                        return true;
                    }

                    tetr.brd_x += dx;
                    tetr.brd_y += dy;
                }
        }
    }
    return true;
}

void board::drop_tetr(std::unique_ptr<tetromino>& move_tetr) {
    while(try_move(&board::tetromino::move_down, move_tetr));
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
        try_move(it -> second, cur_tetr);
        return;
    }
    }

    if (key == "c") save_tetr();
    else if (key == "w") drop_tetr(cur_tetr);

}

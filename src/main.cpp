#include "board/board.h"

#include "ftxui/dom/canvas.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/string.hpp"
#include "ftxui/component/component.hpp"  // for Renderer, CatchEvent, Horizontal, Menu, Tab
#include "ftxui/component/component_base.hpp"      // for ComponentBase
#include "ftxui/component/mouse.hpp"               // for Mouse
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/screen/color.hpp"  // for Color, Color::Red, Color::Blue, Color::Green, ftxui
 
using namespace ftxui;

board brd;
ScreenInteractive screen = ScreenInteractive::FitComponent();

std::chrono::milliseconds drop_time(1300);

void repeating_timer(const std::chrono::milliseconds& dur, std::function<void(void)> func, Receiver<std::string> receiver) {
    std::string msg;

    while (1) {
        while (receiver -> HasPending() && receiver -> Receive(&msg)) {
            if (msg == "Kill")
            return;
        }
        std::this_thread::sleep_for(dur);
        func();
    }
}

void move_on_time(void) {
    screen.PostEvent(Event::Character("s"));
}

Color my_palette(int x) {
    if (x >= 6) x+=3;
    x=x%11 + 1;
    return Color::Palette16(x);
}

Canvas fill_canvas(const std::vector<size_t>& color_data,const std::vector<size_t>& data, size_t width, size_t height) {
    size_t box_size = 4;

    auto can = Canvas(width*box_size,height*box_size);

    auto check = [&](size_t x, size_t y, size_t val) -> bool { 
        return x < width && y < height && data.at(x + y*width) == val;
    };



    for (int y = 0; y < height;y++) {
        for (int x = 0;x < width;x++) {
            auto d = data.at(x + y*width);
            auto c = color_data.at(x + y*width);
            Color col = Color(my_palette(c + brd.level));
            if (d > 0) {
                for (int dif = 0; dif < 4;++dif) {
                    can.DrawPoint(box_size*x + dif, box_size*y,      true,col);
                    can.DrawPoint(box_size*x + dif, box_size*y + 3,  true,col);
                    can.DrawPoint(box_size*x,       box_size*y + dif,true,col);
                    can.DrawPoint(box_size*x + 3,   box_size*y + dif,true,col);
                }
                std::vector<std::pair<int,int>> d_v { {-1,0},{1,0},{0,1},{0,-1} };
                for (auto [dx, dy] : d_v) {
                    if (check(x + dx, y + dy, d)) {
                        can.DrawPointOff(box_size*x + 2 + dx*2, box_size*y + 2 + dy*2);
                        can.DrawPointOff(box_size*x + 1 + dx*2, box_size*y + 2 + dy*2);
                        can.DrawPointOff(box_size*x + 2 + dx*2, box_size*y + 1 + dy*2);
                        can.DrawPointOff(box_size*x + 1 + dx*2, box_size*y + 1 + dy*2);
                    }
                }
            }
        }
    }
    return can;
}

int main(void) {
    srand(time(NULL));

    auto game_rend = Renderer([&](){
        auto data = brd.get_view();
        auto width = brd.width, height = brd.height;
        return canvas(std::move(fill_canvas(brd.get_color_view(),data,width,height))); 
    }); 

    
    auto save_rend = Renderer([&](){
        size_t box_size = 4;

        if (!brd.saved_tetr)
            return canvas(std::move(Canvas(box_size*4,box_size*4))); 

        auto col_data = brd.saved_tetr -> data;
        std::replace(col_data.begin(),col_data.end(),brd.saved_tetr -> cnt, brd.saved_tetr -> piece_type);
        size_t width = brd.saved_tetr -> width, height = brd.saved_tetr -> height;

        return canvas(std::move(fill_canvas(col_data, brd.saved_tetr -> data,width,height)));
    }); 

    auto left_box = Renderer([&](){
        return vbox({
        save_rend -> Render() | size (HEIGHT, GREATER_THAN, 4),
        separator(),
        text("Level: " + std::to_string(brd.level)) | color(Color::RGB(0,255,0)),
        text("Score: " + std::to_string(brd.score)) | color(Color::RGB(255,0,0)),
        text("Lines cleared: " + std::to_string(brd.lines_cleared)) | color(Color::RGB(0,0,255)),
        separator(),
        text("Move with asd keys.")  | bold,
        text("Rotate with q and e.") | bold,
        text("Drop - w.")            | bold,
        text("Save - c.")            | bold,
        text("Exit - ESC")           | bold
        }) | size (WIDTH, LESS_THAN, 20);
    });

    auto queue_rend = Renderer([&](){
        Elements canvases;

        for (int i = 0;i<brd.tetr_queue.size();++i) {

            auto col_data = brd.tetr_queue.front() -> data;
            std::replace(col_data.begin(),col_data.end(),brd.tetr_queue.front() -> cnt, brd.tetr_queue.front() -> piece_type);

            canvases.push_back(canvas(std::move(fill_canvas(col_data,brd.tetr_queue.front() -> data,brd.tetr_queue.front() -> width,brd.tetr_queue.front() -> height))));
            canvases.push_back(separator());
            brd.tetr_queue.emplace(brd.tetr_queue.front().release());
            brd.tetr_queue.pop();
        }

        return vbox(canvases);
    });

    auto right_box = Renderer([&](){
        return queue_rend -> Render();
    });


    auto screen_rend = Renderer([&](){
        return hbox({
        left_box -> Render(),
        separator(),
        game_rend -> Render(),
        separator(),
        right_box -> Render()
        }) | border;
    });

    auto receiver = MakeReceiver<std::string>();
    auto sender = receiver -> MakeSender();



    std::thread timer(repeating_timer, std::ref(drop_time), std::function<void(void)>(move_on_time), std::move(receiver));

    auto game = CatchEvent(screen_rend, [&](Event e) -> bool {
        if (e == Event::Escape) {
            sender -> Send("Kill");
            timer.join();
            screen.ExitLoopClosure()();
            return true;
        }
        if (e.is_character())
            brd.process_input(e.character());
        return false;
    });
    
    screen.Loop(game);

    return EXIT_SUCCESS;
}

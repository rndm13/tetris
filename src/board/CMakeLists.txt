add_library(board board.h board.cpp)

set_target_properties(board PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(board
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component 
)

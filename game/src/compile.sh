clear
emcc -o build/game.html main.cpp collision.cpp globals.cpp karen.cpp scroll.cpp include/engine.cpp include/entity.cpp include/screen.cpp -Os -Wall raylib/src/libraylib.a -I. -Iraylib/src/raylib.h -Iraylib/src/raudio.c -L. -Lraylib/src/libraylib-a -s USE_GLFW=3 -s ASYNCIFY -DPLATFORM_WEB --preload-file ../assets --shell-file build/smallshell.html -s -Wno-narrowing 
http-server
# 
# collision.cpp
# collision.hpp
# globals.cpp
# globals.hpp
# karen.cpp
# karen.hpp
# main.cpp
# scroll.cpp
# scroll.hpp
# ~/emsdk/upstream/emscripten/src/shell_minimal.html

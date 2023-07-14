clear
emcc -o game.html main.cpp include/engine.cpp include/entity.cpp include/screen.cpp -Os -Wall raylib/src/libraylib.a -I. -Iraylib/src/raylib.h -Iraylib/src/raudio.c -L. -Lraylib/src/libraylib-a -s USE_GLFW=3 -s ASYNCIFY -DPLATFORM_WEB --preload-file ../assets --shell-file smallshell.html -s -Wno-narrowing 
http-server
# ~/emsdk/upstream/emscripten/src/shell_minimal.html

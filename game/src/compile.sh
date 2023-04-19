emcc -o game.html main.cpp include/engine.cpp include/entity.cpp include/screen.cpp -Os -Wall /home/catto/raylib/src/libraylib.a -I. -I/home/catto/raylib/src/raylib.h -I/home/catto/raylib/src/raudio.c -L. -L/home/catto/raylib/src/libraylib-a -s USE_GLFW=3 -s ASYNCIFY -DPLATFORM_WEB --preload-file ../assets --shell-file smallshell.html -s -Wno-narrowing 
http-server
# ~/emsdk/upstream/emscripten/src/shell_minimal.html

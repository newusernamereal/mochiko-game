emcc -o game.html main.cpp include/engine.cpp include/entity.cpp include/screen.cpp -O3 -Wall /home/catto/raylib/src/libraylib.a -I. -I/home/catto/raylib/src/raylib.h -L. -L/home/catto/raylib/src/libraylib-a -s USE_GLFW=3 -s ASYNCIFY -DPLATFORM_WEB --preload-file ../assets 
http-server

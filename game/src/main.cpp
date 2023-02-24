#include "include/engine.hpp"
#include "include/entity.hpp"
#include "include/screen.hpp"
#include <iostream>
#include <emscripten/emscripten.h>

Screen currentScreen;

void UpdateDrawFrame(){
	BeginDrawing();
		DrawEntities();
	EndDrawing();
}

int main(){
	initEntityArr();
	Entity Player;
	InitWindow(SCREENX, SCREENY, "Mochiko Platformer");
	Player.addTexture(LoadTexture("../assets/down/top_1.png"));
	Player.addBox(EntityHitbox((Point) { 0, 0 }, 32, 32));
	currentScreen.entities.push_back(*Player.ent);
	currentScreen.ReadFromFile("../assets/Home.ce");
	currentScreen.Load();
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
    }
#endif
}

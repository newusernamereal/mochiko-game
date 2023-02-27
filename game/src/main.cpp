#include "include/engine.hpp"
#include "include/entity.hpp"
#include "include/screen.hpp"
#include <iostream>
#include <cmath>
#include <emscripten/emscripten.h>

#define SCROLL_DEBUG true

Screen currentScreen;
Entity player;
double yVelo = 0;
double xVelo = 0;

bool started = false;
bool dead = false;

const double GRAVITY = 1450; // downwards acceleration from gravity, in px/s^2
const double JUMPSTR = 700; // upwards (one time) velocity gain from jumping, in px/s
const double MOVESPD = 300; // sideways acceleration from moving, in px/s^2

const int TEXTUREBOX = 64; // size of the texture hitbox, in (px)^2 (not a square measurement)
const int COLLISIONBOXH = 60; // height of the collision hitbox, in px
const int COLLISIONBOXW = 50; // width of the collision hitbox in px

const double SCROLLSTR = 1; // how strong the scroll is, in tiles

void Shmove();
bool OutOfBounds(DoublePoint in);
bool OutOfBounds(EntityHitbox in);
bool OnGround();
bool OnGround(Point in);
bool InGround();
bool InGround(Point in);
void SnapToFloor();
bool WallAboveHead();
void DecelerateX();

void ChangeTexture();

void ScrollScreen(bool reset = false);
void MoveEverything(double dir);

void StartScreen();
void DeathScreen();
void InitGame();

void UpdateDrawFrame(){
	if(!started){
		StartScreen();
		return;
	}
	if (TriggerCollision(player) == 3)
		dead = true;
	if(dead){
		DeathScreen();
		return;
	}
	Shmove();
	ScrollScreen();
	player.hitboxes[1].pos = DoublePoint{ player.hitboxes[0].pos.x -  0.5 * (TEXTUREBOX - COLLISIONBOXW) , player.hitboxes[0].pos.y -  0.5 * (TEXTUREBOX - COLLISIONBOXH) };
	BeginDrawing();
		DrawEntities();
		currentScreen.Draw();
	EndDrawing();
}

int main(){
	initEntityArr();
	player.AddToGArry();
	InitWindow(SCREENX, SCREENY, "Mochiko Platformer");
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);
    while (!WindowShouldClose()){
        UpdateDrawFrame();
    }
#endif
}

void Shmove(){
	static bool bonked = false;
	static double timeOnGround = 0;
	bool movingSideways = false;
	// x stuff
	if(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)){
		xVelo = MOVESPD;
		movingSideways = true;
	}
	if(IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)){
		xVelo = (-1 * MOVESPD);
		movingSideways = true;
	}
	if (!movingSideways){
		DecelerateX();
	}
	EntityHitbox checkX(DoublePoint{player.hitboxes[0].pos.x + xVelo * GetFrameTime(), player.hitboxes[0].pos.y},COLLISIONBOXW,COLLISIONBOXH);
	if(currentScreen.CheckMove(checkX) && !OutOfBounds(checkX)){
		player.hitboxes[0].pos = DoublePoint{player.hitboxes[0].pos.x + xVelo * GetFrameTime(), player.hitboxes[0].pos.y};
	}
	// y stuff
	if(!OnGround()){
		yVelo -= (GRAVITY * GetFrameTime());
		timeOnGround = 0;
	}
	else{
		yVelo = 0;
		timeOnGround += GetFrameTime();
	}
	if((IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) && OnGround() && timeOnGround > 0.1){
		if(DEBUG)
			std::cout << "Jumped" << std::endl;
		yVelo = JUMPSTR;
	}
	if(InGround()){
		std::cout << "Mole" << std::endl;
		timeOnGround = 0;
		SnapToFloor();
	}
	else if (WallAboveHead()){
		if (!bonked){
			std::cout << "Bonk" << std::endl;
			bonked = true;
			yVelo *= -1;
		}
		player.hitboxes[0].pos = DoublePoint{ player.hitboxes[0].pos.x, player.hitboxes[0].pos.y - yVelo * GetFrameTime() };
	}
	else{
		bonked = false;
		player.hitboxes[0].pos = DoublePoint{ player.hitboxes[0].pos.x, player.hitboxes[0].pos.y - yVelo * GetFrameTime() };
	}
}

bool OutOfBounds(DoublePoint in){
	if(in.x > SCREENX || in.x < 0){
		return true;
	}
	if(in.y > SCREENY || in.y < 0){
		return true;
	}
	return false;
}

bool OutOfBounds(EntityHitbox in){
	if(OutOfBounds(in.pos))
		return true;
	if(OutOfBounds(DoublePoint{in.pos.x, in.pos.y + in.height}))
		return true;
	if(OutOfBounds(DoublePoint{in.pos.x + in.width, in.pos.y}))
		return true;
	if(OutOfBounds(DoublePoint{in.pos.x + in.width, in.pos.y + in.height}))
		return true;
	return false;
}

bool OnGround(){
	return ((player.hitboxes[0].pos.y + COLLISIONBOXH == SCREENY) || (!currentScreen.CheckMove(EntityHitbox(player.hitboxes[0].pos, COLLISIONBOXW,COLLISIONBOXH + 1))));
}

bool WallAboveHead(){
	return (!currentScreen.CheckMove(EntityHitbox(Point{ player.hitboxes[0].pos.x,player.hitboxes[0].pos.y - yVelo * GetFrameTime() }, COLLISIONBOXW , 1 )));
}

bool OnGround(Point in){
	return (in.y + COLLISIONBOXH == SCREENY) || (!currentScreen.CheckMove(EntityHitbox(in, COLLISIONBOXW,COLLISIONBOXH + 1)));
}

bool InGround(){
	return (player.hitboxes[0].pos.y + COLLISIONBOXH > SCREENY) || !(currentScreen.CheckMove(EntityHitbox(player.hitboxes[0].pos, COLLISIONBOXW,COLLISIONBOXH)));
}

bool InGround(Point in){
	return (in.y + COLLISIONBOXH > SCREENY) || (currentScreen.CheckMove(EntityHitbox(in, COLLISIONBOXW,COLLISIONBOXH)));
}

void SnapToFloor(){
	yVelo = 0;
	xVelo = 0;
	if(DEBUG)
		std::cout << "trying to snap\n";
	int searchUp = player.hitboxes[0].pos.y;
	int searchDown = player.hitboxes[0].pos.y;
	for(int i = 0; i < std::max(player.hitboxes[0].pos.y, SCREENY - player.hitboxes[0].pos.y); i++){
		// find the nearest floor, starting from the feet
		if(OnGround(Point{ player.hitboxes[0].pos.x , searchUp })){
			if(DEBUG)
				std::cout << "found a floor up: " << searchUp << "\n";
			player.hitboxes[0].pos.y = searchUp - 1;
			return;
		}
		if(OnGround(Point{ player.hitboxes[0].pos.x, searchDown })){
			if(DEBUG)
				std::cout << "found a floor down: " << searchDown << "\n";
			player.hitboxes[0].pos.y = searchDown + 1;
			return;
		}
		searchDown++;
		searchUp--;
	}
	// no floors on this x, should never get here because y = SCREENY is always a floor, but just in case
	player.hitboxes[0].pos.y = SCREENY - COLLISIONBOXH - 1;
}

void DecelerateX(){
	//VV i'm so fucking tired and i have no clue why but making this do anything but just set the velocity to 0 will make going right slower and going left faster and im so fucking tired man why does it do this
	xVelo	= 0;
/*	if (std::abs(xVelo) - DECELERATESPD * 0.2) // 0.2 is literally just arbitrary, but i don't think anyone will miss 0.2 seconds of deceleration
		xVelo = 0;
	if(xVelo == 0){
		return;
	}
	if(xVelo < 0){
		xVelo += DECELERATESPD * GetFrameTime();
		return;
	}
	if (xVelo > 0){
		xVelo -= DECELERATESPD * GetFrameTime();
		return;
	}
//	xVelo += DECELERATESPD * GetFrameTime(); */
}

void ScrollScreen(bool reset){
	static double pos = 0;
	if (reset){
		pos = 0;
	}
	if(TriggerCollision(player) == 1 && pos + SCROLLSTR <= currentScreen.width - currentScreen.size){ // player is on the right border; move everything left
		if(DEBUG)
			std::cout << "Scrolling left because " << pos + SCROLLSTR << " <= " << currentScreen.width - currentScreen.size << std::endl;
		pos += SCROLLSTR;
		MoveEverything(-1 * SCROLLSTR);
		return;
	}
	if(TriggerCollision(player) == 2 && pos - SCROLLSTR <= currentScreen.width - currentScreen.size && pos - SCROLLSTR >= 0){ // player is on the left border; move everything right
		if(DEBUG)
			std::cout << "Scrolling right because " << pos - SCROLLSTR << " <= " << currentScreen.width - currentScreen.size << std::endl;
		pos -= SCROLLSTR;
		MoveEverything(SCROLLSTR);
		return;
	}
}

void MoveEverything(double dir){
	for(int i = 0; i < LOADED_ENTITIES_HEAD; i++){
		if(!LOADED_ENTITIES[i].triggerID){
			for(int k = 0; k < LOADED_ENTITIES[i].hitboxes.size(); k++){
				LOADED_ENTITIES[i].hitboxes[k].pos.x += dir * currentScreen.tileSizeX;
				if (SCROLL_DEBUG)
					std::cout << "Moving hitbox " << k << " of entity " << i << " to " << LOADED_ENTITIES[i].hitboxes[k].pos.x << " (" << dir * currentScreen.tileSizeX << ") pixels" << std::endl;
			}
		}
	}
	for(int i = 0; i < currentScreen.barriers.size(); i++){
		for(int k = 0; k < currentScreen.barriers[i].hitboxes.size(); k++){
			currentScreen.barriers[i].hitboxes[k].pos.x += dir * currentScreen.tileSizeX;
			if (SCROLL_DEBUG)
				std::cout << "Moving hitbox " << k << " of barrier " << i << " to " << currentScreen.barriers[i].hitboxes[k].pos.x << " (" << dir * currentScreen.tileSizeX << ") pixels" << std::endl;
		}
	}
}

void StartScreen(){
	static Entity startText;
	static double anim = 0;
	static int directionCounter = 0;
	static bool up = true;
	static bool init = false;
	if (!init){
		startText.addTexture(LoadTexture("assets/start.png"));
		startText.addBox(EntityHitbox(Point{74,170},492,300));
		init = true;
	}
	if(IsKeyPressed(KEY_SPACE)){
		InitGame();
		started = true;
		return;
	}
	anim += GetFrameTime();
	if(anim >= 0.2){
		anim = 0;
		directionCounter++;
		if(up){
			startText.hitboxes[0].pos.y -= 5;
		}
		else{
			startText.hitboxes[0].pos.y += 5;
		}
	}
	if(directionCounter > 5){
		directionCounter = 0;
		up = !up;
	}
	BeginDrawing();
		ClearBackground(WHITE);
		DrawEntity(startText);
	EndDrawing();
}

void DeathScreen(){
	static Entity deathText;
	static double anim = 0;
	static bool up = true;
	static int directionCounter = 0;
	static bool init = false;
	static Texture2D text = LoadTexture("assets/death.png");
	if (!init){
		deathText.addTexture(text);
		deathText.addBox(EntityHitbox(Point{128,170},384,300));
		init = true;
	}
	if(IsKeyPressed(KEY_SPACE)){
		InitGame();
		dead = false;
		anim = 0;
		up = true;
		directionCounter = 0;
		init = false;
		return;
	}
	anim += GetFrameTime();
	if(anim >= 0.2){
		anim = 0;
		directionCounter++;
		if(up){
			deathText.hitboxes[0].pos.y -= 5;
		}
		else{
			deathText.hitboxes[0].pos.y += 5;
		}
	}
	if(directionCounter > 5){
		directionCounter = 0;
		up = !up;
	}
	BeginDrawing();
		ClearBackground(WHITE);
		DrawEntity(deathText);
	EndDrawing();
}

void InitGame(){
	if(!started){
		player.addTexture(LoadTexture("assets/transparent.png")); // transparent texture for the collision box
		player.addTexture(LoadTexture("assets/chicken_right.png")); // actual texture for the drawing box
		player.addBox(EntityHitbox((DoublePoint) { 0.5 * (TEXTUREBOX - COLLISIONBOXW), 0.5 * (TEXTUREBOX - COLLISIONBOXH) }, COLLISIONBOXW, COLLISIONBOXH)); // collision box
		player.addBox(EntityHitbox((DoublePoint) { 0, 0 }, TEXTUREBOX, TEXTUREBOX)); // drawing box
		currentScreen.entities.push_back(*player.ent);
		currentScreen.ReadFromFile("assets/LevelOne.ce");
	}
	clearEntitiesExceptFirst();
	currentScreen.entities.clear();
	currentScreen.entities.push_back(*player.ent);
	currentScreen.ReadFromFile("assets/LevelOne.ce");
	currentScreen.Load();
	player.hitboxes[0].pos.x += 0.5 * (TEXTUREBOX - COLLISIONBOXW);
	player.hitboxes[0].pos.y += 0.5 * (TEXTUREBOX - COLLISIONBOXH);
	currentScreen.backgroundTint = RED;
	ScrollScreen(true);
}

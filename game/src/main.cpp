#include "include/screen.hpp"
#include <iostream>
#include <cmath>
#include <emscripten/emscripten.h>

#define SCROLL_DEBUG false

Screen currentScreen;
Entity player;
double yVelo = 0;
double xVelo = 0;

bool started = false;
bool dead = false;

int AIDIFFICULTY = 1; // AI difficulty, affects movespeed and rate of fire

const double GRAVITY = 1450; // downwards acceleration from gravity, in px/s^2
const double JUMPSTR = 400; // upwards (one time) velocity gain from jumping, in px/s
const int MOVESPEED = 330; // sideways velocity from moving, in px/second

const int TEXTUREBOX = 64; // size of the texture hitbox, in (px)^2 (not a square measurement)
const int COLLISIONBOXH = 60; // height of the collision hitbox, in px
const int COLLISIONBOXW = 36; // width of the collision hitbox in px

double SCROLLSTR = 1; // how strong the scroll is, in tiles/frame

int SIGNFONTSIZE = 20; // how big the sign's font size is, in px
int SIGNELEVATION = 150; // how high the sign text appears above the sign, in px

int MOVESPD = MOVESPEED; // movespeed * DT rounded to the nearest integer because fractions don't play nice. might cause problems if people use crazy high (360hz+) screens but i really don't care

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
void _MoveSideways();

void ChangeTexture(bool right);

void ScrollScreen(bool reset = false);
void MoveEverything(double dir);

void ReadSign();
void FixTextures();

void StartScreen();
void DeathScreen();
void InitGame();

void UpdateKarens();
void KarenAI(int ent);
void AttackPlayer(int source);

void UpdateDrawFrame(){
	MOVESPD = (MOVESPEED * GetFrameTime());
	SCROLLSTR = (MOVESPD / currentScreen.tileSizeX);
	UpdateKarens();
	MoveEntities();
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
		ReadSign();
		currentScreen.Draw();
	EndDrawing();
}

int main(){
	ChangeDirectory("../");
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

void Shmove(){ // not proud of this one
	static bool bonked = false;
	static double timeOnGround = 0;
	bool movingSideways = false;
	bool jumping = false;
	// y stuff
	if(!OnGround()){
		timeOnGround = 0;
	}
	if((IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) && OnGround() && timeOnGround){
		if(DEBUG)
			std::cout << "Jumped" << std::endl;
		yVelo = JUMPSTR;
		timeOnGround = 0;
		jumping = true;
	}
	else if (WallAboveHead()){
		if (!bonked){
			if(DEBUG)
				std::cout << "Bonk" << std::endl;
			bonked = true;
			yVelo *= -1;
		}
	}
	else{
		bonked = false;
	}	
	if(!OnGround()){
		yVelo -= (GRAVITY * GetFrameTime());
		timeOnGround = 0;
	}
	else{
		timeOnGround += GetFrameTime();
	}
	player.hitboxes[0].pos = DoublePoint{ player.hitboxes[0].pos.x, player.hitboxes[0].pos.y - yVelo * GetFrameTime() };
	while(InGround()){
		player.hitboxes[0].pos = DoublePoint{ player.hitboxes[0].pos.x, player.hitboxes[0].pos.y - 1 };
	}
	if(OnGround()){
		yVelo = 0;
	}
	// x stuff
	if(IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)){
		xVelo = (-1 * MOVESPD);
		movingSideways = true;
		_MoveSideways();
	}
	if(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)){
		xVelo = MOVESPD;
		if(IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
			movingSideways = true;
		_MoveSideways();
	}
	if (!movingSideways){
		DecelerateX();
	}
	if(movingSideways){
		xVelo < 0 ? ChangeTexture(false) : ChangeTexture(true);
	}
}

inline bool OutOfBounds(DoublePoint in){
	if(in.x > SCREENX || in.x < 0){
		return true;
	}
	if(in.y > SCREENY || in.y < 0){
		return true;
	}
	return false;
}

inline bool OutOfBounds(EntityHitbox in){
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

inline bool OnGround(){
	return (player.hitboxes[0].pos.y + COLLISIONBOXH == SCREENY) || (!currentScreen.CheckMove(EntityHitbox(DoublePoint{player.hitboxes[0].pos.x,player.hitboxes[0].pos.y + COLLISIONBOXH}, COLLISIONBOXW,1)));
}

inline bool WallAboveHead(){
	return (!currentScreen.CheckMove(EntityHitbox(Point{ player.hitboxes[0].pos.x,player.hitboxes[0].pos.y - yVelo * GetFrameTime() }, COLLISIONBOXW , 1 )));
}

inline bool OnGround(Point in){
	return (in.y + COLLISIONBOXH == SCREENY) || (!currentScreen.CheckMove(EntityHitbox(DoublePoint{in.x,in.y + COLLISIONBOXH}, COLLISIONBOXW,1)));
}

inline bool InGround(){
	return (player.hitboxes[0].pos.y + COLLISIONBOXH > SCREENY) || !(currentScreen.CheckMove(EntityHitbox(player.hitboxes[0].pos, COLLISIONBOXW,COLLISIONBOXH)));
}

inline bool InGround(Point in){
	return (in.y + COLLISIONBOXH > SCREENY) || (currentScreen.CheckMove(EntityHitbox(in, COLLISIONBOXW,COLLISIONBOXH)));
}

void SnapToFloor(){
	yVelo = 0;
	xVelo = 0;
	player.hitboxes[0].pos.y -= currentScreen.tileSizeY;
	if(DEBUG)
		std::cout << "trying to snap\n";
	int searchUp = player.hitboxes[0].pos.y;
	int searchDown = player.hitboxes[0].pos.y;
	for(int i = 0; i < std::max(player.hitboxes[0].pos.y, SCREENY - player.hitboxes[0].pos.y); i++){
		// find the nearest floor, starting from the feet
		if(OnGround(Point{ player.hitboxes[0].pos.x , searchUp }) && !InGround(Point{ player.hitboxes[0].pos.x , searchUp })){
			std::cout << "dsa" << std::endl;
			if(DEBUG)
				std::cout << "found a floor up: " << searchUp << "\n";
			player.hitboxes[0].pos.y = searchUp;
			return;
		}
		if(OnGround(Point{ player.hitboxes[0].pos.x, searchDown })){
			if(DEBUG)
				std::cout << "found a floor down: " << searchDown << "\n";
			player.hitboxes[0].pos.y = searchDown;
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
	const double upperBound = currentScreen.width - ((double)SCREENX/(double)currentScreen.tileSizeX);
	if (reset){
		pos = 0;
	}
	if(TriggerCollision(player) == 1 && pos + SCROLLSTR <= upperBound){ // player is on the right border; move everything left
		if(SCROLL_DEBUG)
			std::cout << "Scrolling left because " << pos + SCROLLSTR << " <= " << upperBound << std::endl;
		pos += SCROLLSTR;
		MoveEverything(-1 * SCROLLSTR);
		return;
	}
	if(TriggerCollision(player) == 2 && pos - SCROLLSTR <= upperBound && pos - SCROLLSTR >= 0){ // player is on the left border; move everything right
		if(SCROLL_DEBUG)
			std::cout << "Scrolling right because " << pos - SCROLLSTR << " <= " << upperBound << std::endl;
		pos -= SCROLLSTR;
		MoveEverything(SCROLLSTR);
		return;
	}
}

void MoveEverything(double dir){
	for(int i = 0; i < LOADED_ENTITIES_HEAD; i++){
		if(LOADED_ENTITIES[i].triggerID != 1 && LOADED_ENTITIES[i].triggerID != 2){
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
	static int frameCounter = 0;
	static bool up = true;

	static bool init = false;
	if (!init){
		startText.addTexture(LoadTexture("assets/start.png"));
		startText.addBox(EntityHitbox(Point{266,138},492,300));
		init = true;
	}
	if(IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)){
		InitGame();
		started = true;
		return;
	}
	anim += GetFrameTime();
	if(anim >= 0.2){
		anim = 0;
		frameCounter++;
		if(up){
			startText.hitboxes[0].pos.y -= 5;
		}
		else{
			startText.hitboxes[0].pos.y += 5;
		}
	}
	if(frameCounter > 5){
		frameCounter = 0;
		up = !up;
	}
	BeginDrawing();
		ClearBackground(WHITE);
		DrawEntity(startText);
	EndDrawing();
}

void DeathScreen(){
	static Entity deathText;
	static Entity plosion;
	static Texture2D text = LoadTexture("assets/death.png");
	static Texture2D plosionText = LoadTexture("assets/ploded.png");

	static double anim = 0;
	static int frameCounter = 0;
	static double frameTime = 0.2f;

	static bool up = true;
	static bool init = false;
	static bool alive = false;
	static bool finishedAnim = false;
	if (!init){
		deathText.addTexture(text);
		plosion.addTexture(plosionText);
		deathText.addBox(EntityHitbox(Point{320,138},384,300));
		plosion.addBox(EntityHitbox(DoublePoint{ player.hitboxes[1].pos.x - 32, player.hitboxes[1].pos.y - 32 },128,128));
		deathText.DontDraw(true);
		player.DontDraw(true);
		init = true;
	}
	if (!alive){
		plosion.hitboxes[0] = (EntityHitbox(DoublePoint{ player.hitboxes[1].pos.x - 32, player.hitboxes[1].pos.y - 32 },128,128));
	}
	if(!finishedAnim){
		alive = true;
		anim += GetFrameTime();
		if (anim > frameTime){
			frameCounter++;
			anim = 0;
		}
		if (frameCounter > 3){
			finishedAnim = true;
			deathText.DontDraw(false);
			player.DontDraw(false);
			anim = 0;
			frameCounter = 0;
			plosion.Offset()->x = 0;
		}
		plosion.Offset()->x = 128 * frameCounter;
		BeginDrawing();
			DrawEntities();
			DrawEntity(plosion);
			currentScreen.Draw();
		EndDrawing();
		return;
	}
	if(IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)){
		InitGame();
		dead = false;
		alive = true;
		frameCounter = 0;
		anim = 0;
		up = true;
		init = false;
		finishedAnim = false;
		return;
	}
	anim += GetFrameTime();
	if(anim >= frameTime){
		anim = 0;
		frameCounter++;
		if(up){
			deathText.hitboxes[0].pos.y -= 5;
		}
		else{
			deathText.hitboxes[0].pos.y += 5;
		}
	}
	if(frameCounter > 5){
		frameCounter = 0;
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
		player.addTexture(LoadTexture("assets/chicken.png")); // actual texture for the drawing box
		player.addBox(EntityHitbox((DoublePoint) { 0.5 * (TEXTUREBOX - COLLISIONBOXW), 0.5 * (TEXTUREBOX - COLLISIONBOXH) }, COLLISIONBOXW, COLLISIONBOXH)); // collision box
		player.addBox(EntityHitbox((DoublePoint) { 0, 0 }, TEXTUREBOX, TEXTUREBOX)); // drawing box
		currentScreen.entities.push_back(*player.ent);
		currentScreen.ReadFromFile("assets/LevelOne.ce");
	}
	clearEntitiesExceptFirst();
	currentScreen.entities.clear();
	currentScreen.entities.push_back(*player.ent);

	currentScreen.ReadFromFile(currentScreen.fileName);
	currentScreen.Load();
	FixTextures();
	player.hitboxes[0].pos.x += 0.5 * (TEXTUREBOX - COLLISIONBOXW);
	player.hitboxes[0].pos.y += 0.5 * (TEXTUREBOX - COLLISIONBOXH);
	currentScreen.backgroundTint = RED;
	ScrollScreen(true);
	//if(started)
		AttackPlayer(-1);
}

void ChangeTexture(bool right){
	if(right){
		player.Offset()->x = 0;
		return;
	}
	player.Offset()->x = 64;
}

void _MoveSideways(){
	EntityHitbox checkX(DoublePoint{player.hitboxes[0].pos.x + xVelo, player.hitboxes[0].pos.y},COLLISIONBOXW,COLLISIONBOXH);
	if(currentScreen.CheckMove(checkX) && !OutOfBounds(checkX)){
		player.hitboxes[0].pos = DoublePoint{player.hitboxes[0].pos.x + xVelo, player.hitboxes[0].pos.y};
	}
}

void ReadSign(){
	for(int i = 0; i < LOADED_ENTITIES_HEAD; i++){
		if (LOADED_ENTITIES[i].signText != ""){
			if (LOADED_ENTITIES[i].Colliding(player.hitboxes[0])){
				DrawText(LOADED_ENTITIES[i].signText.c_str(),
				LOADED_ENTITIES[i].hitboxes[0].pos.x - (0.25 * MeasureText(LOADED_ENTITIES[i].signText.c_str(), SIGNFONTSIZE)) ,
				LOADED_ENTITIES[i].hitboxes[0].pos.y - SIGNELEVATION, SIGNFONTSIZE , WHITE);
			}
			//std::cout << LOADED_ENTITIES[i].signText << std::endl;
		}
	}
}

void FixTextures(){
	for(int i = 0; i < LOADED_ENTITIES_HEAD; i++){
			if(LOADED_ENTITIES[i].triggerID >= 9 && LOADED_ENTITIES[i].trigger && !(LOADED_ENTITIES[i].triggerID % 9)){
				if(LOADED_ENTITIES[i].triggerID > 9)
					LOADED_ENTITIES[i].triggerID /= 9;
				for(int k = 0; k < LOADED_ENTITIES[i].hitboxes.size(); k++){
						LOADED_ENTITIES[i].hitboxes[k].width = LOADED_ENTITIES[i].hitboxTexts[k].width;
						LOADED_ENTITIES[i].hitboxes[k].height = LOADED_ENTITIES[i].hitboxTexts[k].height;
				}
			}
	}
}

void UpdateKarens(){
	
	for(int i = 0; i < LOADED_ENTITIES_HEAD; i++){
		if(LOADED_ENTITIES[i].triggerID == 10){
			KarenAI(i);
		}
	}
}

void KarenAI(int ent){
	static bool detected = false;
	static double timeSinceLastProj = 0;
	static double chaseTimer = 0;
	// detect player
	// if one of them detects the player, all of them see the player
	// it's a feature, not a bug
	chaseTimer += GetFrameTime();
	timeSinceLastProj += GetFrameTime();
	if(player.Colliding(LOADED_ENTITIES[ent])){
		chaseTimer = 0;
	}
	detected = (chaseTimer <= 0.5);
	if(!detected)
		return;
	// move to the player
	if(timeSinceLastProj >= (double)AIDIFFICULTY * 0.5f ){
		if(DEBUG)
			std::cout << "Attacked player" << " , " << LOADED_ENTITIES_HEAD << std::endl;
		timeSinceLastProj = 0;
		AttackPlayer(ent);
	}
	
}

void AttackPlayer(int source){
	static Texture2D badWords = LoadTexture("assets/bad_words.png");
	static Entity proj[30];
	static int i = 0;
	if(source == -1){
		std::cout << "DLSLADSKA" << std::endl;
		for(int k = 0; k < 30; k++){
			proj[k].ent->hitboxes[0].pos = {-10000,-10000};
			proj[k].AddToGArry();
		}
	}
	for(int i = 0; i < 30; i++)
		proj[i].ent->triggerID = 3;
	proj[i].ent->hitboxes.clear();
	proj[i].ent->hitboxTexts.clear();
	proj[i].addBox(EntityHitbox(LOADED_ENTITIES[source].hitboxes[0].pos.x + 45, LOADED_ENTITIES[source].hitboxes[0].pos.y + 80, currentScreen.tileSizeX,currentScreen.tileSizeY));
	if(player.hitboxes[0].pos.x == LOADED_ENTITIES[source].hitboxes[0].pos.x){
		proj[i].hitboxes[0].speed = {0,-1};
	}else{
	proj[i].hitboxes[0].speed = {
		// ((x2 - x1) / |x2 - x1|) * cos(tan^-1((y2 - y1) / (x2 - x1)))
		-1.0f * MOVESPD * AIDIFFICULTY * (((LOADED_ENTITIES[source].hitboxes[0].pos.x + 45) - player.hitboxes[0].pos.x) / std::abs((LOADED_ENTITIES[source].hitboxes[0].pos.x + 45) - player.hitboxes[0].pos.x)) * std::cos(std::atan2((player.hitboxes[0].pos.y - (LOADED_ENTITIES[source].hitboxes[0].pos.y + 80)) / (player.hitboxes[0].pos.x - (LOADED_ENTITIES[source].hitboxes[0].pos.x + 45)),1)) ,
		// ((x2 - x1) / |x2 - x1|) * sin(tan^-1((y2 - y1) / (x2 - x1)))
		-1.0f * MOVESPD * AIDIFFICULTY * (((LOADED_ENTITIES[source].hitboxes[0].pos.x + 45) - player.hitboxes[0].pos.x) / std::abs((LOADED_ENTITIES[source].hitboxes[0].pos.x + 45) - player.hitboxes[0].pos.x)) * std::sin(std::atan2((player.hitboxes[0].pos.y - (LOADED_ENTITIES[source].hitboxes[0].pos.y + 80)) / (player.hitboxes[0].pos.x - (LOADED_ENTITIES[source].hitboxes[0].pos.x + 45)),1))
	}; }
	proj[i].addTexture(badWords);
	i++;
	if(i == 30){
		i = 0;
	}
}


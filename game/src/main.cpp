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

const double GRAVITY = 1300; // downwards acceleration from gravity, in px/s^2
const double JUMPSTR = 400; // upwards (one time) velocity gain from jumping, in px/s
const int MOVESPEED = 400; // sideways velocity from moving, in px/second

const int TEXTUREBOX = 64; // size of the texture hitbox, in (px)^2 (not a square measurement)
const int COLLISIONBOXH = 60; // height of the collision hitbox, in px
const int COLLISIONBOXW = 36; // width of the collision hitbox in px

double SCROLLSTR = 1; // how strong the scroll is, in tiles/sec

int SIGNFONTSIZE = 20; // how big the sign's font size is, in px
int SIGNELEVATION = 150; // how high the sign text appears above the sign, in px

int MOVESPD = MOVESPEED; // movespeed * DT rounded to the nearest integer because fractions don't play nice. might cause problems if people use crazy high (360hz+) screens but i really don't care

bool HAS_KEY = false;
int COINS = 0; // idk?? fun

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

void ChangeTexture(bool right, bool flying = false);

void ScrollScreen(bool reset = false);
void MoveEverything(double dir);

void ReadSign();
void FixTextures();

void StartScreen();
void DeathScreen();
void InitGame();

void InitKarens();
void UpdateKarens();
void KarenAI(int ent);
void AttackPlayer(int source);

bool CollisionsContain(int x, bool update = false);
bool CollisionsContain(EntityContainer ent, int x, bool update = false);

void PickUpKey();
void PickUpCoin();
void UseDoor(bool reset = false);

void NextLevel();

void SwitchMusic(Music& currentMusic);

void UpdateDrawFrame(){
	MOVESPD = (MOVESPEED * GetFrameTime());
	SCROLLSTR = (MOVESPD / currentScreen.tileSizeX);
	static Texture2D hudKey = LoadTexture("assets/still_key.png");
	static Texture2D noKeys = LoadTexture("assets/keyless.png");
	static Music music;
	static std::string world = "";
	if(!IsAudioDeviceReady()){
		InitAudioDevice();
	}
	UpdateKarens();
	MoveEntities();
	PickUpKey();
	PickUpCoin();
	UseDoor();
	if(!started){
		StartScreen();
		return;
	}
	Shmove();
	if (CollisionsContain(3,true))
		dead = true;
	
	if (CollisionsContain(4))
		HAS_KEY = true;
	
	if(dead){
		SeekMusicStream(music,0.0f);
		DeathScreen();
		return;
	}
	if(world != currentScreen.fileName)
		SwitchMusic(music);
	world = currentScreen.fileName;
	UpdateMusicStream(music);
	ScrollScreen();
	player.hitboxes[1].pos = DoublePoint{ player.hitboxes[0].pos.x -  0.5 * (TEXTUREBOX - COLLISIONBOXW) , player.hitboxes[0].pos.y -  0.5 * (TEXTUREBOX - COLLISIONBOXH) };
	BeginDrawing();
		currentScreen.Draw();
		DrawEntities();
		DrawTextureRec(HAS_KEY ? hudKey : noKeys,{0,0,64,64},{0,0},WHITE);
		ReadSign();
	EndDrawing();
}

int main(){
	ChangeDirectory("../");
	initEntityArr();
	player.AddToGArry();
	InitWindow(SCREENX, SCREENY, "Mochiko Platformer");
#if defined(__EMSCRIPTEN__)
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
	static bool lastMoveRight = false;
	bool movingSideways = false;
	// y stuff
	if(!OnGround()){
		timeOnGround = 0;
	}
	if((IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) && OnGround() && timeOnGround){
		if(DEBUG)
			std::cout << "Jumped" << std::endl;
		yVelo = JUMPSTR;
		timeOnGround = 0;
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
		ChangeTexture(lastMoveRight,true);
	}
	else{
		ChangeTexture(lastMoveRight,false);
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
		movingSideways = !(IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A));
		_MoveSideways();
	}
	if (!movingSideways){
		DecelerateX();
	}
	if(movingSideways){
		xVelo < 0 ? ChangeTexture(false, !OnGround()) : ChangeTexture(true, !OnGround());
		lastMoveRight = xVelo > 0;
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
	return (!currentScreen.CheckMove(EntityHitbox(Point{ player.hitboxes[0].pos.x,player.hitboxes[0].pos.y - yVelo * GetFrameTime() }, COLLISIONBOXW , 1 )) || (player.hitboxes[0].pos.y - yVelo * GetFrameTime()) < 0);
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
	static Entity screenAnchor;
	static bool init = false;
	const double upperBound = currentScreen.width - ((double)SCREENX/(double)currentScreen.tileSizeX);
	if(!init){
		screenAnchor.addBox(EntityHitbox(0,0,1,1));
		screenAnchor.addTexture(LoadTexture("assets/transparent.png"));
		init = true;
	}
	if (reset){
		pos = 0;
		screenAnchor.AddToGArry();
		screenAnchor.addBox(EntityHitbox(0,0,1,1));
		screenAnchor.DontDraw(true);
		currentScreen.offset.x = screenAnchor.hitboxes[0].pos.x; 
	}
	if(CollisionsContain(1) && pos + SCROLLSTR <= upperBound){ // player is on the right border; move everything left
		if(SCROLL_DEBUG)
			std::cout << "Scrolling left because " << pos + SCROLLSTR << " <= " << upperBound << std::endl;
		pos += SCROLLSTR;
		MoveEverything(-1 * SCROLLSTR);
		currentScreen.offset.x = screenAnchor.hitboxes[0].pos.x; // quick and dirty
		return;
	}
	if(CollisionsContain(2) && pos - SCROLLSTR <= upperBound && pos - SCROLLSTR >= 0){ // player is on the left border; move everything right
		if(SCROLL_DEBUG)
			std::cout << "Scrolling right because " << pos - SCROLLSTR << " <= " << upperBound << std::endl;
		pos -= SCROLLSTR;
		MoveEverything(SCROLLSTR);
		currentScreen.offset.x = screenAnchor.hitboxes[0].pos.x; 
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
	static Music startMusic;
	static double anim = 0;
	static int frameCounter = 0;
	static bool up = true;

	static bool init = false;
	if (!init){
		startMusic = LoadMusicStream("assets/start_screen.wav");
		SeekMusicStream(startMusic,0.0f);
		SetMusicVolume(startMusic, 0.3);
		PlayMusicStream(startMusic);
		startText.addTexture(LoadTexture("assets/start.png"));
		startText.addBox(EntityHitbox(Point{266,138},492,300));
		init = true;
	}
	if(IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)){
		StopMusicStream(startMusic);
		InitGame();
		started = true;
		return;
	}
	anim += GetFrameTime();
	UpdateMusicStream(startMusic);
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
	static Music deathMusic; 
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
		deathMusic = LoadMusicStream("assets/death_screen.wav");  
		deathText.AddToGArry();
		plosion.AddToGArry();
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
			SeekMusicStream(deathMusic,0.0f);
			PlayMusicStream(deathMusic);
			finishedAnim = true;
			deathText.DontDraw(false);
			player.DontDraw(false);
			anim = 0;
			frameCounter = 0;
			plosion.Offset()->x = 0;
		}
		plosion.Offset()->x = 128 * frameCounter;
		BeginDrawing();
			currentScreen.Draw();
			DrawEntities();
			DrawEntity(plosion);
		EndDrawing();
		return;
	}
	if(IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)){
		StopMusicStream(deathMusic);
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
	UpdateMusicStream(deathMusic);
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

void InitGame(){ // i'm really tired
	static Entity leftScroll;
	static Entity rightScroll;
	static Texture2D sky = LoadTexture("assets/sky.png");
	static Texture2D night = LoadTexture("assets/night.png");
	if(!started){
		player.ent->anim = true;
		player.ent->animOffset = {0,64};
		player.ent->fps = 5;
		player.ent->frames = 2;
		leftScroll.addBox(EntityHitbox(0,0,64,SCREENY));
		leftScroll.ent->triggerID = 2;
		leftScroll.ent->trigger = true;
		leftScroll.ent->dontDraw = false;
		rightScroll.addBox(EntityHitbox(SCREENX - (64 * 11),0,64,SCREENY));
		rightScroll.ent->triggerID = 1;
		rightScroll.ent->trigger = true;
		rightScroll.ent->dontDraw = false;
		player.addTexture(LoadTexture("assets/transparent.png")); // transparent texture for the collision box
		player.addTexture(LoadTexture("assets/chicken.png")); // actual texture for the drawing box
		player.addBox(EntityHitbox((DoublePoint) { 0.5 * (TEXTUREBOX - COLLISIONBOXW), 0.5 * (TEXTUREBOX - COLLISIONBOXH) }, COLLISIONBOXW, COLLISIONBOXH)); // collision box
		player.addBox(EntityHitbox((DoublePoint) { 0, 0 }, TEXTUREBOX, TEXTUREBOX)); // drawing box
		currentScreen.entities.push_back(*player.ent);
		currentScreen.entities.push_back(*leftScroll.ent);
		currentScreen.entities.push_back(*rightScroll.ent);
		currentScreen.ReadFromFile("assets/LevelOne.ce");
	}
	currentScreen.entities.clear();
	currentScreen.entities.push_back(*player.ent);
	currentScreen.ReadFromFile(currentScreen.fileName);
	currentScreen.Load();
	leftScroll.AddToGArry();
	rightScroll.AddToGArry();
	leftScroll.addBox(EntityHitbox(0,0,64,SCREENY));
	leftScroll.ent->triggerID = 2;
	leftScroll.ent->trigger = true;
	leftScroll.ent->dontDraw = false;
	rightScroll.addBox(EntityHitbox(SCREENX - (64 * 11),0,64,SCREENY));
	rightScroll.ent->triggerID = 1;
	rightScroll.ent->trigger = true;
	rightScroll.ent->dontDraw = false;
	FixTextures();
	currentScreen.backgroundIsText = true;
	if(currentScreen.fileName == "assets/LevelOne.ce"){
		currentScreen.background = sky;
		currentScreen.anim = false;
	}
	if(currentScreen.fileName == "assets/LevelTwo.ce"){
		currentScreen.background = night;
		currentScreen.anim = true;
		currentScreen.animFps = 2;
		currentScreen.frames = 2;
		currentScreen.animOffset = {0,-576};
	}
	player.hitboxes[0].pos.x += 0.5 * (TEXTUREBOX - COLLISIONBOXW);
	player.hitboxes[0].pos.y += 0.5 * (TEXTUREBOX - COLLISIONBOXH);
	currentScreen.backgroundTint = WHITE;
	ScrollScreen(true);
	//if(started)
		AttackPlayer(-1);
	UseDoor(true);
	if(FindTrigger(5).has_value())
		LOADED_ENTITIES[FindTrigger(5).value()].anim = false;
}

void ChangeTexture(bool right, bool flying){
	if(right){
		player.Offset()->x = 0 + (flying ? 128 : 0);
		return;
	}
	player.Offset()->x = 64 + (flying ? 128 : 0);
}

void _MoveSideways(){
	EntityHitbox checkX(DoublePoint{player.hitboxes[0].pos.x + xVelo, player.hitboxes[0].pos.y},COLLISIONBOXW,COLLISIONBOXH);
	if(!currentScreen.CheckMove(checkX) || OutOfBounds(checkX)){
		return;
	}
	player.hitboxes[0].pos = DoublePoint{player.hitboxes[0].pos.x + xVelo, player.hitboxes[0].pos.y};
}

void ReadSign(){
	for(int i = 0; i < LOADED_ENTITIES_HEAD; i++){
		if (LOADED_ENTITIES[i].signText != ""){
			if (LOADED_ENTITIES[i].Colliding(player.hitboxes[0])){
				DrawText(LOADED_ENTITIES[i].signText.c_str(),
				LOADED_ENTITIES[i].hitboxes[0].pos.x - (0.25 * MeasureText(LOADED_ENTITIES[i].signText.c_str(), SIGNFONTSIZE)) ,
				LOADED_ENTITIES[i].hitboxes[0].pos.y - SIGNELEVATION, SIGNFONTSIZE , WHITE);
			}
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
			if(LOADED_ENTITIES[i].trigger && !(LOADED_ENTITIES[i].triggerID % 7)){ //fix door
				for(int k = 0; k < LOADED_ENTITIES[i].hitboxes.size(); k++){
						LOADED_ENTITIES[i].hitboxes[k].width = LOADED_ENTITIES[i].hitboxTexts[k].width / 5;
						LOADED_ENTITIES[i].hitboxes[k].height = LOADED_ENTITIES[i].hitboxTexts[k].height;
				}
				LOADED_ENTITIES[i].animOffset = {LOADED_ENTITIES[i].hitboxTexts[0].width / 5, 0};
				LOADED_ENTITIES[i].fps = 5;
				LOADED_ENTITIES[i].frames = 4;
				LOADED_ENTITIES[i].triggerID /= 7;
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
	static double chaseTimer = 1;
	// detect player
	// if one of them detects the player, all of them see the player
	// it's a feature, not a bug
	chaseTimer += GetFrameTime();
	timeSinceLastProj += GetFrameTime();
	if(std::abs(LOADED_ENTITIES[ent].hitboxes[0].pos.x - player.hitboxes[0].pos.x) <= 3 * currentScreen.tileSizeX){
		chaseTimer = 0;
	}
	if (dead)
		chaseTimer = 1;
	detected = (0.5 >= chaseTimer);
	if(!detected)
		return;
	// move to the player
	if(timeSinceLastProj >= (double)(10.0f / AIDIFFICULTY) * 0.5f ){
		if(DEBUG)
			std::cout << "Attacked player" << " , " << LOADED_ENTITIES_HEAD << std::endl;
		timeSinceLastProj = 0;
		AttackPlayer(ent);
	}
	
}

void AttackPlayer(int source){
/*	static Texture2D badWords = LoadTexture("assets/bad_words.png");
	static Entity proj[30];
	static int i = 0;
	if(source == -1){
		for(int k = 0; k < 30; k++){
			// proj[k].ent->hitboxes[0].pos = {-10000,-10000};
			// proj[k].AddToGArry();
			proj[k] = Entity();
			proj[k].hitboxes[0].speed = {0,0};
			proj[k].hitboxes[0].pos = {-128,-128};
		}
	}
	for(int k = 0; k < 30; k++)
		proj[i].ent->triggerID = 3;
	proj[i].ent->hitboxes.clear();
	proj[i].ent->hitboxTexts.clear();
	proj[i].addBox(EntityHitbox(LOADED_ENTITIES[source].hitboxes[0].pos.x + 45, LOADED_ENTITIES[source].hitboxes[0].pos.y + 80, currentScreen.tileSizeX,currentScreen.tileSizeY));
	if(player.hitboxes[0].pos.x == LOADED_ENTITIES[source].hitboxes[0].pos.x){
		proj[i].hitboxes[0].speed = {0,-1};
	}                                   
	?"
	\VESPD * (1.0f / AIDIFFICULTY);
	const double slope = (player.hitboxes[0].pos.y - (sourceP.y)) / (player.hitboxes[0].pos.x - (sourceP.x));
	proj[i].hitboxes[0].speed = {
		// ((x2 - x1) / |x2 - x1|) * cos(tan^-1((y2 - y1) / (x2 - x1)))
		(double)speedMultiplier * (double)xParity * std::cos(std::atan2(slope,1)) ,
		// ((x2 - x1) / |x2 - x1|) * sin(tan^-1((y2 - y1) / (x2 - x1)))
		(double)speedMultiplier * (double)xParity * std::sin(std::atan2(slope,1))
	};
	std::cout << proj[i].hitboxes[0].speed.x << " , " << proj[i].hitboxes[0].speed.y << std::endl;
	}
	proj[i].addTexture(badWords);
	i++;
	if(i == 30){
		i = 0;
	} */
}

void InitKarens(){
	// spooky
}

bool CollisionsContain(int x, bool update){
	static std::optional<std::vector<int>> collisions = TriggerCollision(player); // cache each frame
	if(update)
		collisions = TriggerCollision(player);
	if(!collisions.has_value())
		return false;
	return (std::find(collisions.value().begin(), collisions.value().end(), x) != collisions.value().end());
}

bool CollisionsContain(EntityContainer ent, int x, bool update){
	static std::optional<std::vector<int>> collisions = TriggerCollision(Entity(ent)); // cache each frame
	if(update)
		collisions = TriggerCollision(Entity(ent));
	if(!collisions.has_value())
		return false;
	return (std::find(collisions.value().begin(), collisions.value().end(), x) != collisions.value().end());
}

void PickUpKey(){
	if(!CollisionsContain(4)){
		return;	
	}
	LOADED_ENTITIES[FindTrigger(4).value()].dontDraw = true;
}

void PickUpCoin(){
	if(!CollisionsContain(6)){
		return;	
	}
	int coin = FindTrigger(6).value();
	LOADED_ENTITIES[coin].dontDraw = true;
	LOADED_ENTITIES[coin].hitboxes.clear();
}

void UseDoor(bool reset){
	static bool animStarted = false;
	static int door = 0;
	if(reset){
		door = 0;
		animStarted = false;
		HAS_KEY = false;
	}
	if(!animStarted && (!IsKeyDown(KEY_F) || !CollisionsContain(5) || !HAS_KEY))
		return;
	animStarted = true;
	if(!door)
		door = FindTrigger(5).value();
	 
	LOADED_ENTITIES[door].anim = true;
	if(LOADED_ENTITIES[door].animDone){
		NextLevel();
	}
}

void NextLevel(){
	if(currentScreen.fileName == "assets/LevelOne.ce"){
		currentScreen.fileName = "assets/LevelTwo.ce";
		InitGame();
	}
	return;
}

void SwitchMusic(Music& currentMusic){
	static Music world1 = LoadMusicStream("assets/world_1.wav");
	static Music world2 = LoadMusicStream("assets/world_2.mp3");
	if(currentScreen.fileName == "assets/LevelOne.ce"){
		currentMusic = world1;
	}
	if(currentScreen.fileName == "assets/LevelTwo.ce"){
		currentMusic = world2;
	}
	if(currentScreen.fileName == "assets/LevelThree.ce"){
		
	}
	PlayMusicStream(currentMusic);
}
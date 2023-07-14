#include "include/screen.hpp"
#include <iostream>
#include <cmath>
#include <emscripten/emscripten.h>

#define SCROLL_DEBUG false
#define NO_FALL false
// ^^ for testing jump height

Screen currentScreen;
Entity player;
double yVelo = 0;
double xVelo = 0;

bool started = false;
bool dead = false;

int AIDIFFICULTY = 1; // AI difficulty, affects movespeed and rate of fire

const double GRAVITY = 10; // downwards acceleration from gravity, in px/s^2
const double JUMPSTR = 5; // upwards (one time) velocity gain from jumping, in px/s
const int MOVESPEED = 60; // sideways acceleration from moving, in px/^2
const int MOVESPEEDCAP = 400; // maximum sideways speed, in px/s

const int TICKSPEED = 60;
const double TICKTIME = (1.0f/(double)TICKSPEED);

const int TEXTUREBOX = 64; // size of the texture hitbox, in (px)^2 (not a square measurement)
const int COLLISIONBOXH = 60; // height of the collision hitbox, in px
const int COLLISIONBOXW = 36; // width of the collision hitbox in px

double SCROLLSTR = 1;
double SCROLLSTRY = 1;

int SIGNFONTSIZE = 20; // how big the sign's font size is, in px
int SIGNELEVATION = 150; // how high the sign text appears above the sign, in px

int MOVESPD = MOVESPEED * TICKTIME;
int MOVECAP = MOVESPEEDCAP * TICKTIME;

bool HAS_KEY = false;
int COINS = 0; // idk?? fun

// for moving blocks
const double animPeriodTime = 6;
std::vector<int> animPeriod;
std::vector<int> animID;
std::vector<float> animTime;

bool OnGround(){
	if(!currentScreen.CheckMove(player.hitboxes[0]))
		return false;
	bool flag;
	player.hitboxes[0].pos.y += 1;
	flag = !currentScreen.CheckMove(player.hitboxes[0]);
	player.hitboxes[0].pos.y -= 1;
	return flag;
}

bool SnapIfClose(int r = 5){
	for(int i = 0; i < r; i++){
		player.hitboxes[0].pos.y += i;
		if(OnGround())
			return true;
		player.hitboxes[0].pos.y -= 2 * i;
		if(OnGround())
			return true;
		player.hitboxes[0].pos.y += i;
	}
	return false;
}

class Input{
public:
	DoublePoint dir = {0,0};
	bool justJumped = false;
	bool discardSide = false;
	bool lastRight = true;
	void GetInput();
	void ResetInput(){
		dir = {0,0};
	}
};

void Shmove(Input& playerInput);
void ChangeTexture(bool right, bool flying = false);

void ScrollScreen(bool reset = false);
void MoveEverything(double dir);
void MoveEverythingY(double dir);

void ReadSign();
void FixTextures();

void ReadMoving();
bool AnimMoving(Input& playerInput,bool backwards = false);

bool StartScreen(Music &music);
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

void WanderAminals();

inline void Tick(double &time, Input &playerInput, DoublePoint &old, std::string &world, Music &music);

void UpdateDrawFrame(){
	static Texture2D hudKey = LoadTexture("assets/still_key.png");
	static Texture2D noKeys = LoadTexture("assets/keyless.png");
	static Music music;
	static double time = 0;
	static std::string world = "";
	static Input playerInput;
	static DoublePoint old;
	static Image screen;
	static Texture2D screenText;
	static Shader shader = LoadShader(0, TextFormat("shader/crt.glsl", 100));
	if(!IsAudioDeviceReady()){
		InitAudioDevice();
	}
	time += GetFrameTime();
	if(!started){
		if(StartScreen(music)){
			return;
		}
	}
	if(dead){
		if(DEBUG)
			DrawFPS(0,SCREENY - 100);
		SeekMusicStream(music,0.0f);
		DeathScreen();
		playerInput.ResetInput();
		return;
	}
	if(DEBUG && !dead && started){
		if(IsKeyPressed(KEY_F2)){
			currentScreen.fileName = "assets/LevelTwo.ce";
			InitGame();
		}
	}
	playerInput.GetInput();
	if(time > TICKTIME) {
		Tick(time, playerInput, old, world, music);
	}
	UpdateMusicStream(music);
	player.hitboxes[1].pos = DoublePoint{ player.hitboxes[0].pos.x -  0.5 * (TEXTUREBOX - COLLISIONBOXW) , player.hitboxes[0].pos.y -  0.5 * (TEXTUREBOX - COLLISIONBOXH) };
	BeginDrawing();
		currentScreen.Draw();
		DrawEntities();
		ReadSign();
/*		screen = LoadImageFromScreen();
		UnloadTexture(screenText);
		screenText = LoadTextureFromImage(screen);
		free(screen.data);
		ClearBackground(WHITE);
	BeginShaderMode(shader);
		DrawTexture(screenText, 0, 0, WHITE);
	EndShaderMode(); */
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

void Input::GetInput(){
	if((IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) ^ (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))){
		if(!discardSide){
			dir.x += (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) ? MOVESPD : -1 * MOVESPD;
			lastRight = (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D));
		}
	}
	else{
		dir.x = 0;
		discardSide = false;
	}
	
	if((OnGround() || CollisionsContain(787)) && (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W) || IsKeyDown(KEY_SPACE))){
		std::cout << "JHUMPED !!! \n";
		dir.y = -1 * JUMPSTR;
		justJumped = true;
		if(CollisionsContain(787)){ // remove double jump orb 
			if(FindTrigger(787).has_value()){
				std::cout << "trigger collision player : " << FindTrigger(787).value() << std::endl;
				LOADED_ENTITIES[FindTrigger(787).value()].dontDraw = true;
				LOADED_ENTITIES[FindTrigger(787).value()].triggerID = 0; // evil
			}
		}
	}
	// clamp x and y speeds
	if(dir.y < -1 * JUMPSTR) // hacky
		dir.y = -1 * JUMPSTR;
	if(std::abs(dir.x) > MOVECAP)
		dir.x = MOVECAP * (std::abs(dir.x)/dir.x);
}

void Shmove(Input& playerInput){
	if(!started){
		return;
	}
	static bool flying = true;
	
	if(OnGround() && !playerInput.justJumped)
		playerInput.dir.y = 0;
	else{
		playerInput.dir.y += TICKTIME * GRAVITY;
		if(NO_FALL && playerInput.dir.y < 0)
			playerInput.dir.y = 0;
		playerInput.justJumped = false;
	}
	player.hitboxes[0].pos.x += playerInput.dir.x;
	if(!currentScreen.CheckMove(player.hitboxes[0]) || player.hitboxes[0].pos.x < 0 || player.hitboxes[0].pos.y > SCREENX){
		player.hitboxes[0].pos.x -= playerInput.dir.x;
		playerInput.dir.x = 0;
	}

	player.hitboxes[0].pos.y += playerInput.dir.y;
	if(!currentScreen.CheckMove(player.hitboxes[0])){
		player.hitboxes[0].pos.y -= playerInput.dir.y;
		playerInput.dir.y *= -1;
	}
	if (playerInput.dir.y != 0){
		flying = true;
	}

	if (SnapIfClose(playerInput.dir.y * 2)){
		playerInput.dir.y = 0;
		flying = false;
	}

	ChangeTexture(playerInput.lastRight,flying);
}

void ScrollScreen(bool reset){
	static DoublePoint pos = {0,0};
	static DoublePoint oldpos = pos;
	static Entity screenAnchor;
	static bool init = false;
	static Texture2D anchorText = LoadTexture("assets/transparent.png");
	static double upperBound = currentScreen.width - ((double)SCREENX/(double)currentScreen.tileSizeX); 
	// how far right you can scroll
	static double lowerBoundY = currentScreen.height - ((double)SCREENY/(double)currentScreen.tileSizeY);
	// how far down you can scroll
	if(!init){
		screenAnchor.addBox(EntityHitbox(0,0,1,1));
		screenAnchor.addTexture(anchorText);
		screenAnchor.DontDraw(true);
		init = true;
	}
	if (reset){
		std::cout << "resetting scroll" << std::endl;
		pos = {0,0};
		upperBound = currentScreen.width - ((double)SCREENX/(double)currentScreen.tileSizeX);
		screenAnchor.AddToGArry();
		screenAnchor.addBox(EntityHitbox(0,0,1,1));
		screenAnchor.DontDraw(true);
		screenAnchor.addTexture(anchorText);
		currentScreen.offset.x = screenAnchor.hitboxes[0].pos.x; 
		if(((oldpos.x - pos.x) || (oldpos.y - pos.y)) && SCROLL_DEBUG)
			std::cout << "new pos : " << pos.x << " , " << pos.y << std::endl;
		oldpos = pos;
		return;
	}
	// tile sky to save memory. there's definitely a better way to do this
	currentScreen.offset.x = screenAnchor.hitboxes[0].pos.x;
	if((currentScreen.offset.x + ((float)currentScreen.background.width / 3.0f)) > SCREENX / 2)
		screenAnchor.hitboxes[0].pos.x -= (float)currentScreen.background.width / 3.0f;
	if((currentScreen.offset.x + (2.0f * (float)currentScreen.background.width / 3.0f)) < SCREENX / 2)
		screenAnchor.hitboxes[0].pos.x += (float)currentScreen.background.width / 3.0f;

	if((CollisionsContain(1) || CollisionsContain(2)) && pos.x + SCROLLSTR <= upperBound && pos.x + SCROLLSTR >= 0){
		if ((CollisionsContain(1) && (std::abs(SCROLLSTR) / SCROLLSTR) == 1) ||
			(CollisionsContain(2) && (std::abs(SCROLLSTR) / SCROLLSTR) == -1)  ){
			MoveEverything(-1.0f * SCROLLSTR);
			pos.x += SCROLLSTR;
		}
		currentScreen.offset.x = screenAnchor.hitboxes[0].pos.x; // quick and dirty
	}	
	if((CollisionsContain(443) || CollisionsContain(449))
		&& pos.y + SCROLLSTRY <= lowerBoundY && pos.y + SCROLLSTRY >= 0){
		if ((CollisionsContain(443) && (std::abs(SCROLLSTRY) / SCROLLSTRY) == -1) ||
			(CollisionsContain(449) && (std::abs(SCROLLSTRY) / SCROLLSTRY) == 1)  ){
			MoveEverythingY(-1.0f * SCROLLSTRY);
			pos.y += SCROLLSTRY;
		}
	}
	oldpos = pos;
}

void MoveEverything(double dir){
	for(int i = 0; i < LOADED_ENTITIES_HEAD; i++){
		if(LOADED_ENTITIES[i].triggerID == 1 || LOADED_ENTITIES[i].triggerID == 2
		|| LOADED_ENTITIES[i].triggerID == 443 || LOADED_ENTITIES[i].triggerID == 449){
			if(++i < LOADED_ENTITIES_HEAD)
				return;	
		}
		for(int k = 0; k < LOADED_ENTITIES[i].hitboxes.size(); k++){
			LOADED_ENTITIES[i].hitboxes[k].pos.x += (int)(dir * currentScreen.tileSizeX);
			if (SCROLL_DEBUG)
				std::cout << "Moving hitbox " << k << " of entity " << i << " to " << LOADED_ENTITIES[i].hitboxes[k].pos.x << " (" << dir << " * " << currentScreen.tileSizeX << ") pixels" << std::endl;
		}
	}
}

void MoveEverythingY(double dir){
	for(int i = 0; i < LOADED_ENTITIES_HEAD; i++){
		if(LOADED_ENTITIES[i].triggerID == 1 || LOADED_ENTITIES[i].triggerID == 2
		|| LOADED_ENTITIES[i].triggerID == 443 || LOADED_ENTITIES[i].triggerID == 449){
			if(++i < LOADED_ENTITIES_HEAD)
				return;	
		}
		for(int k = 0; k < LOADED_ENTITIES[i].hitboxes.size(); k++){
			LOADED_ENTITIES[i].hitboxes[k].pos.y += (int)(dir * currentScreen.tileSizeY);
			if (SCROLL_DEBUG)
				std::cout << "Moving hitbox " << k << " of entity " << i << " to " << LOADED_ENTITIES[i].hitboxes[k].pos.y << " (" << dir << " * " << currentScreen.tileSizeY << ") pixels" << std::endl;
		}
	}
}

bool StartScreen(Music &music){
	static const Texture logoText = LoadTexture("assets/mochiko_title.png");
	static const int START = 1;
	static const int CREDITS = 2;
	static bool showing_credits = false;
	static bool init = false;
	static int selection = START;
	
	if (!init){
		SwitchMusic(music);
		currentScreen.fileName = "assets/start_screen.ce";
		currentScreen.ReadFromFile(currentScreen.fileName);
		currentScreen.Load();

		// set title dimensions
		LOADED_ENTITIES[0].hitboxes[0].width = 816;
		LOADED_ENTITIES[0].hitboxes[0].height = 53;
		LOADED_ENTITIES[0].offset = {0,0};
		LOADED_ENTITIES[0].anim = false;
		LOADED_ENTITIES[0].hitboxes[0].pos = {104,0};
		LOADED_ENTITIES[0].hitboxTexts.push_back(logoText);

		// set start dimensions and anim
		LOADED_ENTITIES[1].hitboxes[0].width = 236;
		LOADED_ENTITIES[1].hitboxes[0].height = 120;
		LOADED_ENTITIES[1].anim = true;
		LOADED_ENTITIES[1].animOffset = {236,0};
		LOADED_ENTITIES[1].frames = 4;
		LOADED_ENTITIES[1].fps = 5;
		LOADED_ENTITIES[1].hitboxes[0].pos.x = 394;

		// set credits dimensions and anim
		LOADED_ENTITIES[2].hitboxes[0].width = 236;
		LOADED_ENTITIES[2].hitboxes[0].height = 120;
		LOADED_ENTITIES[2].anim = true;
		LOADED_ENTITIES[2].animOffset = {236,0};
		LOADED_ENTITIES[2].fps = 5;
		LOADED_ENTITIES[2].hitboxes[0].pos.x = 394;

		init = true;
	}
	if(((IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) && selection == START)
	 ||((IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) && selection == CREDITS)){ 
		// switch selection (since there're only two)
		selection = (selection == CREDITS) ? START : CREDITS;
	}
	// set the selection offset
	(selection == CREDITS) ? LOADED_ENTITIES[2].offset = {0,120} : LOADED_ENTITIES[1].offset = {0,120};
	// unset the unselected offset
	(selection == CREDITS) ? LOADED_ENTITIES[1].offset = {0,0} : LOADED_ENTITIES[2].offset = {0,0};
	if(IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)){
		if(showing_credits){
			showing_credits = false;
			return false;
		}
		if(selection == CREDITS){
			showing_credits = true;
		}
		if(selection == START){
			clearEntities();
			currentScreen.entities.clear();
			InitGame();
			started = true;
			return false;
		}
	}
	if(showing_credits){
		// draw credits 
		BeginDrawing();
			ClearBackground(WHITE);
			DrawText("Coding by Emelia Aleksanyan\nEngine by Emelia Aleksanyan\nPixel art by Emelia Aleksanyan\nMusic by Emelia Aleksanyan\nMapping by Dash Raimond",300,100,SIGNFONTSIZE,BLACK);
			DrawText("[PRESS ENTER TO CONTINUE]",300,300,SIGNFONTSIZE,GRAY);
		EndDrawing();
		return true; // return early from the updatedraw cycle, get control of drawing
	}
	return false;
}

void DeathScreen(){
	static Entity deathText;
	static Entity plosion;
	static Music deathMusic = LoadMusicStream("assets/death_screen.wav"); 
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
	Entity leftScroll;
	Entity rightScroll;
	Entity topScroll;
	Entity bottomScroll;
	static Texture2D sky = LoadTexture("assets/sky.png");
	static Texture2D night = LoadTexture("assets/night.png");
	if(DEBUG)
		std::cout << "(re)Initializing game" << std::endl;
	if(!started){
		// set up player
		player.ent->anim = true; 
		player.ent->animOffset = {0,64};
		player.ent->fps = 5;
		player.ent->frames = 2;
		player.addTexture(LoadTexture("assets/transparent.png")); // transparent texture for the collision box
		player.addTexture(LoadTexture("assets/chicken.png")); // actual texture for the drawing box
		player.addBox(EntityHitbox((DoublePoint) { 0.5 * (TEXTUREBOX - COLLISIONBOXW), 0.5 * (TEXTUREBOX - COLLISIONBOXH) }, COLLISIONBOXW, COLLISIONBOXH)); // collision box
		player.addBox(EntityHitbox((DoublePoint) { 0, 0 }, TEXTUREBOX, TEXTUREBOX)); // drawing box

		// set up screen
		currentScreen.fileName = "assets/LevelOne.ce";
	}
	currentScreen.entities.clear();
	currentScreen.entities.push_back(*player.ent);
	currentScreen.ReadFromFile(currentScreen.fileName);
	currentScreen.Load();
	currentScreen.backgroundIsText = true;
	currentScreen.backgroundTint = WHITE;

	// add scrolls to the array
	leftScroll.AddToGArry();
	rightScroll.AddToGArry();
	topScroll.AddToGArry();
	bottomScroll.AddToGArry();

	// set up left scroll hitbox
	leftScroll.addBox(EntityHitbox(0,0,64,SCREENY));
	leftScroll.ent->triggerID = 2;
	leftScroll.ent->trigger = true;
	leftScroll.ent->dontDraw = true;

	// set up right scroll hitbox
	rightScroll.addBox(EntityHitbox(SCREENX - (64 * 11),0,64,SCREENY));
	rightScroll.ent->triggerID = 1;
	rightScroll.ent->trigger = true;
	rightScroll.ent->dontDraw = true;
	
	// set up top scroll hitbox
	topScroll.addBox(EntityHitbox(0,
								  SCREENY - (64 * 9),
								  SCREENX,
								  64));
	topScroll.ent->triggerID = 443;
	topScroll.ent->trigger = true;
	topScroll.ent->dontDraw = true;

	// set up bottom scroll hitbox
	bottomScroll.addBox(EntityHitbox(0,
								  SCREENY - (64 * 1),
								  SCREENX,
								  64));	
	bottomScroll.ent->triggerID = 449;
	bottomScroll.ent->trigger = true;
	bottomScroll.ent->dontDraw = true;

	// set up sky
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

	// misc initializations
	FixTextures(); 
	ReadMoving(); // read the period of moving blocks
	ScrollScreen(true);
	AttackPlayer(-1);
	UseDoor(true);
	if(FindTrigger(5).has_value())
		LOADED_ENTITIES[FindTrigger(5).value()].anim = false;

	// center the player hitboxes
	player.hitboxes[0].pos.x += 0.5 * (TEXTUREBOX - COLLISIONBOXW);
	player.hitboxes[0].pos.y += 0.5 * (TEXTUREBOX - COLLISIONBOXH);

}

void ChangeTexture(bool right, bool flying){
	if(right){
		player.Offset()->x = 0 + (flying ? 128 : 0);
		return;
	}
	player.Offset()->x = 64 + (flying ? 128 : 0);
}

void ReadSign(){
	for(int i = 0; i < LOADED_ENTITIES_HEAD; i++){
		if (LOADED_ENTITIES[i].signText != ""){
			if (LOADED_ENTITIES[i].Colliding(player.hitboxes[0])){
				DrawText(LOADED_ENTITIES[i].signText.c_str(),
				LOADED_ENTITIES[i].hitboxes[0].pos.x - (0.25 * MeasureText(LOADED_ENTITIES[i].signText.c_str(), SIGNFONTSIZE)) ,
				LOADED_ENTITIES[i].hitboxes[0].pos.y - SIGNELEVATION, SIGNFONTSIZE , BLACK);
				return;
			}
		}
	}
}

void FixTextures(){
	for(int i = 0; i < LOADED_ENTITIES_HEAD; i++){
			if(LOADED_ENTITIES[i].trigger && !(LOADED_ENTITIES[i].triggerID % 11)){
				if(LOADED_ENTITIES[i].triggerID > 11)
					LOADED_ENTITIES[i].triggerID /= 11;
				 
				for(int k = 0; k < LOADED_ENTITIES[i].hitboxes.size(); k++){
						LOADED_ENTITIES[i].hitboxes[k].width = LOADED_ENTITIES[i].hitboxTexts[k].width;
						LOADED_ENTITIES[i].hitboxes[k].height = LOADED_ENTITIES[i].hitboxTexts[k].height;
				}
			}
			else if(LOADED_ENTITIES[i].trigger && !(LOADED_ENTITIES[i].triggerID % 7)){ //fix door
				for(int k = 0; k < LOADED_ENTITIES[i].hitboxes.size(); k++){
						LOADED_ENTITIES[i].hitboxes[k].width = LOADED_ENTITIES[i].hitboxTexts[k].width / 4;
						LOADED_ENTITIES[i].hitboxes[k].height = LOADED_ENTITIES[i].hitboxTexts[k].height;
				}
				LOADED_ENTITIES[i].animOffset = {LOADED_ENTITIES[i].hitboxTexts[0].width / 4, 0};
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
	chaseTimer += TICKTIME;
	timeSinceLastProj += TICKTIME;
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
	static Music world_start = LoadMusicStream("assets/start_screen.wav");
	static Music world1 = LoadMusicStream("assets/world_1.mp3");
	static Music world2 = LoadMusicStream("assets/world_2.mp3");
	if(currentScreen.fileName == "assets/start_screen.ce"){
		currentMusic = world_start;
	}
	if(currentScreen.fileName == "assets/LevelOne.ce"){
		currentMusic = world1;
	}
	if(currentScreen.fileName == "assets/LevelTwo.ce"){
		currentMusic = world2;
	}
	if(currentScreen.fileName == "assets/LevelThree.ce"){
		
	}
	SeekMusicStream(currentMusic,0.0f);
	PlayMusicStream(currentMusic);
}

void ReadMoving(){
	animPeriod.clear();
	animTime.clear();
	animID.clear();
	for(int i = 0; i < LOADED_ENTITIES_HEAD; i++){
		if(!(LOADED_ENTITIES[i].triggerID % 17) && LOADED_ENTITIES[i].triggerID){
			animPeriod.push_back(0);
			animTime.push_back(0);
			animID.push_back(i);
			while(LOADED_ENTITIES[i].triggerID != 17){
				LOADED_ENTITIES[i].triggerID /= 17;
				animPeriod[animPeriod.size() - 1]++;
			}
			animTime[animPeriod.size() - 1] = 0;
			animPeriod[animPeriod.size() - 1]++;
			animPeriod[animPeriod.size()] *= currentScreen.tileSizeX;
		}
	}
}

bool AnimMoving(Input& playerInput, bool backwards){
	int move;
	for(int i = 0; i < animID.size(); i++){
		animTime[i] += backwards ? TICKTIME * -1.0f : TICKTIME;
		move = animPeriod[i] / animPeriodTime;
		(animTime[i] < animPeriodTime / 2) ? : move *= -1.0f;
		if(backwards) 
			move *= -1.0f;
		if(animTime[i] > animPeriodTime)
			animTime[i] = 0;
		LOADED_ENTITIES[animID[i]].hitboxes[0].pos.x += move;
		if(player.Colliding(LOADED_ENTITIES[animID[i]])){
			playerInput.dir.x += move;
			if(player.hitboxes[0].pos.y + COLLISIONBOXH > LOADED_ENTITIES[animID[i]].hitboxes[0].pos.y){ // player is in the thing
				playerInput.dir.x -= move;
				int side = (player.hitboxes[0].pos.x - LOADED_ENTITIES[animID[i]].hitboxes[0].pos.x) / 
				   std::abs(player.hitboxes[0].pos.x - LOADED_ENTITIES[animID[i]].hitboxes[0].pos.x);
				player.hitboxes[0].pos.x += side * std::abs(move - playerInput.dir.x); 
			}
		}
	}
	return false;
}

void WanderAminals(){ // fix this shit 
/*
	const static int threshold = 45;
	const static int speed = 2;
	auto res;
	for(int i = 0; i < LOADED_ENTITIES_HEAD; i++){
		if(std::abs(LOADED_ENTITIES[i].triggerID) == 7){
			LOADED_ENTITIES[i].hitboxes[0].pos.x += (LOADED_ENTITIES[i].triggerID / 7) * speed;
			res = TriggerCollision(LOADED_ENTITIES[i]);
			if (res.has_value() && res.){
				LOADED_ENTITIES[i].hitboxes[0].pos.x -= (LOADED_ENTITIES[i].triggerID / 7) * speed;
				LOADED_ENTITIES[i].triggerID *= -1;
			}
			else
				LOADED_ENTITIES[i].hitboxes[0].pos.x += (LOADED_ENTITIES[i].triggerID / 7) * speed;

			LOADED_ENTITIES[i].hitboxes[0].pos.x += (LOADED_ENTITIES[i].triggerID / 7) * speed;
		}
	} */
}

inline void Tick(double &time, Input &playerInput, DoublePoint &old, std::string &world, Music &music){
		time = 0;
		WanderAminals();
		AnimMoving(playerInput);
		Shmove(playerInput);
		SCROLLSTR = ((player.hitboxes[0].pos.x - old.x) / currentScreen.tileSizeX);
		SCROLLSTRY = ((player.hitboxes[0].pos.y - old.y) / currentScreen.tileSizeY);
		ScrollScreen();
		UpdateKarens();
		PickUpKey();
		PickUpCoin();
		UseDoor();
		if (CollisionsContain(3,true))
			dead = true;
		
		if (CollisionsContain(4))
			HAS_KEY = true;
		
		if(dead){
			old = player.hitboxes[0].pos;
			if(DEBUG)
				DrawFPS(0,SCREENY - 100);
			SeekMusicStream(music,0.0f);
			DeathScreen();
			return;
		}
		if(world != currentScreen.fileName)
			SwitchMusic(music);
		world = currentScreen.fileName;
		old = player.hitboxes[0].pos;
}
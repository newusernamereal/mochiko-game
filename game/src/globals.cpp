#include "include/screen.hpp"
#include "globals.hpp"
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


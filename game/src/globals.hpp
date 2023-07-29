#pragma once
#ifndef GLOBALS_H
#define GLOBALS_H
#define SCROLL_DEBUG false
#define NO_FALL false // for testing jump height
#include "include/screen.hpp"
extern Screen currentScreen;
extern Entity player;
extern double yVelo;
extern double xVelo;

extern bool started;
extern bool dead;

extern int AIDIFFICULTY; // AI difficulty, affects movespeed and rate of fire

extern const double GRAVITY; // downwards acceleration from gravity, in px/s^2
extern const double JUMPSTR; // upwards (one time) velocity gain from jumping, in px/s
extern const int MOVESPEED; // sideways acceleration from moving, in px/^2
extern const int MOVESPEEDCAP; // maximum sideways speed, in px/s

extern const int TICKSPEED;
extern const double TICKTIME;

extern const int TEXTUREBOX; // size of the texture hitbox, in (px)^2 (not a square measurement)
extern const int COLLISIONBOXH; // height of the collision hitbox, in px
extern const int COLLISIONBOXW; // width of the collision hitbox in px

extern double SCROLLSTR;
extern double SCROLLSTRY;

extern int SIGNFONTSIZE; // how big the sign's font size is, in px
extern int SIGNELEVATION; // how high the sign text appears above the sign, in px

extern int MOVESPD;
extern int MOVECAP;

extern bool HAS_KEY;
extern int COINS; // idk?? fun

// for moving blocks
extern const double animPeriodTime;
extern std::vector<int> animPeriod;
extern std::vector<int> animID;
extern std::vector<float> animTime;
#endif

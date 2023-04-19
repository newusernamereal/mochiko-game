#ifndef engine_h
#define engine_h
#include <string>
#include <iostream>
#include "/home/catto/raylib/src/raylib.h"
// Macros / global variables = SNAKE_CASE_CAPS
// Functions / classes = PascalCase
// Variables = camelCase
#define SCREENX 1024
#define SCREENY 576
#define ENTITY_MAX 2000
#define DEBUG false
#define DRAW_DEBUG false
// ^^ debug for specifically drawing hitboxes, which spams console and renders a lot of other debug output unreadable with a lot of entities, but still useful.
#define EMAX_CRASH false
#define SQUARE_TILES true
// ^^ if true, tiles are square, allowing for a nonsquare resolution without stretching tiles
class Point {
public :
	 int x;
	 int y;
};
class DoublePoint {
public :
	 int x;
	 int y;
};
class BackgroundScreen {
public:
	bool backgroundIsText;
	Texture2D background;
	Color backgroundTint = WHITE;
	DoublePoint offset = {0,0};
	Point animOffset = {0,0};
	bool anim = false;
	int animFps = 5;
	int currentFrame = 0;
	int frames = 5;
	bool animDone = false;
	double time = 0;
	void Draw();
	BackgroundScreen(std::string backgroundImage = "\n");
	void Init(std::string windowName, int fps = 0, std::string backgroundImage = "\n");
};

Point DoublePointToPoint(DoublePoint d);
DoublePoint PointToDoublePoint(Point p);
bool ColorsEqual(Color c1, Color c2);

#endif

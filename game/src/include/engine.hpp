#ifndef engine_h
#define engine_h
#include <string>
#include <iostream>
#include "/home/catto/raylib/src/raylib.h"
// Macros / global variables = SNAKE_CASE_CAPS
// Functions / classes = PascalCase
// Variables = camelCase
#define SCREENX 640
#define SCREENY 640
#define ENTITY_MAX 2000
#define DEBUG false
#define DRAW_DEBUG false
// ^^ debug for specifically drawing hitboxes, which spams console and renders a lot of other debug output unreadable with a lot of entities, but still useful.
#define EMAX_CRASH false
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
	void Draw();
	BackgroundScreen(std::string backgroundImage = "\n");
	void Init(std::string windowName, int fps = 0, std::string backgroundImage = "\n");
};

Point DoublePointToPoint(DoublePoint d);
DoublePoint PointToDoublePoint(Point p);
bool ColorsEqual(Color c1, Color c2);

#endif

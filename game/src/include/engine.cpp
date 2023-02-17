#include "engine.hpp"
// Macros / global variables = SNAKE_CASE_CAPS
// Functions / classes = PascalCase
// Variables = camelCase
#define SCREENX 800
#define SCREENY 800
#define ENTITY_MAX 2000
#define DEBUG false
#define EMAX_CRASH false
Point DoublePointToPoint(DoublePoint d){
	return(Point({ int(d.x) , int(d.y) }));
}
DoublePoint PointToDoublePoint(Point p){
	return(DoublePoint({ double(p.x) , double(p.y) }));
}
bool ColorsEqual(Color c1, Color c2){
	if(c1.r == c2.r && c1.g == c2.g && c1.b == c2.b) {
//	std::cout<<"colours are equal"<<std::endl;
	return true;
	}
	return false;
}
void BackgroundScreen::Draw(){
	if((*this).backgroundIsText){
		DrawTextureRec(background, { 0,0,SCREENX,SCREENY }, {0,0},backgroundTint);
	}
	else{
		ClearBackground(backgroundTint);
	}
}
BackgroundScreen::BackgroundScreen(std::string backgroundImage) {
	backgroundIsText = false;// Texture, not literal text
	if (backgroundImage != "\n") {
		background = LoadTexture(backgroundImage.c_str());
		backgroundIsText = true;
	}
}
void BackgroundScreen::Init(std::string windowName, int fps, std::string backgroundImage) { // just using \n as a placeholder because it wont be in any file names
	std::cout << std::endl << "Starting Initialization" << std::endl;
	InitWindow(SCREENX, SCREENY, windowName.c_str());
	if (fps) {
		SetTargetFPS(fps);
	}
	backgroundIsText = false;// Texture, not literal text
	if (backgroundImage != "\n") {
		background = LoadTexture(backgroundImage.c_str());
		backgroundIsText = true;
	}
	std::cout << std::endl << "Initialization Finished" << std::endl;
}

	

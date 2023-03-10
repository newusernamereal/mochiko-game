#ifndef screen_h
#define screen_h
#include "engine.hpp"
#include "entity.hpp"
#include <fstream>
class Screen : public BackgroundScreen {
public:
	std::vector<EntityContainer> entities;
	std::vector<EntityContainer> barriers;
	double tileSizeX = 1;
	double tileSizeY = 1;
	int size = 1;
	int width = 1;
	std::string fileName;
	Screen(void) {}
	Screen(Texture2D backin, std::vector<EntityContainer> entin);
	void Load(void);
	void ReadFromFile(std::string fileName, DoublePoint player = { -1,-1 });
	bool CheckMove(Point to);
	bool CheckMove(EntityContainer to);
	bool CheckMove(EntityHitbox to);
};

int TriggerCollision(Entity& Ent);
Point FindTrigger(Screen& screen, int find);
#endif

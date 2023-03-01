#ifndef entity_h
#define entity_h
#include <vector>
#include "engine.hpp"
#include <cmath>
class EntityHitbox {
public:
	DoublePoint speed = { 0 , 0 }; // speed, in px/s
	DoublePoint pos = { 0 , 0 }; // topleft corner
	int width = 1;
	int height = 1;
	double debugMoveCounter = 0;
	EntityHitbox(double posX, double posY, int widthin, int len, double speedin = 0) {
		pos.x = posX;
		pos.y = posY;
		width = widthin;
		height = len;
		speed = DoublePoint{ speedin, speedin };
	}
	EntityHitbox(DoublePoint posIn, int widthin, int len, double speedin = 0) {
		pos = posIn;
		width = widthin;
		height = len;
		speed = DoublePoint{ speedin, speedin };
	}
	EntityHitbox(Point posIn, int widthin, int len, double speedin = 0) {
		pos = PointToDoublePoint(posIn);
		width = widthin;
		height = len;
		speed = DoublePoint{ speedin, speedin };
	}
	EntityHitbox(){}
	inline void Move() {
		pos.x += GetFrameTime() * speed.x;
		pos.y += GetFrameTime() * speed.y;
	}
}; // hitboxes, no drawing
class EntityContainer { // container class for entities
public:
	bool trigger = false;
	int triggerID = 0;
	std::vector<EntityHitbox> hitboxes;
	std::vector<Texture2D> hitboxTexts;
	Color tint = WHITE;
	double debugDrawCounter = 0;
	bool Colliding(DoublePoint p);
	Point offset = { 0 , 0 }; // offset of the texture, in px
	std::string signText;
	bool dontDraw = false;
	inline bool Colliding(Point p) {
		return(Colliding(PointToDoublePoint(p)));
	}
	inline bool Colliding(EntityHitbox ent) {
		for (int i = 0; i < hitboxes.size(); i++) {
			if (Colliding((DoublePoint) { ent.pos.x, ent.pos.y })) { // topleft
				return true;
			}
			if (Colliding((DoublePoint) { (ent.pos.x + ent.width), ent.pos.y })) { // topright
				return true;
			}
			if (Colliding((DoublePoint) { (ent.pos.x + ent.width), (ent.pos.y + ent.height) })) { // bottomright
				return true;
			}
			if (Colliding((DoublePoint) { ent.pos.x, (ent.pos.y + ent.height) })) { // bottomleft
				return true;
			}
		}
		return false;
	}
	inline bool Colliding(EntityContainer ent) {
		for (int i = 0; i < ent.hitboxes.size(); i++) {
			if (Colliding(ent.hitboxes[i])) {
				return true;
			}
		}
		return false;
	}
};
inline EntityContainer LOADED_ENTITIES[ENTITY_MAX];
inline int LOADED_ENTITIES_HEAD = 0;
inline void initEntityArr() {
	EntityContainer empty;
	for (int i = 0; i < ENTITY_MAX; i++) {
		LOADED_ENTITIES[i] = empty;
	}
	LOADED_ENTITIES_HEAD = 0;
}
inline void clearEntities() {
	initEntityArr();
}
inline void clearEntitiesExceptFirst() {
	EntityContainer empty;
	for (int i = 1; i < ENTITY_MAX; i++) {
		LOADED_ENTITIES[i] = empty;
	}
	LOADED_ENTITIES_HEAD = 1;
}
class Entity { // this is so scuffed and i hate it dont hmu i will break down if you ask me about it
public:
	EntityContainer* ent;
	EntityHitbox* hitboxes;
	Texture2D* hitboxTexts;
	Entity(void);
	void AddToGArry(void);
	Entity(EntityContainer in);
	inline Entity operator = (Entity const& in) { // without this it would simply replace the pointers and that is usually not what is intended
		ent->debugDrawCounter = in.ent->debugDrawCounter;
		ent->hitboxes = in.ent->hitboxes;
		ent->hitboxTexts = in.ent->hitboxTexts;
		ent->trigger = in.ent->trigger;
		ent->tint = in.ent->tint;
		return (*this);
	}
	inline bool OutOfBounds() {
		for (int i = 0; i < ent->hitboxes.size(); i++) {
			if (std::min(hitboxes[i].pos.x, hitboxes[i].pos.y) < 0) {
				return true;
			}
			if (hitboxes[i].pos.x + hitboxes[i].width > SCREENX) {
				return true;
			}
			if (hitboxes[i].pos.y + hitboxes[i].height > SCREENY) {
				return true;
			}
		}
		return false;
	}
	// inheritance wont work here so
	inline bool Colliding(DoublePoint p) { return (*ent).Colliding(p); };
	inline bool Colliding(Point p) { return (*ent).Colliding(p); };
	inline bool Colliding(EntityHitbox entin) { return (*ent).Colliding(entin); };
	inline bool Colliding(Entity entin) { return (*ent).Colliding(*entin.ent); };
	inline bool Colliding(EntityContainer entin) { return (*ent).Colliding(entin); };
	inline void Trigger(bool trigger) { (*ent).trigger = trigger; }
	inline bool Trigger() { return (*ent).trigger; }
	inline bool DontDraw() { return (*ent).dontDraw; }
	inline void DontDraw(bool dontDraw){ (*ent).dontDraw = dontDraw; }
	inline void Offset(Point p){ ent->offset = p; }
	inline Point* Offset(){ return &(ent->offset); }
	inline void addBox(EntityHitbox in) {
		ent->hitboxes.push_back(in);
		hitboxes = ent->hitboxes.data();
	}
	inline void addTexture(Texture2D in) {
		ent->hitboxTexts.push_back(in);
		hitboxTexts = ent->hitboxTexts.data();
	}
	//	EntityHitbox* hitboxes(int x) { return &(ent->hitboxes[x]); }
	//	Texture2D* hitboxTexts(int x) { return &(ent->hitboxTexts[x]); }
	inline Color tint() { return ent->tint; }
	inline void tint(Color in) { ent->tint = in; }
};

void initEntityArr(void);
void clearEntities(void);
void clearEntitiesExceptFirst(void);

void DrawEntity(int k);
void DrawEntity(Entity ent);
void DrawEntity(EntityContainer ent, int k = -1);
void DrawEntities(void);
void MoveEntities(void);
#endif

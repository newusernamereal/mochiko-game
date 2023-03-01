#include "entity.hpp"
bool EntityContainer::Colliding(DoublePoint p) {
	for (int i = 0; i < hitboxes.size(); i++) {
		if (p.x >= hitboxes[i].pos.x && p.x <= (hitboxes[i].pos.x + hitboxes[i].width)) { // check if x coord is inside
			if (p.y >= hitboxes[i].pos.y && p.y <= (hitboxes[i].pos.y + hitboxes[i].height)) { // check if y coord is inside
				return true;
			}
		}
	}
	return false;
}
void Entity::AddToGArry() {
	{
		if (DEBUG) {
			std::cout << "Entity Constructer Called" << std::endl;
		}
		if (LOADED_ENTITIES_HEAD > ENTITY_MAX) {
			// fuck
			// easy way out = crash
			if (EMAX_CRASH) {
				std::cerr << "EMAX_CRASH";
				exit(1);
			}
			else { // uh um hm help
				ent = &LOADED_ENTITIES[ENTITY_MAX]; // bad solution
				if (DEBUG) {
					std::cout << "Entity Overflow! Entity replaced end of array" << std::endl;
				}
			}
		}
		else {
			ent = &LOADED_ENTITIES[LOADED_ENTITIES_HEAD++];
			if (DEBUG) {
				std::cout << "Entity Loaded Succesfully" << std::endl;
			}
		}
		return;
	}
}
Entity::Entity() {
	AddToGArry();
}
Entity::Entity(EntityContainer in) {
	ent = &in;
}
void DrawEntity(int k){
	DrawEntity(LOADED_ENTITIES[k], k);
}
void DrawEntity(Entity ent){
	DrawEntity(*ent.ent);
}
void DrawEntity(EntityContainer ent, int k){
	if(!ent.hitboxes.size() && !DEBUG){
		return;
	}
	if(ent.dontDraw){
		return;
	}
	for (int i = 0; i < ent.hitboxes.size(); i++) {
		DrawTextureRec(ent.hitboxTexts[i],
			{ ent.offset.x , ent.offset.y , ent.hitboxes[i].width, ent.hitboxes[i].height },
			{ ent.hitboxes[i].pos.x , ent.hitboxes[i].pos.y },
			ent.tint);
			if (DRAW_DEBUG) {
				ent.debugDrawCounter += GetFrameTime();
				if (ent.debugDrawCounter >= 1) { // so console doesnt get spammed
					ent.debugDrawCounter = 0;
					std::cout << "At Draw : Entity : " << k << " Hitbox : " << i << " " << ent.hitboxes[i].pos.x << " " << ent.hitboxes[i].pos.y << std::endl;
				}
			}
			if (DEBUG){
				std::string name;
				name = std::to_string(k) + "." + std::to_string(i);
				DrawText(name.c_str(), ent.hitboxes[i].pos.x, ent.hitboxes[i].pos.y, 20, BLACK);
			}
	}
}
void DrawEntities() {
	for (int k = 1; k < LOADED_ENTITIES_HEAD; k++) {
		DrawEntity(k);
	}
	// draw entity 0 last, because it's typically the player
	DrawEntity(0);
}
void MoveEntities() {
	for (int i = 0; i < LOADED_ENTITIES_HEAD; i++) {
		if (!LOADED_ENTITIES[i].trigger) {
			for (int k = 0; k < LOADED_ENTITIES[k].hitboxes.size(); k++) {
				LOADED_ENTITIES[i].hitboxes[k].Move();
			}
		}
	}
}

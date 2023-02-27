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
	if (!LOADED_ENTITIES[k].isTrigger) {
		for (int i = 0; i < LOADED_ENTITIES[k].hitboxes.size(); i++) {
			DrawTextureRec(LOADED_ENTITIES[k].hitboxTexts[i],
				{ 0 , 0 , static_cast<float>(LOADED_ENTITIES[k].hitboxes[i].width), static_cast<float>(LOADED_ENTITIES[k].hitboxes[i].height) },
				{ static_cast<float>(LOADED_ENTITIES[k].hitboxes[i].pos.x) , static_cast<float>(LOADED_ENTITIES[k].hitboxes[i].pos.y) },
				LOADED_ENTITIES[k].tint);
			if (DRAW_DEBUG) {
				LOADED_ENTITIES[k].debugDrawCounter += GetFrameTime();
				if (LOADED_ENTITIES[k].debugDrawCounter >= 1) { // so console doesnt get spammed
					LOADED_ENTITIES[k].debugDrawCounter = 0;
					std::cout << "At Draw : Entity : " << k << " Hitbox : " << i << " " << LOADED_ENTITIES[k].hitboxes[i].pos.x << " " << LOADED_ENTITIES[k].hitboxes[i].pos.y << std::endl;
				}
			}
			if (DEBUG){
				std::string name;
				name = std::to_string(k) + "." + std::to_string(i);
				DrawText(name.c_str(), LOADED_ENTITIES[k].hitboxes[i].pos.x, LOADED_ENTITIES[k].hitboxes[i].pos.y, 20, BLACK);
			}
		}
	}
}
void DrawEntity(Entity ent){
	for (int i = 0; i < ent.ent->hitboxes.size(); i++) {
		DrawTextureRec(ent.hitboxTexts[i],
			{ 0 , 0 , static_cast<float>(ent.hitboxes[i].width), static_cast<float>(ent.hitboxes[i].height) },
			{ static_cast<float>(ent.hitboxes[i].pos.x) , static_cast<float>(ent.hitboxes[i].pos.y) },
			ent.tint());
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
		if (!LOADED_ENTITIES[i].isTrigger) {
			for (int k = 0; k < LOADED_ENTITIES[k].hitboxes.size(); k++) {
				LOADED_ENTITIES[i].hitboxes[k].Move();
			}
		}
	}
}

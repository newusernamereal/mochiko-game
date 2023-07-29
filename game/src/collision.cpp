#include "collision.hpp"
#include "include/screen.hpp"
#include "include/entity.hpp"
#include <algorithm>
bool CollisionsContain(int x, bool update, Entity &player){
	static std::optional<std::vector<int>> collisions = TriggerCollision(player); // cache each frame
	if(update)
		collisions = TriggerCollision(player);
	if(!collisions.has_value())
		return false;
	return (std::find(collisions.value().begin(), collisions.value().end(), x) != collisions.value().end());
}

bool CollisionsContain(EntityContainer ent, int x, bool update, Entity &player){
	static std::optional<std::vector<int>> collisions = TriggerCollision(Entity(ent)); // cache each frame
	if(update)
		collisions = TriggerCollision(Entity(ent));
	if(!collisions.has_value())
		return false;
	return (std::find(collisions.value().begin(), collisions.value().end(), x) != collisions.value().end());
}


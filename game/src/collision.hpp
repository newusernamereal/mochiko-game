#pragma once
#ifndef COLLISION_H
#define COLLISION_H
#include "include/entity.hpp"
bool CollisionsContain(int x, bool update = false);
bool CollisionsContain(EntityContainer ent, int x, bool update = false);
#endif

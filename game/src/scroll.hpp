#pragma once
#ifndef SCROLL_H
#define SCROLL_H
#include "include/engine.hpp"
#include "include/entity.hpp"
#include "include/screen.hpp"
#include "collision.hpp"
#define SCROLL_DEBUG false

void ScrollScreen(bool reset, Screen& currentScreen);
void MoveEverything(DoublePoint dir, Screen& currentScreen);

#endif

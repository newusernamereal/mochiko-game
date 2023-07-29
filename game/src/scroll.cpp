#include "scroll.hpp"
#include "globals.hpp"
#include "collision.hpp"
void ScrollScreen(bool reset, Screen &currentScreen){
	static DoublePoint pos = {0,0};
	static DoublePoint oldpos = pos;
	static Entity screenAnchor;
	static bool init = false;
	static Texture2D anchorText = LoadTexture("assets/transparent.png");
	static double upperBound = currentScreen.width - ((double)SCREENX/(double)currentScreen.tileSizeX); 
	// how far right you can scroll
	static double lowerBoundY = currentScreen.height - ((double)SCREENY/(double)currentScreen.tileSizeY);
	// how far down you can scroll
	if(!init){
		screenAnchor.addBox(EntityHitbox(0,0,1,1));
		screenAnchor.addTexture(anchorText);
		screenAnchor.DontDraw(true);
		init = true;
	}
	if (reset){
		std::cout << "resetting scroll" << std::endl;
		pos = {0,0};
		upperBound = currentScreen.width - ((double)SCREENX/(double)currentScreen.tileSizeX);
		screenAnchor.AddToGArry();
		screenAnchor.addBox(EntityHitbox(0,0,1,1));
		screenAnchor.DontDraw(true);
		screenAnchor.addTexture(anchorText);
		currentScreen.offset.x = screenAnchor.hitboxes[0].pos.x; 
		if(((oldpos.x - pos.x) || (oldpos.y - pos.y)) && SCROLL_DEBUG)
			std::cout << "new pos : " << pos.x << " , " << pos.y << std::endl;
		oldpos = pos;
		return;
	}
	// tile sky to save memory. there's definitely a better way to do this
	currentScreen.offset.x = screenAnchor.hitboxes[0].pos.x;
	if((currentScreen.offset.x + ((float)currentScreen.background.width / 3.0f)) > (float)SCREENX / 2)
		screenAnchor.hitboxes[0].pos.x -= (float)currentScreen.background.width / 3.0f;
	if((currentScreen.offset.x + (2.0f * (float)currentScreen.background.width / 3.0f)) < (float)SCREENX / 2)
		screenAnchor.hitboxes[0].pos.x += (float)currentScreen.background.width / 3.0f;

	if((CollisionsContain(1) || CollisionsContain(2)) && pos.x + SCROLLSTR <= upperBound && pos.x + SCROLLSTR >= 0){
		if ((CollisionsContain(1) && (std::abs(SCROLLSTR) / SCROLLSTR) == 1) ||
			(CollisionsContain(2) && (std::abs(SCROLLSTR) / SCROLLSTR) == -1)  ){
			MoveEverything(-1.0f * SCROLLSTR);
			pos.x += SCROLLSTR;
		}
		currentScreen.offset.x = screenAnchor.hitboxes[0].pos.x; // quick and dirty
	}	
	if((CollisionsContain(443) || CollisionsContain(449))){
//		&& pos.y + SCROLLSTRY <= lowerBoundY && pos.y + SCROLLSTRY >= 0){
		std::cout << " you're mother (line 308) " << std::endl; 
		if ((CollisionsContain(443) && (std::abs(SCROLLSTRY) / SCROLLSTRY) == -1) ||
			(CollisionsContain(449) && (std::abs(SCROLLSTRY) / SCROLLSTRY) == 1)  ){
			MoveEverythingY(-1.0f * SCROLLSTRY);
			pos.y += SCROLLSTRY;
		}
	}
	oldpos = pos;
}

void MoveEverything(DoublePoint dir, Screen &currentScreen){
	for(int i = 0; i < LOADED_ENTITIES_HEAD; i++){
		if(LOADED_ENTITIES[i].triggerID == 1 || LOADED_ENTITIES[i].triggerID == 2
		|| LOADED_ENTITIES[i].triggerID == 443 || LOADED_ENTITIES[i].triggerID == 449){
			if(++i < LOADED_ENTITIES_HEAD)
				return;	
		}
		for(int k = 0; k < LOADED_ENTITIES[i].hitboxes.size(); k++){
			LOADED_ENTITIES[i].hitboxes[k].pos.x += (int)(dir * currentScreen.tileSizeX);
			if (SCROLL_DEBUG)
				std::cout << "Moving hitbox " << k << " of entity " << i << " to " << LOADED_ENTITIES[i].hitboxes[k].pos.x << " (" << dir << " * " << currentScreen.tileSizeX << ") pixels" << std::endl;
		}
	}
}


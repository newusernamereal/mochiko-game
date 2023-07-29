#include "karen.hpp"
#include "globals.hpp"
void UpdateKarens(){
	for(int i = 0; i < LOADED_ENTITIES_HEAD; i++){
		if(LOADED_ENTITIES[i].triggerID == 10){
			KarenAI(i);
		}
	}
}

void KarenAI(int ent){
	static bool detected = false;
	static double timeSinceLastProj = 0;
	static double chaseTimer = 1;
	// detect player
	// if one of them detects the player, all of them see the player
	// it's a feature, not a bug
	chaseTimer += TICKTIME;
	timeSinceLastProj += TICKTIME;
	if(std::abs(LOADED_ENTITIES[ent].hitboxes[0].pos.x - player.hitboxes[0].pos.x) <= 3 * currentScreen.tileSizeX){
		chaseTimer = 0;
	}
	if (dead)
		chaseTimer = 1;
	detected = (0.5 >= chaseTimer);
	if(!detected)
		return;
	// move to the player
	if(timeSinceLastProj >= (double)(10.0f / AIDIFFICULTY) * 0.5f ){
		if(DEBUG)
			std::cout << "Attacked player" << " , " << LOADED_ENTITIES_HEAD << std::endl;
		timeSinceLastProj = 0;
		AttackPlayer(ent);
	}
	
}

void AttackPlayer(int source){
/*	static Texture2D badWords = LoadTexture("assets/bad_words.png");
	static Entity proj[30];
	static int i = 0;
	if(source == -1){
		for(int k = 0; k < 30; k++){
			// proj[k].ent->hitboxes[0].pos = {-10000,-10000};
			// proj[k].AddToGArry();
			proj[k] = Entity();
			proj[k].hitboxes[0].speed = {0,0};
			proj[k].hitboxes[0].pos = {-128,-128};
		}
	}
	for(int k = 0; k < 30; k++)
		proj[i].ent->triggerID = 3;
	proj[i].ent->hitboxes.clear();
	proj[i].ent->hitboxTexts.clear();
	proj[i].addBox(EntityHitbox(LOADED_ENTITIES[source].hitboxes[0].pos.x + 45, LOADED_ENTITIES[source].hitboxes[0].pos.y + 80, currentScreen.tileSizeX,currentScreen.tileSizeY));
	if(player.hitboxes[0].pos.x == LOADED_ENTITIES[source].hitboxes[0].pos.x){
		proj[i].hitboxes[0].speed = {0,-1};
	}                                   
	?"
	\VESPD * (1.0f / AIDIFFICULTY);
	const double slope = (player.hitboxes[0].pos.y - (sourceP.y)) / (player.hitboxes[0].pos.x - (sourceP.x));
	proj[i].hitboxes[0].speed = {
		// ((x2 - x1) / |x2 - x1|) * cos(tan^-1((y2 - y1) / (x2 - x1)))
		(double)speedMultiplier * (double)xParity * std::cos(std::atan2(slope,1)) ,
		// ((x2 - x1) / |x2 - x1|) * sin(tan^-1((y2 - y1) / (x2 - x1)))
		(double)speedMultiplier * (double)xParity * std::sin(std::atan2(slope,1))
	};
	std::cout << proj[i].hitboxes[0].speed.x << " , " << proj[i].hitboxes[0].speed.y << std::endl;
	
	proj[i].addTexture(badWords);
	i++;
	if(i == 30){
		i = 0;
	} */
}

void InitKarens(){
	// spooky
}

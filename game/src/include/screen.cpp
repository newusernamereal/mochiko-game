#include "screen.hpp"
Screen::Screen(Texture2D backin, std::vector<EntityContainer> entin) {
		background = backin;
		entities = entin;
}
void Screen::Load() {
	clearEntities();
	if (DEBUG)
		std::cout << "Screen Cleared \n";
	for (int i = 0; i < entities.size(); i++) {
		LOADED_ENTITIES[i] = entities[i];
	}
	LOADED_ENTITIES_HEAD = entities.size();
	if (DEBUG) {
		std::cout << "Entities Added to Entity Array \n";
		std::cout << "EArr Head : " << LOADED_ENTITIES_HEAD << std::endl;
	}
}
void Screen::ReadFromFile(std::string fileName, DoublePoint player) {
		(*this).fileName = fileName;
		std::ifstream file;
		file.open(fileName);
		std::vector<std::string> data;
		#ifdef CACHE_LEVEL_TEXTURES
			static std::vector<Texture2D> texts;
			static std::vector<std::string> loadedTexts;
		#endif
		std::vector<Texture2D> expectedTexts;
		bool textureCached = false;
		std::string currentLine;
		std::string currentNumber;
		int xcounter = 0;
		barriers.clear();
		EntityContainer empty;
		EntityContainer emptyBarrier;
		clearEntitiesExceptFirst();
		int datastart = -1;
		currentNumber = "0";
		if(DEBUG){
			std::cout << "at beginning of reading : " << std::endl;
			std::cout << "		barriers : ";
			for(int i = 0; i < barriers.size(); i++){
				std::cout << barriers[i] << " , ";
			}
			std::cout << "\n		entities : ";
			for(int i = 0; i < LOADED_ENTITIES_HEAD; i++){
				std::cout << i << " , ";
			}
			std::cout << std::endl;
		}
		int size;
		if (file.is_open()) {
			while (getline(file, currentLine)) {
				while (currentLine.find("\r") != std::string::npos) {
					currentLine.erase(currentLine.find("\r"), 1);
				}
				data.push_back(currentLine);
			}
		}
		file.close();
		try{
			size = std::stoi(data[0]); // first line is the size of the grid
		}catch(std::invalid_argument){
			std::cout << "Could not proccess '" << data[0] << "', defaulting to 1.." << std::endl;
			size = 1;
		}

		// find the start of the data
		for(int i = 0; i < data.size(); i++){
			if(data[i] == "DATA")
				datastart = i - 1;
		}
		// if it wasn't found default to assuming the map's height = size
		if(datastart == -1){
			datastart = (data.size() - (size + 1));
		}
		
		this->height = (data.size() - 1) - (data.size() - (size + 1));
		this->size = size;
		tileSizeX = (float)SCREENY / size;
		tileSizeY = (float)SCREENY / size;
		if(SQUARE_TILES){
			tileSizeX = tileSizeY;
		}
		// 1 to datastart is all textures
		for (size_t i = 1; i <= datastart; i++) {
			if(CACHE_LEVEL_TEXTURES){
				for (size_t k = 0; k < loadedTexts.size(); k++) {
					if (loadedTexts[k] == data[i]) {
						textureCached = true;
						break;
					}
				}
			}
			if (!textureCached || !CACHE_LEVEL_TEXTURES) {
				loadedTexts.push_back(data[i]);
				texts.push_back(LoadTexture(data[i].c_str()));
			}
			textureCached = false;
		}
		// add cached textures
		if(CACHE_LEVEL_TEXTURES){
			for (size_t i = 1; i <= (data.size() - (size + 1)); i++) {
				for (size_t k = 0; k < loadedTexts.size(); k++) {
					if (loadedTexts[k] == data[i]) {
						expectedTexts.push_back(texts[k]);
					}
				}
			}
		}
		// datastart to data.size() - 1 is all tile data
		for (size_t y = datastart; y < data.size(); y++) {
			xcounter = 0;
			for (size_t x = 0; x < data[y].size(); x++) {
				if (data[y][x] == 'p') {
					if (entities.size()) {
						if (entities[0].hitboxes.size()) {
							if (player.x == -1 && player.y == -1) {
								entities[0].hitboxes[0].pos = { static_cast<double>(xcounter * SCREENX / size), static_cast<double>((y - datastart - 2) * SCREENY / size) };
								if (DEBUG)
									std::cout << "\n\nPlayer X: " << entities[0].hitboxes[0].pos.x << "\nPlayer Y: " << entities[0].hitboxes[0].pos.y << "\n\n";
							}
							else {
								entities[0].hitboxes[0].pos = { player.x * SCREENX / size, player.y * SCREENY / size };
							}
						}
					}
				}
				else if (data[y][x] == '[') {
					currentNumber = "0";
					x++;
					while (data[y][x] != ']') {
						currentNumber += data[y][x];
						x++;
					}
					empty.trigger = true;
					try{
						empty.triggerID = std::stoi(currentNumber);
					}catch(std::invalid_argument){
						std::cout << "Could not proccess '" << currentNumber << "', defaulting to 0.." << std::endl;
						empty.triggerID = 0;
					}
					currentNumber = "0";				
				}
				else if (data[y][x] == '{') {
					currentNumber = "";
					x++;
					while (data[y][x] != '}') {
						currentNumber += data[y][x];
						x++;
					}
					while (currentNumber.find("\\n") != std::string::npos) {
						currentNumber.replace(currentNumber.find("\\n"), 2, "\n");
					}
					empty.signText = currentNumber;
					currentNumber = "0";
				}
				else if (data[y][x] == 'a'){
					empty.anim = true;
				}
				else if (data[y][x] == 'b') {
					barriers.push_back(entities.size());
				}
				else if (data[y][x] == 'B') {
					if (stoi(currentNumber) || empty.trigger || empty.signText != "") {
						if (stoi(currentNumber))
							empty.hitboxTexts.push_back(expectedTexts[stoi(currentNumber) - 1]);
						if(!SQUARE_TILES)
							empty.hitboxes.push_back(EntityHitbox((DoublePoint) { static_cast<double>(xcounter * SCREENY / size), static_cast<double>((y - datastart - 2)* SCREENY / size) }, SCREENX / size, SCREENY / size));
						if(SQUARE_TILES)
							empty.hitboxes.push_back(EntityHitbox((DoublePoint) { static_cast<double>(xcounter * SCREENY / size), static_cast<double>((y - datastart - 2)* SCREENY / size) }, SCREENY / size, SCREENY / size));
						entities.push_back(empty);
						empty.hitboxTexts.clear();
						empty.hitboxes.clear();
						empty.trigger = false;
						empty.triggerID = 0;
						empty.signText = "";
						empty.anim = false;
					}
					currentNumber = " ";
				}
				else if (data[y][x] == ',') {
					if (stoi(currentNumber) || empty.trigger || empty.signText != "") {
						if (stoi(currentNumber))
							empty.hitboxTexts.push_back(expectedTexts[stoi(currentNumber) - 1]);
						if(!SQUARE_TILES)
							empty.hitboxes.push_back(EntityHitbox((DoublePoint) { static_cast<double>(xcounter * SCREENY / size), static_cast<double>((y - datastart - 2)* SCREENY / size) }, SCREENX / size, SCREENY / size));
						if(SQUARE_TILES)
							empty.hitboxes.push_back(EntityHitbox((DoublePoint) { static_cast<double>(xcounter * SCREENY / size), static_cast<double>((y - datastart - 2)* SCREENY / size) }, SCREENY / size, SCREENY / size));
						entities.push_back(empty);
						empty.hitboxTexts.clear();
						empty.hitboxes.clear();
						empty.trigger = false;
						empty.triggerID = 0;
						empty.signText = "";
						empty.anim = false;
						empty.dontDraw = false;
					}
					currentNumber = " ";
					xcounter++;
				}
				else if(data[y][x] == 'i'){
					empty.dontDraw = true;
				}
				else {
					currentNumber += data[y][x];
				}
			}
			if (y > datastart + 1 && xcounter != width && DEBUG){
				std::cout << "width mismatch on line " << y << "!\n";
			}
			width = xcounter;
		}
	}
bool Screen::CheckMove(const Point to) {
		for (size_t i = 0; i < barriers.size(); i++) {
			if (LOADED_ENTITIES[barriers[i]].Colliding(to)) {
				return false;
			}
		}
		return true;
	}
bool Screen::CheckMove(const EntityContainer& to) {
		for (size_t i = 0; i < barriers.size(); i++) {
			if (LOADED_ENTITIES[barriers[i]].Colliding(to)) {
				return false;
			}
		}
		return true;
	}
bool Screen::CheckMove(const EntityHitbox& to) {
	for (size_t i = 0; i < barriers.size(); i++) {
		if (LOADED_ENTITIES[barriers[i]].Colliding(to)) {
			return false;
		}
	}
	return true;
}
std::optional<int> FindTrigger(int find){
	std::optional<int> ret; 
	for(int i = 0; i < LOADED_ENTITIES_HEAD; i++){
		if(LOADED_ENTITIES[i].triggerID == find){
			ret = i;
			return ret;
		}
	}
	ret.reset();
	return ret;
}
std::optional<std::vector<int>> FindTriggerV(int find){
	std::optional<std::vector<int>> ret; 
	ret.reset();
	for(int i = 0; i < LOADED_ENTITIES_HEAD; i++){
		if(LOADED_ENTITIES[i].triggerID == find){
			ret->push_back(i);
		}
	}
	return ret;
}
std::optional<std::vector<int>> TriggerCollision(const Entity& Player) {
	std::vector<int> out;
	for (int i = 0; i < LOADED_ENTITIES_HEAD; i++) {
		if (LOADED_ENTITIES[i].Colliding(Player.hitboxes[0]) && LOADED_ENTITIES[i].triggerID != 0) {
			out.push_back(LOADED_ENTITIES[i].triggerID);
		}
	}
	if(out.size())
		return out;
	return {};
}
std::optional<std::vector<int>> TriggerCollision(const EntityContainer& Player) {
	std::vector<int> out;
	for (int i = 0; i < LOADED_ENTITIES_HEAD; i++) {
		if (LOADED_ENTITIES[i].Colliding(Player.hitboxes[0]) && LOADED_ENTITIES[i].triggerID != 0) {
			out.push_back(LOADED_ENTITIES[i].triggerID);
		}
	}
	if(out.size())
		return out;
	return {};
}
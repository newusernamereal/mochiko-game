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
		static std::vector<Texture2D> texts;
		static std::vector<std::string> loadedTexts;
		std::vector<Texture2D> expectedTexts;
		bool textureCached = false;
		std::string currentLine;
		std::string currentNumber;
		int xcounter = 0;
		barriers.clear();
		EntityContainer empty;
		EntityContainer emptyBarrier;
		currentNumber = "0";
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
		size = std::stoi(data[0]); // first line is the size of the grid
		this->size = size;
		tileSizeX = (float)SCREENY / size;
		tileSizeY = (float)SCREENY / size;
		if(SQUARE_TILES){
			tileSizeX = tileSizeY;
		}
		// 1 to (data.size() - (size + 1)) is all textures
		for (size_t i = 1; i <= (data.size() - (size + 1)); i++) {
			for (size_t k = 0; k < loadedTexts.size(); k++) {
				if (loadedTexts[k] == data[i]) {
//					std::cout << loadedTexts[k] << " = " << data[i] << std::endl;
					textureCached = true;
					break;
				}
			}
			if (!textureCached) {
				loadedTexts.push_back(data[i]);
				texts.push_back(LoadTexture(data[i].c_str()));
			}
			textureCached = false;
		}
		for (size_t i = 1; i <= (data.size() - (size + 1)); i++) {
			for (size_t k = 0; k < loadedTexts.size(); k++) {
				if (loadedTexts[k] == data[i]) {
					expectedTexts.push_back(texts[k]);
				}
			}
		}
		//(data.size() - (size + 1)) to data.size() - 1 is all tile data
		for (size_t y = (data.size() - (size)); y < data.size(); y++) {
			xcounter = 0;
			for (size_t x = 0; x < data[y].size(); x++) {
				if (data[y][x] == 'p') {
					if (entities.size()) {
						if (entities[0].hitboxes.size()) {
							if (player.x == -1 && player.y == -1) {
								entities[0].hitboxes[0].pos = { static_cast<double>(xcounter * SCREENX / size), static_cast<double>((y - (data.size() - (size))) * SCREENY / size) };
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
					empty.triggerID = std::stoi(currentNumber);
					currentNumber = "0";				
				}
				else if (data[y][x] == '{') {
					currentNumber = "";
					x++;
					while (data[y][x] != '}') {
						if(DEBUG)
							std::cout << currentNumber << std::endl;
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
					emptyBarrier.hitboxTexts.clear();
					emptyBarrier.hitboxes.clear();
					emptyBarrier.triggerID = 0;
					if(!SQUARE_TILES)
						emptyBarrier.hitboxes.push_back(EntityHitbox((DoublePoint) { static_cast<double>(xcounter * SCREENY / size), static_cast<double>((y - (data.size() - (size)))* SCREENY / size) }, SCREENX / size, SCREENY / size));
					if(SQUARE_TILES)
						emptyBarrier.hitboxes.push_back(EntityHitbox((DoublePoint) { static_cast<double>(xcounter * SCREENY / size), static_cast<double>((y - (data.size() - (size)))* SCREENY / size) }, SCREENY / size, SCREENY / size));
					emptyBarrier.trigger = true;
					barriers.push_back(emptyBarrier);
				}
				else if (data[y][x] == 'B') {
					if (stoi(currentNumber) || empty.trigger || empty.signText != "") {
						if (stoi(currentNumber))
							empty.hitboxTexts.push_back(expectedTexts[stoi(currentNumber) - 1]);
						if(!SQUARE_TILES)
							empty.hitboxes.push_back(EntityHitbox((DoublePoint) { static_cast<double>(xcounter * SCREENY / size), static_cast<double>((y - (data.size() - (size)))* SCREENY / size) }, SCREENX / size, SCREENY / size));
						if(SQUARE_TILES)
							empty.hitboxes.push_back(EntityHitbox((DoublePoint) { static_cast<double>(xcounter * SCREENY / size), static_cast<double>((y - (data.size() - (size)))* SCREENY / size) }, SCREENY / size, SCREENY / size));
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
							empty.hitboxes.push_back(EntityHitbox((DoublePoint) { static_cast<double>(xcounter * SCREENY / size), static_cast<double>((y - (data.size() - (size)))* SCREENY / size) }, SCREENX / size, SCREENY / size));
						if(SQUARE_TILES)
							empty.hitboxes.push_back(EntityHitbox((DoublePoint) { static_cast<double>(xcounter * SCREENY / size), static_cast<double>((y - (data.size() - (size)))* SCREENY / size) }, SCREENY / size, SCREENY / size));
						entities.push_back(empty);
						empty.hitboxTexts.clear();
						empty.hitboxes.clear();
						empty.trigger = false;
						empty.triggerID = 0;
						empty.signText = "";
						empty.anim = false;
					}
					currentNumber = " ";
					xcounter++;
				}
				else {
					currentNumber += data[y][x];
				}
			}
		}
		width = xcounter;
	}
bool Screen::CheckMove(Point to) {
		for (size_t i = 0; i < barriers.size(); i++) {
			if (barriers[i].Colliding(to)) {
				return false;
			}
		}
		return true;
	}
bool Screen::CheckMove(EntityContainer to) {
		for (size_t i = 0; i < barriers.size(); i++) {
			if (barriers[i].Colliding(to)) {
				return false;
			}
		}
		return true;
	}
bool Screen::CheckMove(EntityHitbox to) {
	for (size_t i = 0; i < barriers.size(); i++) {
		if (barriers[i].Colliding(to)) {
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

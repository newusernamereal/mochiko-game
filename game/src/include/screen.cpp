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
		std::ifstream file;
		file.open(fileName);
		std::vector<std::string> data;
		static std::vector<Texture2D> texts;
		static std::vector<std::string> loadedTexts;
		std::vector<Texture2D> expectedTexts;
		bool flag = false;
		std::string currentLine;
		std::string currentNumber;
		int xcounter;
		barriers.clear();
		EntityContainer empty;
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
		tileSizeX = (float)SCREENX / size;
		tileSizeY = (float)SCREENY / size;
		// 1 to (data.size() - (size + 1)) is all textures
		for (int i = 1; i <= (data.size() - (size + 1)); i++) {
			for (int k = 0; k < loadedTexts.size(); k++) {
				if (loadedTexts[k] == data[i]) {
//					std::cout << loadedTexts[k] << " = " << data[i] << std::endl;
					flag = true;
					break;
				}
			}
			if (!flag) {
				loadedTexts.push_back(data[i]);
				texts.push_back(LoadTexture(data[i].c_str()));
			}
			flag = false;
		}
		for (int i = 1; i <= (data.size() - (size + 1)); i++) {
			for (int k = 0; k < loadedTexts.size(); k++) {
				if (loadedTexts[k] == data[i]) {
					expectedTexts.push_back(texts[k]);
				}
			}
		}
		//(data.size() - (size + 1)) to data.size() - 1 is all tile data
		for (int y = (data.size() - (size)); y < data.size(); y++) {
			xcounter = 0;
			for (int x = 0; x < data[y].size(); x++) {
				if (data[y][x] == 'p') {
					if (entities.size()) {
						if (entities[0].hitboxes.size()) {
							if (player.x == -1 && player.y == -1) {
								entities[0].hitboxes[0].pos = { static_cast<double>(xcounter * SCREENX / size), static_cast<double>((y - (data.size() - (size))) * SCREENY / size) };
								std::cout << "\n\nPlayer X: " << entities[0].hitboxes[0].pos.x << "\nPlayer Y: " << entities[0].hitboxes[0].pos.y << "\n\n";
							}
							else {
								entities[0].hitboxes[0].pos = { player.x * SCREENX / size, player.y * SCREENY / size };
							}
						}
					}
				}
				else if (data[y][x] == '[') { // this is an abomonation upon mankind, truly disgusting
					currentNumber = "0";
					x++;
					while (data[y][x] != ']') {
						currentNumber += data[y][x];
						x++;
					}
					empty.hitboxTexts.clear();
					empty.hitboxes.clear();
					empty.isTrigger = true;
					empty.triggerID = std::stoi(currentNumber);
					currentNumber = "0";
					empty.hitboxes.push_back(EntityHitbox((DoublePoint) { static_cast<double>(xcounter* SCREENX / size), static_cast<double>((y - (data.size() - (size)))* SCREENY / size) }, SCREENX / size, SCREENY / size));
					entities.push_back(empty);
				}
				else if (data[y][x] == 'b') {
					empty.hitboxTexts.clear();
					empty.hitboxes.clear();
					empty.triggerID = 0;
					empty.hitboxes.push_back(EntityHitbox((DoublePoint) { static_cast<double>(xcounter* SCREENX / size), static_cast<double>((y - (data.size() - (size)))* SCREENY / size) }, SCREENX / size, SCREENY / size));
					empty.isTrigger = true;
					barriers.push_back(empty);
				}
				else if (data[y][x] == ',') {
					if (stoi(currentNumber)) {
						empty.hitboxTexts.clear();
						empty.hitboxes.clear();
						empty.isTrigger = false;
						empty.triggerID = 0;
						empty.hitboxTexts.push_back(expectedTexts[stoi(currentNumber) - 1]);
						empty.hitboxes.push_back(EntityHitbox((DoublePoint) { static_cast<double>(xcounter * SCREENX / size), static_cast<double>((y - (data.size() - (size)))* SCREENY / size) }, SCREENX / size, SCREENY / size));
						entities.push_back(empty);
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
		for (int i = 0; i < barriers.size(); i++) {
			if (barriers[i].Colliding(to)) {
				return false;
			}
		}
		return true;
	}
bool Screen::CheckMove(EntityContainer to) {
		for (int i = 0; i < barriers.size(); i++) {
			if (barriers[i].Colliding(to)) {
				return false;
			}
		}
		return true;
	}
bool Screen::CheckMove(EntityHitbox to) {
	for (int i = 0; i < barriers.size(); i++) {
		if (barriers[i].Colliding(to)) {
			return false;
		}
	}
	return true;
}

int TriggerCollision(Entity& Player) {
	for (int i = 0; i < LOADED_ENTITIES_HEAD; i++) {
		if (LOADED_ENTITIES[i].Colliding(Player.hitboxes[0]) && LOADED_ENTITIES[i].triggerID != 0) {
			return LOADED_ENTITIES[i].triggerID;
		}
	}
	return 0;
}
Point FindTrigger(Screen & screen, int find) {
	Entity empty;
	empty.addBox(EntityHitbox(0, 0, screen.tileSizeX, screen.tileSizeY));
	for (int x = 0; x < SCREENX; x += screen.tileSizeX) {
		for (int y = 0; y < SCREENY; y += screen.tileSizeY) {
			if (TriggerCollision(empty) == find) {
				return { x,y };
			}
		}
	}
	return { 0,0 };
}

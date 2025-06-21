#include <fstream>
#include <iostream>

class Level {
 private:
  std::vector<std::vector<int>> map;

 public:
  Level(std::vector<std::vector<int>>& map) : map(std::move(map)) {};
};

int main() {
  std::vector<std::vector<int>> map(3, std::vector<int>(4, 0));

  std::ifstream file("map.txt");
  if (file.is_open()) {
    std::string line;
    for (size_t y = 0; std::getline(file, line); y++) {
      for (size_t x = 0; x < line.size(); x++) {
        map[y][x] = line[x] - '0';
      }
    }
  }
  file.close();

  auto other = std::move(map);
  std::cout << map.size() << '\n';

  for (int r = 0; r < 3; r++) {
    for (int c = 0; c < 4; c++) std::cout << other[r][c];
    std::cout << '\n';
  }
}

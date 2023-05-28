#include "Common.h"

#include <algorithm>
#include <chrono>

#include "Debug.h"

namespace marvin {

std::string Lowercase(const std::string& str) {
  std::string result;

  std::string name = str;

  //remove "^" that gets placed on names when biller is down
  if (!name.empty() && name[0] == '^') {
    name.erase(0, 1);
  }

  result.resize(name.size());

  std::transform(name.begin(), name.end(), result.begin(), ::tolower);

  return result;
}

std::string GetWorkingDirectory() {
  std::string directory;

  directory.resize(GetCurrentDirectory(0, NULL));

  GetCurrentDirectory(directory.size(), &directory[0]);

  return directory.substr(0, directory.size() - 1);
}

bool CheckStatus(GameProxy& game, KeyController& keys, bool use_max) {
  float max_energy = 0.0f;

  if (use_max) {
    max_energy = (float)game.GetShipSettings().MaximumEnergy;
  } else {
    max_energy = (float)game.GetShipSettings().InitialEnergy;
  }

  float energy_pct = ((float)game.GetEnergy() / max_energy) * 100.0f;
  bool result = false;

  if ((game.GetPlayer().status & 2) != 0) {
    game.Cloak(keys);
  } else if (energy_pct == 100.0f) {
    result = true;
  }

  return result;
}

std::size_t FindOpenFreq(const std::vector<uint16_t>& list, std::size_t start_pos) {
  std::size_t open_freq = 0;

  for (std::size_t i = start_pos; i < list.size(); i++) {
    if (list[i] == 0) {
      open_freq = i;
      break;
    }
  }

  return open_freq;
}

}  // namespace marvin

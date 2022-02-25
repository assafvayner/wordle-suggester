#include <unordered_set>
#include <unordered_map>
#include <cstdint>
#include <string>
#include <algorithm>
#include <vector>

using std::unordered_set;
using std::unordered_map;
using std::string;
using std::sort;
using std::stable_sort;
using std::vector;

namespace Suggester {
  vector<string> suggest(unordered_set<string>& options);
}

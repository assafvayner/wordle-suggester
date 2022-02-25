#include "Suggester.h"
#include <iostream>

unordered_map<char, int> counts_map;
unordered_map<string, uint64_t> ranks_cache;

static unordered_map<char, int> counts(unordered_set<string>& words) {
  unordered_map<char, int> res;
  for (const auto& word : words) {
    for (const auto& c : word) {
      res[c]++;
    }
  }
  return res;
}

static uint64_t num_unique_chars(const string& word) {
  unordered_set<char> seen;
  for (auto& c : word) {
    seen.insert(c);
  }
  return seen.size();
}

static bool is_vowel(const char c) {
  return c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u';
}

static int64_t rank(const string& word) {
  if (word == "") {
    return 0;
  }
  if (ranks_cache.find(word) != ranks_cache.end()) {
    return ranks_cache[word];
  }
  uint64_t res = 0;
  for (const auto& c: word) {
    res += counts_map[c];
    if (is_vowel(c)) {
      res++;
    }
  }
  res *= num_unique_chars(word);
  ranks_cache[word] = res;
  return res;
}

static bool comp(const string& str1, const string& str2) {
  uint64_t rank1, rank2;
  rank1 = rank(str1);
  rank2 = rank(str2);

  uint64_t diff = rank1 - rank2;
  if (diff == 0) {
    return str1 < str2;
  }
  return diff > 0;
};

vector<string> Suggester::suggest(unordered_set<string>& words) {
  counts_map = counts(words);
  ranks_cache = unordered_map<string, uint64_t>();
  
  vector<string> res(words.begin(), words.end());
  for (auto& word: res) {
    if (word.length() != 5) {
      std::cout << word.length() << " ";
      std::cout << word << std::endl;
    }
  }

  stable_sort(res.begin(), res.end(), comp);
  return res;
}

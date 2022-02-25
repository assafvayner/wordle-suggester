#include <cstdint>
#include <stdexcept>
#include <string>
#include <sstream>
#include <cstring>
#include <unordered_set>
#include <algorithm>

#define EXISTS_MASK 1
#define NO_POSITION 0
#define NUM_LETTERS 26
#define NUM_LETTERS_IN_WORD 5
#define INITIAL_CONDITION 0b00111110
#define BASELINE_LETTER 'a'

using std::invalid_argument;
using std::stringstream;
using std::string;
using std::unordered_set;
using std::fill_n;
using std::memcpy;

class Conditions {
public:
  Conditions();
  Conditions(Conditions& other);
  ~Conditions() {}; // do nothing destructor

  enum ConditionCode {
    SET_FINAL, SET_OFF, SET_POSITION_OFF, INVALID
  };
  struct Condition {
    ConditionCode code;
    char letter;
    uint8_t position; // ignored if Condition.code == SET_OFF or INVALID
  };
  
  // mutators
  void set_final(char letter, uint8_t position);
  void set_off(char letter);
  void set_position_off(char letter, uint8_t position);
  void set_condition(Condition* condition);
  
  // actions
  bool meets_conditions(string word);
  void filter_words(unordered_set<string>& words);


private:
  uint8_t char_positions_[NUM_LETTERS];
  char    finals_[NUM_LETTERS_IN_WORD];
};

int GetConditionFromString(string& input, Conditions::Condition* condition);

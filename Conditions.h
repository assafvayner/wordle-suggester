#include <cstdint>
#include <vector>
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
using std::vector;
using std::memcpy;


enum ConditionCode {
  SET_FINAL, SET_OFF, SET_POSITION_OFF, INVALID
};

struct Condition {
  ConditionCode code;
  char letter;
  uint8_t position; // ignored if Condition.code == SET_OFF or INVALID
};

class Conditions {
 public:
  Conditions();
  Conditions(Conditions& other);
  ~Conditions() = default;        // do nothing destructor

  // mutators:
  // note that across the this interface, letter must be between
  // 'a' to 'z' to be valid and position must be from 1 to 5,
  // if either condition is not met then an invalid_argument
  // exception is raised.
  //
  // note that for a word 'arise' a is at position 1, r at position 2
  // and so on and so forth.

  /*
   * set letter as definitely in the word at the given position
   * if another letter has already been set final in position
   * may become undefined state
   */
  void set_final(char letter, uint8_t position);

  /*
   *set that this letter does not appear in the word
   *if previously any other condition was applied with this letter,
   *this may be at an undefined state.
   */
  void set_off(char letter);
  
  /*
   *sets that letter is in the word but definitely not at position
   */
  void set_position_off(char letter, uint8_t position);

  /*
   *sets one of the possible conditions on this given the input pointer
   *to a condition struct
   */
  void set_condition(Condition* condition);
  
 
  // actions:
  
  /*
   *returns true if the given word matches meets the conditions set by
   *the state of this
   */
  bool meets_conditions(string word);

  /*
   *removes words from the input set which do not meet the conditions i.e.
   *for which this->meets_conditions returns false
   */
  void filter_words(unordered_set<string>& words);

 private:
  // for each possible letter we keep a status byte at index letter - BASELINE_LETTER
  // where each byte is of the form:
  // _________________________
  // |__|__|__|__|__|__|__|__|
  //  7   6  5  4  3  2  1  0
  //
  // bits 6 and 7 are ignored
  //
  // The bit in position 0 signifies if the given letter is guarenteed to exist in
  // the word. This bit is most conveniently checked using the EXISTS_MASK. If it is
  // a 0 bit then we do not know if the word exists solely on this bit
  //
  // for each bit 1-5 if it is a 1 then it is possible that the letter at the given index
  // exist in the word in the position corresponding to the bit (i.e. position 1, bit 1...)
  // if all bits 1-5 are off, then the letter is not found in the word.
  //
  uint8_t char_positions_[NUM_LETTERS];

  // letters which have been finalized
  // used to determine finalized letters without iterating over all letter and checking they
  // exist and only at one position.
  char    finals_[NUM_LETTERS_IN_WORD];
};

/*
 *Parses a string input into 1 condition or a sequence of set_off conditions
 *the structure of the string can be found in the README.md. If successful
 *the parsed conditions are pushed to the back of the vector given as a
 *vector pointer.
 *
 *returns true if successfully parsed and inserted, otherwise false
 */
bool GetConditionsFromString(string& input, vector<Condition>* condition_vec);


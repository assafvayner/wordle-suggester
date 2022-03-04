#include "Conditions.h"

Conditions::Conditions() {
  // set initial conditions, all positions allowed for all
  // letters and no letters finalized
  fill_n(char_positions_, NUM_LETTERS, INITIAL_CONDITION);
  fill_n(finals_, NUM_LETTERS_IN_WORD, 0);
}

Conditions::Conditions(Conditions& other) {
  // copy fields
  memcpy(other.char_positions_, this->char_positions_, sizeof(char) * NUM_LETTERS);
  memcpy(other.finals_, this->finals_, sizeof(char) * NUM_LETTERS_IN_WORD);
}

void Conditions::set_final(char letter, uint8_t position) {
  int letter_idx = letter - BASELINE_LETTER;
  if (position > 5 || position <= 0 || letter_idx < 0 || letter_idx >= NUM_LETTERS) {
    throw invalid_argument("position must be in [1, 5]\n\tletter must be in a-z");
  }
  char_positions_[letter_idx] |= EXISTS_MASK;
  finals_[position - 1] = letter;
}

void Conditions::set_off(char letter) {
  int letter_idx = letter - BASELINE_LETTER;
  if (letter_idx < 0 || letter_idx >= NUM_LETTERS) {
    throw invalid_argument("letter must be in a-z");
  }
  char_positions_[letter_idx] = NO_POSITION;
  // ensure not in finals_ arr
  for (int i = 0; i < NUM_LETTERS_IN_WORD; i++) {
    if (finals_[i] != letter) {
      continue;
    }
    finals_[i] = 0;
  }
}

void Conditions::set_position_off(char letter, uint8_t position) {
  int letter_idx = letter - BASELINE_LETTER;
  if (position > 5 || position <= 0 || letter_idx < 0 || letter_idx >= NUM_LETTERS) {
    throw invalid_argument("position must be in [1, 5]\n\tletter must be in a-z");
  }
  char_positions_[letter_idx] &= ~(1 << position);
  char_positions_[letter_idx] |= EXISTS_MASK;

  // check current options for letter
  // if no options, position_option => 0, call set_off
  // more than one option, position_option => -1, do nothing
  // otherwise only one option, position_option => position, call set_final
  int position_option = 0;
  for (int i = 1; i <= 5; i++) {
    if (!(char_positions_[letter_idx] & (1 << i))) {
      continue;
    }
    if (position_option == 0) {
      position_option = i;
      continue;
    }
    position_option = -1;
    break;
  }
  if (position_option > 0) {
    this->set_final(letter, position_option);
  } else if (position_option == 0) {
    this->set_off(letter);
  }
}

void Conditions::set_condition(Condition* condition) {
  switch (condition->code) {
    case ConditionCode::SET_FINAL: {
      this->set_final(condition->letter, condition->position);
      break;
    }
    case ConditionCode::SET_OFF: {
      this->set_off(condition->letter);
      break;
    }
    case ConditionCode::SET_POSITION_OFF: {
      this->set_position_off(condition->letter, condition->position);
      break;
    }
    default: {
      throw invalid_argument("invalid condition code");
      break;
    }
  }
}

void Conditions::filter_words(unordered_set<string>& words) {
  unordered_set<string>::iterator it, curr;
  for (it = words.begin(); it != words.end();) {
    curr = it++;
    if (!this->meets_conditions(*curr)) {
      // modify set reference, remove words
      words.erase(curr);
    }
  }
}

bool Conditions::meets_conditions(string word) {
  if (word.length() != NUM_LETTERS_IN_WORD) {
    return false;
  }
  uint8_t idx, condition;
  // check that each finalized letter is found at final position
  for (int i = 0; i < NUM_LETTERS_IN_WORD; i++) {
    if (finals_[i] && word[i] != finals_[i]) {
      return false;
    }
    idx = word[i] - BASELINE_LETTER;
    condition = char_positions_[idx];
    if (!(condition & (1 << (i + 1)))) {
      return false;
    }
  }
  // check for all letters that if they for sure exist in the word
  // that they are present in the word even if not finalized
  for (int letter_idx = 0; letter_idx < NUM_LETTERS; letter_idx++) {
    if (!(char_positions_[letter_idx] & EXISTS_MASK)) {
      continue;
    }
    char letter = BASELINE_LETTER + static_cast<char>(letter_idx);
    if (word.find(letter) == string::npos) {
      return false;
    }
  }
  return true;
}

// hash constexpr to allow for switch on a string
constexpr uint64_t dumbhash(const char* str, int length) {
  int hash = 1;
  for (int i = 0; i < length; i++) {
    hash += str[i];
    hash *= 7;
    hash &= 0xFFFFFFFF;
  }
  return hash;
}

bool GetConditionsFromString(string& input, vector<Condition>* condition_vec) {
  string command;
  stringstream ss(input);

  ss >> command;
  if (ss.fail() || ss.eof()) {
    return false;
  }

  char letter;
  int position;
  int res = false;
  switch (dumbhash(command.c_str(), command.length())) {
    case dumbhash("final", 5): {
      // condition final requires a char letter and a position
      ss >> letter;
      if (ss.fail() || ss.eof()) {
        return false;
      }
      ss >> position;
      if (ss.fail() || position <= 0 || position > 5) {
        break;
      }
      condition_vec->push_back(Condition{ConditionCode::SET_FINAL, letter, static_cast<uint8_t>(position)});
      res = true;
      break;
    }
    case dumbhash("off", 3): {
      // condition off requires a string of characters of length equal to
      // or greater than 1
      string letters;
      ss >> letters;
      if (ss.fail()) {
        break;
      }
      for (auto& c : letters) {
        if (c < BASELINE_LETTER || c >= BASELINE_LETTER + NUM_LETTERS) {
          continue;
        }
        condition_vec->push_back(Condition{ConditionCode::SET_OFF, c, 0}); // position proxy, ignored
      }
      res = true;
      break;
    }
    case dumbhash("pos", 3):
    case dumbhash("position", 8): {
      // condition position off requires a char letter and a position
      ss >> letter;
      if (ss.fail() || ss.eof()) {
        return false;
      }
      ss >> position;
      if (ss.fail() || position <= 0 || position > 5) {
        break;
      }
      condition_vec->push_back(Condition{ConditionCode::SET_POSITION_OFF, letter, static_cast<uint8_t>(position)});
      res = true;
      break;
    }
    default: {
      break;
    }
  }
  return res;
}

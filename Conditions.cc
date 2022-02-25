#include "Conditions.h"

int position_stat(uint8_t positions) {
  uint8_t mask = 0b00000010; // start at second bit since first is reserved
  int pos = 0;
  for (int i = 1; i <= NUM_LETTERS_IN_WORD; i++) {
    if (!(positions & mask++)) {
      continue;
    }
    if (pos) {
      return -1;
    }
    pos = i;
  }
  return pos;
}

Conditions::Conditions() {
  fill_n(char_positions_, NUM_LETTERS, INITIAL_CONDITION);
  fill_n(finals_, NUM_LETTERS_IN_WORD, 0);
}

Conditions::Conditions(Conditions& other) {
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
}

void Conditions::set_position_off(char letter, uint8_t position) {
  int letter_idx = letter - BASELINE_LETTER;
  if (position > 5 || position <= 0 || letter_idx < 0 || letter_idx >= NUM_LETTERS) {
    throw invalid_argument("position must be in [1, 5]\n\tletter must be in a-z");
  }
  char_positions_[letter_idx] &= ~(1 << position);
  char_positions_[letter_idx] |= EXISTS_MASK;
  int positions_c = position_stat(char_positions_[letter_idx]);
  if (positions_c == 0) {
    this->set_off(letter);
  } else if (positions_c > 0) {
    this->set_final(letter, positions_c);
  }
}

void Conditions::set_condition(Condition* condition) {
  switch (condition->code) {
    case Conditions::ConditionCode::SET_FINAL: {
      this->set_final(condition->letter, condition->position);
      break;
    }
    case Conditions::ConditionCode::SET_OFF: {
      this->set_off(condition->letter);
      break;
    }
    case Conditions::ConditionCode::SET_POSITION_OFF: {
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
      words.erase(curr);
    }
  }
}

bool Conditions::meets_conditions(string word) {
  if (word.length() != NUM_LETTERS_IN_WORD) {
    return false;
  }
  uint8_t idx, condition;
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

constexpr uint64_t dumbhash(const char* str, int length) {
  int hash = 1;
  for (int i = 0; i < length; i++) {
    hash *= 13;
    hash += str[i];
    hash &= 0xFFFF;
  }
  return hash;
}

int GetConditionFromString(string& input, Conditions::Condition* condition) {
  string command;
  char letter;
  stringstream ss(input);

  ss >> command;
  if (ss.fail() || ss.eof()) {
    condition->code = Conditions::ConditionCode::INVALID;
    return -1;
  }

  ss >> letter;
  if (ss.fail() || ss.eof()) {
    condition->code = Conditions::ConditionCode::INVALID;
    return -1;
  }

  int position;
  int res = -1;
  switch (dumbhash(command.c_str(), command.length())) {
    case dumbhash("final", 5): {
      ss >> position;
      if (ss.fail() || position <= 0 || position > 5) {
        condition->code = Conditions::ConditionCode::INVALID;
        break;
      }
      *condition = Conditions::Condition{Conditions::ConditionCode::SET_FINAL, letter, static_cast<uint8_t>(position)};
      res = 1;
      break;
    }
    case dumbhash("off", 3): {
      *condition = Conditions::Condition{Conditions::ConditionCode::SET_OFF, letter, 0}; // position proxy, ignored
      res = 1;
      break;
    }
    case dumbhash("pos", 3):
    case dumbhash("position", 8): {
      ss >> position;
      if (ss.fail() || position <= 0 || position > 5) {
        condition->code = Conditions::ConditionCode::INVALID;
        break;
      }
      *condition = Conditions::Condition{Conditions::ConditionCode::SET_POSITION_OFF, letter, static_cast<uint8_t>(position)};
      res = 1;
      break;
    }
    default: {
      break;
    }
  }
  return res;
}

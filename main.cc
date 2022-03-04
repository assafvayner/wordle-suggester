#include <cstdlib>
#include <sys/wait.h>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <unordered_set>
#include <string>
#include <filesystem>

#include "Conditions.h"
#include "Suggester.h"

#define MAX_COMMAND_ARG 3
#define WORDLE_WORDS "wordle_words.txt"
#define QUORDLE_WORDS "quordle_words.txt"
#define ALL_WORDS "words_alpha.txt"
#define WORDS_ZIP_NAME "words_alpha.zip"
#define WORDS_ZIP_URL "https://github.com/dwyl/english-words/raw/master/words_alpha.zip"

using std::any_of;
using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::filesystem::exists;
using std::ifstream;
using std::istream;
using std::unordered_set;
using std::string;


/*
 * ensures that the file WORDS_FILE name is present and if not will fetch the
 * zip file from WORDS_ZIP_URL (if it is not present) and unzip it to so that
 * the word file is present at the directory of the executable.
 *
 * returns true the file exists or was properly fetched, false otherwise
 */
bool ensure_words_file();

/*
 * takes a reference to an empty unordered_set and a string for the name of the
 * words file and reads each line in the file as if it is 1 word and if that word
 * is all lowercase alphabetic characters, will insert into the unordered_set.
 *
 * returns true if the whole file was parsed and the proper words were inserted
 * to the unordered_set, false if the file was not found or a read error occured.
 */
bool fill_words(unordered_set<string>& words, const string& words_file_name);

/*
 * prints the first 25 words in the words vector, or all of them if there are
 * less than 25.
 */
void print_top_25(vector<string>& words);

/*
 *determines the words file based on the argument for the words file character
 *W/w => wordle_words.txt, Q/q => quordle_words.txt, A/a => words_alpha.txt
 *if neither of the above options returns empty strings.
 */
string arg_to_words_file(char arg);

/*
 *prints and error message and exits
 */
void Usage();

int main(int argc, char* argv[]) {
  if (argc > 3 || argc < 2) {
    Usage();
  }

  string words_file_name = arg_to_words_file(argv[1][0]);

  // set in to either command file or cin depending on params.
  ifstream ifs;
  bool use_file = false;
  if (argc == 2) {
    if (!exists(argv[2])) {
      Usage();
    }
    use_file = true;
    ifs.open(argv[1]); // unhandled exception on failure
  }
  istream& in = use_file ? ifs : cin;

  // currently the file fetched by this function is not used unless you wish to use
  // alpha_words.txt for options. However ensure_words_file is a cool function
  // fetch files of English words from github
  if (words_file_name == ALL_WORDS && !ensure_words_file()) {
    return EXIT_FAILURE;
  }

  // init options, only narrowed down
  unordered_set<string> options;
  if (!fill_words(options, words_file_name)) {
    return EXIT_FAILURE;
  }

  Conditions conditions;
  vector<Condition> condition;
  
  string input;
  for (;;) {
    cout << endl << "input?:" << endl;
    getline(in, input);
    // if fail on non-eof we treat as fail and exit immediately
    if (!in.eof() && in.fail()) {
      cerr << "a read failed. ending program." << endl;
      return EXIT_FAILURE;
    }
    // exit conditions
    if (in.eof() || input == "exit") {
      cout << "thanks for playing!" << endl;
      break;
    }
    if (input == "show") {
      conditions.filter_words(options);
      vector<string> suggestions = suggester::suggest(options);
      print_top_25(suggestions);
      continue;
    }

    // read string as condition, could be a number of conditions
    if (!GetConditionsFromString(input, &condition)) {
      cerr << "bad user input" << endl;
    }
    for (auto& cond : condition) {
      if (cond.code == ConditionCode::INVALID) {
        cerr << "bad user input for a condition try again" << endl;
        continue;
      }
      conditions.set_condition(&cond);
    }
  }

  return EXIT_SUCCESS;
}

bool ensure_words_file() {
  int wait_res;

  // if file is present we are done
  if (exists(ALL_WORDS)) {
    return true;
  }
  
  char* const envp_empty[] = {nullptr};

  // fetch zip only if the zip is not already present
  if (!exists(WORDS_ZIP_NAME)) {
    if (!fork()) {
      // exec wget to get zip file
      char* const command[] = {
        const_cast<char*>("wget"),
        const_cast<char*>("-q"), // quiet
        const_cast<char*>(WORDS_ZIP_URL),
        nullptr // args must end in null pointer
      };
      execve("/usr/bin/wget", command, envp_empty);
      exit(EXIT_FAILURE);
    }

    // wait for child fetching process to be done
    wait(&wait_res);

    // if zip is still not present after we fetched then return error occured
    if (!exists(ALL_WORDS)) {
      return false;
    }
  }

  if (!fork()) {
    // exec unzip
    char* const command[] = {
      const_cast<char*>("unzip"),
      const_cast<char*>("-qq"),
      const_cast<char*>(WORDS_ZIP_NAME),
      nullptr // args must end in null pointer
    };
    execve("/usr/bin/unzip", command, envp_empty);
    cerr << "exec failed" << endl;
    exit(EXIT_FAILURE);
  }

  // parent wait until unzip is finished
  wait(&wait_res);

  // return if file exists
  return exists(ALL_WORDS);
}

bool fill_words(unordered_set<string>& words, const string& words_file_name) {
  // check file exists
  if (!exists(words_file_name)) {
    return false;
  }

  // open words file
  ifstream file;
  file.open(words_file_name);

  string line;

  // lambda function which returns true if character c is non-alphabetic
  // or upper case
  auto is_upper_or_nonalpha = [](char c) {
    return !isalpha(c) || isupper(c);
  };

  // loop condition: end of file
  while (!file.eof()) {
    // read line and error check
    file >> line;
    if (!file.eof() && file.fail()) {
      return false;
    }
    // don't insert undesireable word
    if (line.length() != 5 || any_of(line.begin(), line.end(), is_upper_or_nonalpha)) {
      continue;
    }
    words.insert(line);
  }
  return true;
}


void print_top_25(vector<string>& words) {
  int num_to_print = words.size() < 25 ? words.size() : 25; // min
  for (int i = 0; i < num_to_print; i++) {
    cout << words[i] << endl;
  }
}

void Usage() {
  cerr << "Usage: ./main <word bank w/q/a> <optionl commands file>" <<  endl;
  exit(EXIT_FAILURE);
}

string arg_to_words_file(char arg) {
  arg = tolower(arg);
  switch (arg) {
  case ('w'):
    return WORDLE_WORDS;
  case ('q'):
    return QUORDLE_WORDS;
  case ('a'):
    return ALL_WORDS;
  default:
    return "";  
  }
}

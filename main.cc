#include <stdlib.h>
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
#define WORDS_FILE_NAME "words_alpha.txt"
#define WORDS_ZIP_NAME "words_alpha.zip"
#define WORDS_ZIP_URL "https://github.com/dwyl/english-words/raw/master/words_alpha.zip"

using std::any_of;
using std::cerr;
using std::cout;
using std::cin;
using std::endl;
using std::filesystem::exists;
using std::ifstream;
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

int main(int argc, char* argv[]) {
  bool op_successful;

  // not used
  op_successful = ensure_words_file();
  if (!op_successful) {
    return EXIT_FAILURE;
  }

  unordered_set<string> options;
  if (!fill_words(options, WORDLE_WORDS)) {
    return EXIT_FAILURE;
  }

  Conditions conditions;
  Conditions::Condition condition;
  string input;
  for (;;) {
    cout << endl << "input?:" << endl;
    getline(cin, input);
    if (!cin.eof() && cin.fail()) {
      cerr << "a read from stdin failed. ending program." << endl;
      return EXIT_FAILURE;
    }
    if (cin.eof() || input == "exit") {
      cout << "thanks for playing!" << endl;
      break;
    }
    if (input == "show") {
      conditions.filter_words(options);
      vector<string> suggestions = Suggester::suggest(options);
      print_top_25(suggestions);
      continue;
    }
    GetConditionFromString(input, &condition);
    if (condition.code == Conditions::ConditionCode::INVALID) {
      cerr << "bad user input for a condition try again" << endl;
      continue;
    }
    conditions.set_condition(&condition);
  }

  return EXIT_SUCCESS;
}

bool ensure_words_file() {
  int wait_res;

  // if file is present we are done
  if (exists(WORDS_FILE_NAME)) {
    return true;
  }
  
  char* const envp_empty[] = {nullptr};
  
  if (!exists(WORDS_ZIP_NAME)) {
    if (!fork()) {
      // exec wget to get zip file
      char* const command[] = {const_cast<char*>("wget"),
                               const_cast<char*>("-q"),
                               const_cast<char*>(WORDS_ZIP_URL),
                               nullptr}; // args must end in null pointer
      execve("/usr/bin/wget", command, envp_empty);
      exit(EXIT_FAILURE);
    }

    // wait for child fetching process to be done
    wait(&wait_res);

    // if zip is still not present after we fetched then return error occured
    if (!exists(WORDS_ZIP_NAME)) {
      return false;
    }
  }


  if (!fork()) {
    // exec unzip
    char* const command[] = {const_cast<char*>("unzip"),
                             const_cast<char*>("-qq"),
                             const_cast<char*>(WORDS_ZIP_NAME),
                             nullptr}; // args must end in null pointer
    execve("/usr/bin/unzip", command, envp_empty);
    cerr << "exec failed" << endl;
    exit(EXIT_FAILURE);
  }

  // parent wait until unzip is finished
  wait(&wait_res);

  // return if file exists
  return exists(WORDS_FILE_NAME);
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
  int num_to_print = words.size() > 25 ? 25 : words.size();
  for (int i = 0; i < num_to_print; i++) {
    cout << words[i] << endl;
  }
}

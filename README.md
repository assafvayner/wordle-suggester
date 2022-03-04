# Wordle Word Suggester

## Preface
The goal of this project was to learn more C++ and actually use it for something. I am no expert in information theory or wordle, so the suggester heuristics may not be 'smart'.

## Details

This repo contains code for a command line interface to provide suggestions and narrow down words from for the game wordle.

This project is compiled using `make` and run using `./main <wordbank> <commands>` or omit the commands argument to use the CLI (recommended).

The first must be a character either `q/w/a` which will determine which word bank to use, for quordle, wordle or english words dictionary respectively.
Provided in the repo are 2 files `wordle_words.txt` and `quordle_words.txt` that refer to the whole word bank of wordle and quordle respectively taken in February 2022.
For the file of English words, from a fresh start or after a make clean the program may have to fetch and unzip the file which will require an internet connection as well as that the wget and unzip utilities are installed on the system.
Note however that this list of words contains many bogus words.

If a command file is provided then it will be used to feed commands to the program as opposed to the CLI (command line interface).
The file contents should be a series of commands that you would give the CLI, 1 per line. You can see an example in `sample.txt`.

The commands that you can pass to the CLI are:
- `show` prints the top 25 options or less if there are less options
- `exit` to exit the program, you can also hit ctrl+D instead.
- `off <letters>` this tells the program that the none of the letters (string of a-z characters) are found in the word.
- `final <letter> <position>` which states that the letter exists at position.
- `pos <letter> <position>` tells the program that the letter exists in the word but not at the position given.
  - can replace `pos` with the word `position`

position should always be in [1, 5] from left to right and letter from 'a' to 'z' inclusive.
Providing contradictory conditions causes undefined behavior, e.g. setting 'a' final at position 1 but then setting it as not found.

By default the file of words that is used as the sample of words to narrow down the options is set in the variable in main called: `WORDLE_WORDS`.
The file used must have 1 word per line.
Additionally you may specify `words_alpha.txt` which the program will download a zip from github and unzip it to give a more general list of English words.

The option sorting operates on 2 heuristics, first prioritizing words with letters which appear commonly across all options still left, and secondly giving priorities to words with more unique letters.
The goals of these heuristics is not to get a single answer quickly, but to narrow down the options.
It is advisable to look at the provided options but not necessarily choose the top option each time for best results.

This project was developed and tested on Rocky 8 Linux, results on other OS may vary.


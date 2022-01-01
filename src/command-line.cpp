#include "command-line.hpp"

Termsequel::CommandLineParser::CommandLineParser(char **argv, int argc) {
  for (int arg = 0; arg < argc; arg++) {
    this->arguments.push_back(argv[arg]);
  }
  this->current = 0;
}

bool Termsequel::CommandLineParser::has_next() {
  return current < arguments.size();
}

std::string Termsequel::CommandLineParser::parse() {
  return arguments[current++];
}
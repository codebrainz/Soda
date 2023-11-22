#include "soda.hpp"

#include <iostream>

int main(int argc, char **argv) {

  if (argc < 2) {
    for (auto const &tok : soda::tokenizer(std::cin)) {
      std::cout << tok << std::endl;
    }
  } else {
    for (int i = 1; i < argc; i++) {
      for (auto const &tok : soda::tokenizer(argv[i])) {
        std::cout << tok << std::endl;
      }
    }
  }

  return 0;
}

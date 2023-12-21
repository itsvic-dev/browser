#include "libhtml/parser.h"
#include <exception>
#include <fstream>
#include <iostream>

#define READ_CHUNK_SIZE 1024

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " <html>\n";
    return 1;
  }

  LibHTML::Parser parser;
  std::ifstream file(argv[1]);
  char chunk[READ_CHUNK_SIZE];

  while (true) {
    size_t read = file.readsome(chunk, READ_CHUNK_SIZE);
    if (read == 0)
      break;
    try {
      parser.parse(chunk, read);
    } catch (std::exception &exc) {
      std::cout << "[TEST FAIL] An exception has occurred during LibHTML "
                   "parsing of test case: "
                << exc.what() << std::endl;
      return -1;
    }
  }

  // let the parser know we're EOF'd now
  const wchar_t eof[] = {EOF};
  try {
    parser.parse(eof, 1);
  } catch (std::exception &exc) {
    std::cout << "[TEST FAIL] An exception has occurred during LibHTML parsing "
                 "of implied EOF: "
              << exc.what() << std::endl;
    return -1;
  }

  return 0;
}

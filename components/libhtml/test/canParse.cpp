#include "libhtml/parser.h"
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
    parser.parse(chunk, read);
  }

  // let the parser know we're EOF'd now
  const wchar_t eof[] = {EOF};
  parser.parse(eof, 1);

  return 0;
}

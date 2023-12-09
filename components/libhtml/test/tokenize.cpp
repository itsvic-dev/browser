#include "libhtml.h"
#include <fstream>
#include <iostream>

#define READ_CHUNK_SIZE 512

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " <html>\n";
    return 1;
  }

  LibHTML::Tokenizer tokenizer;
  std::ifstream file(argv[1]);
  char chunk[READ_CHUNK_SIZE];

  while (true) {
    size_t read = file.readsome(chunk, READ_CHUNK_SIZE);
    if (read == 0)
      break;
    tokenizer.process(chunk, read);
  }

  // let the tokenizer know we're EOF'd now
  const char eof[] = {EOF};
  tokenizer.process(eof, 1);

  return 0;
}

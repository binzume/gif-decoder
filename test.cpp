#include <iostream>
#include <fstream>
#include "GIFDecoder.h"

struct DummyPixelWriter {
  int width, height, pos;
  void setSize(uint16_t w, uint16_t h) {
    std::cout << w << "x" << h << std::endl;
    width = w; height = h; pos = 0;
  }
  void setPalette(uint8_t c, uint8_t r, uint8_t g, uint8_t b) {
  }
  void writePixel(uint8_t c) {
    pos++;
    std::cout << (int)c;
    if(pos % width == 0) {
      std::cout << std::endl;
    }
  }
};

struct FileByteReader {
  std::ifstream &in;
  FileByteReader(std::ifstream &_in): in(_in) {}
  bool eof() {
    return !in;
  }
  uint8_t readByte() {
    char b;
    in.read(&b, 1);
    return (uint8_t)b;
  }
};

int main() {
    std::ifstream in = std::ifstream("test.gif", std::ios_base::binary);
    FileByteReader r(in);
    DummyPixelWriter w;
    GIFDecoder<FileByteReader, DummyPixelWriter> decoder(r, w);
    int err = decoder.decode();
    if (err) {
          std::cout << "error " << err << std::endl;
    }
    in.close();
    return 0;
}

#include <cstdint>

template <typename Reader, typename Writer>
class GIFDecoder {
  Reader reader;
  Writer writer;
  uint8_t read8() { return reader.readByte(); }
  uint16_t read16() { return (uint16_t)read8() | (uint16_t)read8() << 8; }
  void skip(int n) {
    for (int i = 0; i < n; i++) {
      read8();
    }
  }

  // LZW block
  uint8_t block_nbuf, block_sz;
  uint32_t block_buf;
  void resetBlock() {
    block_sz = 0;
    block_buf = 0;
    block_nbuf = 0;
  }
  int readBlockBits(int n) {
    while (block_nbuf < n) {
      if (block_sz == 0) block_sz = read8();
      block_buf = ((uint32_t)read8() << block_nbuf) | block_buf;
      block_sz--;
      block_nbuf = block_nbuf + 8;
    }
    int r = ((1 << n) - 1) & block_buf;
    block_buf = block_buf >> n;
    block_nbuf = block_nbuf - n;
    return r;
  }

  int16_t table_rest[4096];
  uint8_t table_last[4096];
  uint8_t tmp[4096];  // buffer to reverse
  uint8_t getFirst(int c) const {
    uint8_t last;
    do {
      last = table_last[c];
      c = table_rest[c];
    } while (c != -1);
    return last;
  }

  void writePixels(int c) {
    int i = 0, rest = c;
    while (rest != -1) {
      tmp[i] = table_last[rest];
      rest = table_rest[rest];
      i++;
    }
    for (i--; i >= 0; i--) {
      writer.writePixel(tmp[i]);
    }
  }
  bool ispot(int x) const { return (x & (x - 1)) == 0; }

 public:
  GIFDecoder(Reader r, Writer w) : reader(r), writer(w) {}

  int decode() {
    const char *magic = "GIF89a";
    for (int p = 0; magic[p]; p++)
      if (read8() != magic[p]) return 2;
    writer.setSize(read16(), read16());
    const uint8_t flags = read8();
    skip(2);
    const uint8_t colbits = 1 + (flags & 7);
    const int colors = 1 << colbits;
    const int clr = colors;
    const int end = 1 + colors;
    int free = 1 + end;
    uint8_t bits = 1 + colbits;

    for (int c = 0; c < colors; c++) {
      writer.setPalette(c, read8(), read8(), read8());
    }

    // init table
    for (int c = 0; c < colors; c++) {
      table_rest[c] = -1;
      table_last[c] = c;
    }
    while (!reader.eof()) {
      uint8_t tag = read8();
      if (tag == 0x2C) {
        skip(8);
        if ((read8() & 0x80) != 0) return 3;
        skip(1);
        resetBlock();
        bool stop = false;
        int code = -1, last = -1;
        do {
          last = code;
          code = readBlockBits(bits);
          if (code == clr) {
            free = 1 + end;
            bits = 1 + colbits;
            code = -1;
          } else if (code == end) {
            stop = true;
          } else if (last == -1) {
            writePixels(code);
          } else if (code <= free) {
            table_rest[free] = last;
            table_last[free] = getFirst(code == free ? last : code);
            writePixels(code);
            free++;
            if (ispot(free) && (free < 4096)) bits++;
          }
        } while (!stop);
        if (read8() != 0) return 4;
      } else if (tag == 0x21) {
        skip(1);
        int length = read8();
        skip(1 + length);
      } else if (tag == 0x3b) {
        break;
      }
    }
    return 0;
  }
};

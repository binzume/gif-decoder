フットプリントの小さなGIFデコーダーです。

ArduinoやESP32上でGIFをデコードしながらピクセルデータをディスプレイなどのデバイスに書き込むことを意図して実装しています。

# 特徴：

- 出力先のフレームバッファが不要
  - `writePixel(col)` が左上のピクセルから順番に呼び出されます
- 入力するGIFデータもメモリ上に無くてもよい
  - 読み込み用の `readByte()` と `eof()` だけ実装すれば良いです
- LZWの辞書を小さくするのは [この記事](http://www.technoblogy.com/show?45YI) を参考にしました。
- アニメーションやインターレースGIFは対応していません

# 利用方法

用途に合わせてByteReaderとPixelWriterを実装して、デコード時に渡してください。
実装方法は test.cpp を参照。
最低限16KB+αのRAMが必要です。

# 参考

- http://www.technoblogy.com/show?45YI
- https://en.wikipedia.org/wiki/GIF
- https://www.tohoho-web.com/wwwgif.htm#GIFHeader

# License

MIT

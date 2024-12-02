Buddy system で確保できるメモリーブロックのサイズは最小でページ一つ分の 4 KiB である。
だが、プログラムではより小さなメモリサイズを扱うことがほとんどであり、その場合に internal fragmentation を生じさせる可能性がある。

そこで Slab allocator が登場する。

- SLAB
- SLUB
  SLAB をシンプルにしつつスケーラビリティを確保した改良版。デフォルトの slab allocator になっている。
- SLOB
  small footprint

という 3 種類の Slab Allocator が用意されている。
カーネルのビルド時にそのうちどれを選択するかを選べるようになっている。

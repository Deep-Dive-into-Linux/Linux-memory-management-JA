# Overview

Buddy Memory Allocation とは、External fragmentation を最小限に抑えるためのアルゴリズム
External fragmentation とは、空き領域全体としては十分あるのに、連続している空き領域が少ないこと。

External fragmentation の削減アルゴリズムには、Buddy Memory Allocation 以外にも bit map algorithm もあるが、以下の理由で Buddy Memory Allocation が採用されている。

- bit map algorithm
- Buddy Memory Allocation

# How it works

このアルゴリズムはメモリの割り当てと解放をブロック単位で行う。2 の冪乗のサイズである必要があり、Linux では 4 Kib (2^12 bytes) と定義されている。
4 KiB というサイズは、Linux のページング機構で使われる最小のページサイズに相当し、これによって Physical memory と Virtual memory の間でのマッピングを効率的に行うことができる。

メモリブロックが解放されると、前後に空きブロックがないかを確認して、存在する場合はマージして、できるだけ大きなブロックを作成する。

Buddy system では 4 KiB (1 page) ~ 4 MiB (1,024 pages) の 11 種類のサイズのメモリーブロックを管理する。
何種類のブロックを管理するかは、include/linux/mmzone.h で指定する。

https://github.com/torvalds/linux/blob/adc218676eef25575469234709c2d87185ca223a/include/linux/mmzone.h#L30C24-L30C26

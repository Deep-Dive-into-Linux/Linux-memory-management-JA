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

- https://github.com/torvalds/linux/blob/v6.12/include/linux/mmzone.h#L30

page 構造体

- include/linux/mm_types.h - struct page
  https://github.com/torvalds/linux/blob/v6.12/include/linux/mm_types.h#L72-L219

page 構造体の pageflags フィールド

- struct page's pageflags
  https://github.com/torvalds/linux/blob/v6.12/include/linux/page-flags.h#L93-L190

- memory zone

  - ZONE_DMA
    physical memory の先頭から 16 M bytes まで
  - ZONE_DMA32
    physical memory の先頭から 4 G bytes までのうち、ZONE_DMA を除く部分
  - ZONE_NORMAL
    physical memory のうち、ZONE_DMA、ZONE_DMA32 を除く zone

Buddy system を通じて、physical memory 領域を確保する `alloc_pages` 関数を呼び出す場合は、第一引数に memory zone を示す zone flag を設定する。

# external memory fragmentation の確認

```sh
[ec2-user@ip-192-168-0-36 ~]$ cat /etc/os-release
NAME="Amazon Linux"
VERSION="2023"
ID="amzn"
ID_LIKE="fedora"
VERSION_ID="2023"
PLATFORM_ID="platform:al2023"
PRETTY_NAME="Amazon Linux 2023.6.20241121"
ANSI_COLOR="0;33"
CPE_NAME="cpe:2.3:o:amazon:amazon_linux:2023"
HOME_URL="https://aws.amazon.com/linux/amazon-linux-2023/"
DOCUMENTATION_URL="https://docs.aws.amazon.com/linux/"
SUPPORT_URL="https://aws.amazon.com/premiumsupport/"
BUG_REPORT_URL="https://github.com/amazonlinux/amazon-linux-2023"
VENDOR_NAME="AWS"
VENDOR_URL="https://aws.amazon.com/"
SUPPORT_END="2028-03-15"
[ec2-user@ip-192-168-0-36 ~]$ uname -a
Linux ip-192-168-0-36.ec2.internal 6.1.115-126.197.amzn2023.x86_64 #1 SMP PREEMPT_DYNAMIC Tue Nov  5 17:36:57 UTC 2024 x86_64 x86_64 x86_64 GNU/Linux
```

/proc/buddyinfo を見ると、現在の空きページの状態が分かる。

この例では、DMA ゾーンの 1 << 0 (4 KiB) に 0 個、....1 << 7 (128 KiB) に 1 個、1 << 8 (256 KiB) に 0 個、1 << 9 (512 KiB) に 1 個、1 << 10 (1024 KiB) に 3 個の空きがある。
External fragmentation が起きると、大きな塊 (1 << 10 とか 1 << 9 とか) が少なくなる。
DMA とか CPU を介さずに Physical memory に直接アクセスする場合は、この fragmentation が大きいと困る。

```sh
[ec2-user@ip-192-168-0-36 ~]$ cat /proc/buddyinfo
Node 0, zone      DMA      0      0      0      0      0      0      0      1      0      1      3
Node 0, zone    DMA32      4      6      5      6      6      8      5      7      6      4    723
Node 0, zone   Normal    315    154     97     86    160     71     35     23     14      8     35
[ec2-user@ip-192-168-0-36 ~]$
```

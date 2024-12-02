Linux では Paging を使用し、virtual memory を管理している。
Physical memory も同様に管理が必要である。

- Physical memory を固定長の page frame に分割する。
- プロセスの virtual address 空間は、page の並びに分割する。
- page の大きさと page frame の大きさは、同じ。
- 任意のページは、任意の page frame にマップできる。

# Physical memory の管理が必要な理由

- カーネル用の virtual memory 領域の大部分で、physical memory を連続的に割り当てている (straight map しているため)
  physical memory を straight map する direct map 領域はもちろんだが、vmalloc/ioremap 領域を除く他の領域でも、physical memory を straight map している。

Physical memory

| kinds           | description                                                    |
| --------------- | -------------------------------------------------------------- |
| others          | physical memory のさまざまな特定領域が straight map される領域 |
| vmalloc/ioremap | paging によって動的に physical page が割り当てられる領域       |
| direct map      | physical memory 全体が straight map される領域                 |

- DMA (Direct memory access) controller の存在も理由の一つ
  DMA controller は、一般に MMU を介さずに physical memory に直接アクセスする。
  これを管理するのにも、physical memory の管理は必要になる。

- fragmentation を避ける。
  - internal fragmentation
    不必要で実際には使用しない領域まで確保してしまい、メモリ使用効率が下がること。
    -> Slab allocation で対応
  - external fragmentation
    連続した空きメモリ領域を確保できにくくなること。
    -> Buddy system で対応

# kernel code reading

include/linux/mm_types.h

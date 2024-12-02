Linux のメモリ管理についての概要を学ぶ。

# アドレス管理

メモリ管理を語る上でアドレスの話は避けられない。
アドレスにはデータが入っている。アドレスは場所を指し、アドレスにはデータが格納されている。

つまり、正しくアドレスを参照すればデータの中身を見ることは可能である。

Physical memory といっても色々あるけど、例えば [DDR4 SDRAM](https://en.wikipedia.org/wiki/DDR4_SDRAM) とかがある。Physical memory は重要だけど、ここでは深入りしない。

では、この Physical memory をどうやってアプリケーション・OS 側で扱うというのが課題になってくる。
Linux を含む多くの OS、Windows や 組み込み OS は、virtual address と physical address を使用する。

アプリケーションからは virtual address を参照するけど、OS や CPU の力を借りて Physical address に変換されるというわけだ。

ちなみに、組み込みシステムによっては、Physical address を直接指定するものもある。世の中のシステムが全て virtual address から physical address に変換されるというわけではないので補足。

## Virtual memory の必要性

シンプルに何で、physical address を直接参照してはいけないのか。アドレス変換する時間を省けるし、効率も良さそうに思えるが、主に以下の理由で virtual memory は使用される。

1. memory fragmentation を解消する
   memory には主に使用領域と空き領域がある。そして、空き領域は多くの場合、連続したスペースが欲しい場合が多い。

2. マルチプロセスの実現させる
   複数のプロセスが同時に実行される時、各プロセスがお互いの領域を侵犯せず実行するには、各プロセスが自身のメモリ使用可能なアドレス範囲を把握しておく必要がある。
   全てのアプリケーションがどのメモリ領域にアクセスするのかを事前に把握しておくというのは、現実的でないしまず無理だろう。
   サードパーティ製品とかが混在する OS では、それぞれの製品はお互いの製品の事に気を使って作成されたものでもないだろうし。

3. 不正なメモリ領域へアクセスさせない
   アプリケーション側であらかじめ物理メモリのアドレスを把握してアクセスするのは大変。

上記課題を解決・実現するために Virtual memory が使用される。

Linux でもこの Virtual memory の話が重要になってくる。

## virtual memory と physical memory の変換

virtual memory と physical memory の変換表の事をページテーブルという。ページテーブル自体はカーネルのメモリ上に存在する。
x86_64 の場合、CPU の制御レジスタ (cr3) にページテーブルの位置を示す情報 (Physical address など) を持っておく。

- https://www.kernel.org/doc/gorman/html/understand/understand006.html

  ```
  On the x86, the process page table is loaded by copying mm_struct→pgd into the cr3 register which has the side effect of flushing the TLB. In fact this is how the function __flush_tlb() is implemented in the architecture dependent code.
  ```

- https://en.wikipedia.org/wiki/Translation_lookaside_buffer

CPU <-> virtual address <-> MMU <-> Physical address

毎回 virtual address と Physical address の変換を行うのは効率悪いので、Translation Look Aside Buffer (TLB) という MMU の機能の一部を使用して変換を効率化している。

### MMU の特徴

- Virtual memory と Physical memory の変換
- 誤ったアドレスへの参照を防止
  プロセスごとにページテーブルを持っている。なので、各プロセスが同じ仮想テーブルを参照することはない。
  Inter process communicating (IPC) を使用してプロセス間通信をしたりすることは可能。

- include/linux/sched.h - struct task_struct
  https://github.com/torvalds/linux/blob/v6.12/include/linux/sched.h#L778-L1610

## Virtual address に Physical address にアドレスが割り当てられていない場合

上記ページテーブルは、プロセスごとに virtual address と physical address が割り当てられている。
ただし、このページテーブルはプロセス生成後に作成されるわけではない。

Virtual address にアクセスした際、Virtual address に対応する Physical address が存在しない場合、例外が発生する。

例外をトリガーとして実際の Physical memory 領域を確保する。
このような動きを demand paging という。

## memory maps

page table は各プロセスごとに作成される。
linear address で表現される virtual memory 空間をそのまま割り当て、分割された領域の前半をプロセス用 (user land で使用)、後半をカーネル用として利用する。前半部分には各プロセスで独立したデータを保持できるが、後半部分は全プロセスで共通のデータを保持することになる。

各プロセスの virtual memory 空間にカーネル用の領域を割り当てるのは、処理速度向上のための工夫がある。
カーネル用の領域にはシステム管理上重要な情報が格納されているため、一部を除いてプロセスからはアクセスできないようになっている。

カーネルモードに切り替わった時だけカーネル用の領域を割り当てる「PTI (Page Table Isolation)」という仕組みを採用している。

## page table の階層化

ページテーブルはどのくらいの量のメモリを消費するのか。

x86_64 において、virtual address 空間の大きさは 128 TiB で、1 page の大きさは 4 KiB 、page table entry のサイズは 8 bytes。

- virtual address 空間の大きさは 128 TiB
- 1 page の大きさは 4 KiB
- page table entry のサイズは 8 bytes

- ページ数
  128 TiB / 4 KiB = 32,000,000 pages
- プロセス 1 つあたりの page table に必要な大きさ (単純計算)
  256 GiB (= 8 bytes x 128 TiB / 4 KiB)

単純計算した場合、1 つのプロセスも生成できないということになる。

↓
↓
↓

page table を階層化する。
仮想メモリ量がある程度多くなると、フラットな page table よりもメモリ使用量が大きくなる。しかし、そのようなことは稀なので、全プロセスの page table に必要な合計メモリの量は、フラットな page table よりも階層型 page table の方が小さくなることがほとんど。

x86_64 アーキテクチャーでは、ページサイズが 4 KiB の場合ページテーブルの段数は 4 段階、Ice Lake (microprocessor) 以降だと 5 段に設定することが可能。

### システムが使用している物理メモリのうち、page table として使用しているメモリ

`sar -r ALL` コマンドの kbpgtbl field から得られる。

- http://man.he.net/man1/sar
  ```
  kbpgtbl
    Amount  of  memory  in  kilobytes dedicated to the lowest
    level of page tables.
  ```

### huge page and transparent huge page

Huge page は、Linux かねるが最新のハードウェアアーキテクチャーのページサイズ機能を複数利用できるようにする機能。

- Huge Page とは何ですか? また、それを使用する利点は何ですか?
  https://access.redhat.com/ja/solutions/293173

  ```
  Linux は、トランスレーションルックアサイドバッファー (TLB) と呼ばれる CPU アーキテクチャーのメカニズムを使用して、仮想メモリーページの実際の物理メモリーアドレスへのマッピングを管理します。TLB のハードウェアリソースには限りがあるため、デフォルトのページサイズで大量の物理メモリーを利用すると TLB が消費され、処理のオーバーヘッドが追加されます。サイズが 4096 バイトのページが多数あると、多くの TLB リソースが消費されます。Huge page を利用することで、サイズがより大きいページを作成して、ページごとにTLB 内の 1 つのリソースを消費できます。Huge page の作成の副作用として、Huge page にマップされた物理メモリーは通常のメモリー割り当ての対象ではなくなり、カーネル仮想メモリーのマネージャーで管理されなくなるため、Huge page は実質的に「保護」され、Huge page を要求するアプリケーションに対してのみ提供されます。Huge Page は物理 RAM に固定されており、スワップ/ページアウトできません。
  ```

- huge page のメリット

  - page table 処理の削減
    huge table では、デフォルトのページよりも大きなサイズでメモリ空間を管理する。そのため、デフォルトの page size (4 KiB) よりもページヒット率が高くなり、virtual address から physical address へ変換するための CPU オーバーヘッドが軽減される。
  - page 数自体の削減
    huge page では 1 entry (1 page) で管理できるメモリ情報が大きくなるため、page 数が削減され、page table 管理のためのメモリ使用量も減少する。

  - Huge page の大きさは以下のようにして確認することが可能。一般的な x86 システムの Huge page サイズは 2048 kB になる。

    ```sh
    cat /proc/meminfo | grep Hugepagesize
    ```

  - huge page は mmap 関数の flags 引数に MAP_HUGETLB フラグを与えるなどすれば獲得できる。

- transparent huge page
  huge page はメモリ獲得時に逐一要求する実装をする必要がある
  ↓

  - メリット
    transparent huge page という Linux の機能を使えば楽になる。
    virtual address 空間内の連続する複数の 4 KiB ページが所定の条件を満たせば、それらをまとめて自動的に huge page にしてくれるというもの。
  - デメリット
    複数のページをまとめて huge page にする処理、および前述の条件を満たさなくなったときに huge page を 4 KiB に再分解する処理によって、局所的に性能が劣化する場合がある。このため、これらの設定はシステム管理者が変更できるようになっている。

  - 有効・無効の確認
    有効になっているかどうかは以下のファイルを参照することで確認できる。デフォルトは、madvise。

```sh
$ cat /sys/kernel/mm/transparent_hugepage/enabled
always [madvise] never
```

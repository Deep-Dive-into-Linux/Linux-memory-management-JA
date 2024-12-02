Buddy system で確保できるメモリーブロックのサイズは最小でページ一つ分の 4 KiB である。
だが、プログラムではより小さなメモリサイズを扱うことがほとんどであり、その場合に internal fragmentation を生じさせる可能性がある。
例えば、以下プログラム上では、32 bits (4 bytes) の要素を 10 個格納できる配列をスタック領域 (メモリ空間上の領域)に確保してる。つまり、4 bytes x 10 個で 40 bytes の容量があればいい。でも、この配列を確保するのに 4 KiB 割り当てるとかなると、シンプルにリソースが勿体無い。

```c
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    int32_t arrray[10]= {0,1,2,3,4,5,6,7,8,9};
    printf("%d: %d\n", sizeof(arrray), sizeof(int32_t));
    return 0;
}
```

```sh
[ec2-user@ip-192-168-0-36 ~]$ gcc main.c -o main
[ec2-user@ip-192-168-0-36 ~]$ ./main
40: 4
[ec2-user@ip-192-168-0-36 ~]$
```

そこで Slab allocator が登場する。

- SLAB
  マルチコアプロセッサーには向いていない。
- SLUB
  SLAB をシンプルにしつつスケーラビリティを確保した改良版。デフォルトの slab allocator になっている。
  特にマルチコアプロセッサーに向いている。
- SLOB
  small footprint

という 3 種類の Slab Allocator が用意されている。
カーネルのビルド時にそのうちどれを選択するかを選べるようになっている。

以下では、SLUB を中心に学んでいく。

# How SLUB works

Buddy Memory Allocation によって割り当てられた 4KiB のメモリブロック(Page)を Slab Allocator によってオブジェクトに分割される。

# Slab allocator の状況確認

```sh
[ec2-user@ip-192-168-0-36 ~]$ sudo cat /proc/slabinfo
slabinfo - version: 2.1
# name            <active_objs> <num_objs> <objsize> <objperslab> <pagesperslab> : tunables <limit> <batchcount> <sharedfactor> : slabdata <active_slabs> <num_slabs> <sharedavail>
fat_inode_cache       20     20    784   20    4 : tunables    0    0    0 : slabdata      1      1      0
fat_cache              0      0     40  102    1 : tunables    0    0    0 : slabdata      0      0      0
rpc_inode_cache       23     23    704   23    4 : tunables    0    0    0 : slabdata      1      1      0
fuse_request           0      0    152   26    1 : tunables    0    0    0 : slabdata      0      0      0
fuse_inode             0      0    832   19    4 : tunables    0    0    0 : slabdata      0      0      0
kcopyd_job             0      0   3240   10    8 : tunables    0    0    0 : slabdata      0      0      0
dm_uevent              0      0   2888   11    8 : tunables    0    0    0 : slabdata      0      0      0
dax_cache             19     19    832   19    4 : tunables    0    0    0 : slabdata      1      1      0
fscrypt_info           0      0    128   32    1 : tunables    0    0    0 : slabdata      0      0      0
bio-136               84     84    192   21    1 : tunables    0    0    0 : slabdata      4      4      0
MPTCPv6                0      0   2112   15    8 : tunables    0    0    0 : slabdata      0      0      0
PINGv6                 0      0   1216   13    4 : tunables    0    0    0 : slabdata      0      0      0
RAWv6                 26     26   1216   13    4 : tunables    0    0    0 : slabdata      2      2      0
UDPv6                 24     24   1344   12    4 : tunables    0    0    0 : slabdata      2      2      0
tw_sock_TCPv6          0      0    272   15    1 : tunables    0    0    0 : slabdata      0      0      0
request_sock_TCPv6      0      0    312   13    1 : tunables    0    0    0 : slabdata      0      0      0
TCPv6                 26     26   2432   13    8 : tunables    0    0    0 : slabdata      2      2      0
bfq_queue              0      0    568   14    2 : tunables    0    0    0 : slabdata      0      0      0
mqueue_inode_cache     17     17    960   17    4 : tunables    0    0    0 : slabdata      1      1      0
zonefs_inode_cache      0      0    664   12    2 : tunables    0    0    0 : slabdata      0      0      0
xfs_dqtrx              0      0    528   15    2 : tunables    0    0    0 : slabdata      0      0      0
xfs_bui_item          19     19    208   19    1 : tunables    0    0    0 : slabdata      1      1      0
xfs_rui_item           0      0    688   23    4 : tunables    0    0    0 : slabdata      0      0      0
xfs_rud_item          46     46    176   23    1 : tunables    0    0    0 : slabdata      2      2      0
xfs_icr               44     44    184   22    1 : tunables    0    0    0 : slabdata      2      2      0
xfs_ili             3160   3160    200   20    1 : tunables    0    0    0 : slabdata    158    158      0
xfs_inode          14896  14896   1024   16    4 : tunables    0    0    0 : slabdata    931    931      0
xfs_efi_item          72     72    432   18    2 : tunables    0    0    0 : slabdata      4      4      0
xfs_efd_item          36     36    440   18    2 : tunables    0    0    0 : slabdata      2      2      0
xfs_trans             34     34    232   17    1 : tunables    0    0    0 : slabdata      2      2      0
xfs_da_state           0      0    480   17    2 : tunables    0    0    0 : slabdata      0      0      0
xfs_rmapbt_cur         0      0    248   16    1 : tunables    0    0    0 : slabdata      0      0      0
xfs_bmbt_cur          24     24    328   12    1 : tunables    0    0    0 : slabdata      2      2      0
xfs_buf             1197   1197    384   21    2 : tunables    0    0    0 : slabdata     57     57      0
kioctx                 0      0    576   14    2 : tunables    0    0    0 : slabdata      0      0      0
userfaultfd_ctx_cache      0      0    192   21    1 : tunables    0    0    0 : slabdata      0      0      0
dnotify_struct         0      0     32  128    1 : tunables    0    0    0 : slabdata      0      0      0
pid_namespace          0      0    136   30    1 : tunables    0    0    0 : slabdata      0      0      0
UNIX                 150    150   1088   15    4 : tunables    0    0    0 : slabdata     10     10      0
ip4-frags             40     40    200   20    1 : tunables    0    0    0 : slabdata      2      2      0
MPTCP                  0      0   1920   17    8 : tunables    0    0    0 : slabdata      0      0      0
request_sock_subflow_v6      0      0    384   21    2 : tunables    0    0    0 : slabdata      0      0      0
request_sock_subflow_v4      0      0    384   21    2 : tunables    0    0    0 : slabdata      0      0      0
xfrm_dst_cache         0      0    320   12    1 : tunables    0    0    0 : slabdata      0      0      0
xfrm_state             0      0    768   21    4 : tunables    0    0    0 : slabdata      0      0      0
ip_fib_trie           85     85     48   85    1 : tunables    0    0    0 : slabdata      1      1      0
ip_fib_alias          73     73     56   73    1 : tunables    0    0    0 : slabdata      1      1      0
PING                   0      0   1024   16    4 : tunables    0    0    0 : slabdata      0      0      0
RAW                   16     16   1024   16    4 : tunables    0    0    0 : slabdata      1      1      0
tw_sock_TCP           45     45    272   15    1 : tunables    0    0    0 : slabdata      3      3      0
request_sock_TCP      26     26    312   13    1 : tunables    0    0    0 : slabdata      2      2      0
TCP                   28     28   2240   14    8 : tunables    0    0    0 : slabdata      2      2      0
hugetlbfs_inode_cache     12     12    656   12    2 : tunables    0    0    0 : slabdata      1      1      0
dquot                  0      0    256   16    1 : tunables    0    0    0 : slabdata      0      0      0
bio-264              228    228    320   12    1 : tunables    0    0    0 : slabdata     19     19      0
ep_head              512    512     16  256    1 : tunables    0    0    0 : slabdata      2      2      0
request_queue_srcu      0      0   1344   12    4 : tunables    0    0    0 : slabdata      0      0      0
request_queue         34     34    952   17    4 : tunables    0    0    0 : slabdata      2      2      0
bio-200              112    112    256   16    1 : tunables    0    0    0 : slabdata      7      7      0
biovec-max            84    104   4096    8    8 : tunables    0    0    0 : slabdata     13     13      0
biovec-128            32     32   2048   16    8 : tunables    0    0    0 : slabdata      2      2      0
biovec-64             32     32   1024   16    4 : tunables    0    0    0 : slabdata      2      2      0
khugepaged_mm_slot      0      0    112   36    1 : tunables    0    0    0 : slabdata      0      0      0
user_namespace        13     13    624   13    2 : tunables    0    0    0 : slabdata      1      1      0
dmaengine-unmap-256     15     15   2112   15    8 : tunables    0    0    0 : slabdata      1      1      0
dmaengine-unmap-128     15     15   1088   15    4 : tunables    0    0    0 : slabdata      1      1      0
sock_inode_cache     266    266    832   19    4 : tunables    0    0    0 : slabdata     14     14      0
skbuff_ext_cache      42     42    192   21    1 : tunables    0    0    0 : slabdata      2      2      0
skbuff_fclone_cache     80     80    512   16    2 : tunables    0    0    0 : slabdata      5      5      0
skbuff_head_cache    112    112    256   16    1 : tunables    0    0    0 : slabdata      7      7      0
file_lock_cache       36     36    216   18    1 : tunables    0    0    0 : slabdata      2      2      0
buffer_head          117    117    104   39    1 : tunables    0    0    0 : slabdata      3      3      0
taskstats             38     38    416   19    2 : tunables    0    0    0 : slabdata      2      2      0
proc_dir_entry       483    483    192   21    1 : tunables    0    0    0 : slabdata     23     23      0
pde_opener           204    204     40  102    1 : tunables    0    0    0 : slabdata      2      2      0
proc_inode_cache    1840   1840    704   23    4 : tunables    0    0    0 : slabdata     80     80      0
seq_file              68     68    120   34    1 : tunables    0    0    0 : slabdata      2      2      0
sigqueue             153    153     80   51    1 : tunables    0    0    0 : slabdata      3      3      0
bdev_cache            21     21   1536   21    8 : tunables    0    0    0 : slabdata      1      1      0
shmem_inode_cache   1051   1092    776   21    4 : tunables    0    0    0 : slabdata     52     52      0
kernfs_iattrs_cache    874    874     88   46    1 : tunables    0    0    0 : slabdata     19     19      0
kernfs_node_cache  16224  16224    128   32    1 : tunables    0    0    0 : slabdata    507    507      0
mnt_cache            780    780    320   12    1 : tunables    0    0    0 : slabdata     65     65      0
filp                1393   1520    256   16    1 : tunables    0    0    0 : slabdata     95     95      0
inode_cache        18449  18450    632   25    4 : tunables    0    0    0 : slabdata    738    738      0
dentry             38891  38913    192   21    1 : tunables    0    0    0 : slabdata   1853   1853      0
names_cache           24     24   4096    8    8 : tunables    0    0    0 : slabdata      3      3      0
net_namespace          7      7   4288    7    8 : tunables    0    0    0 : slabdata      1      1      0
avtab_extended_perms    306    306     40  102    1 : tunables    0    0    0 : slabdata      3      3      0
avtab_node        108460 108460     24  170    1 : tunables    0    0    0 : slabdata    638    638      0
avc_xperms_data      256    256     32  128    1 : tunables    0    0    0 : slabdata      2      2      0
iint_cache             0      0    128   32    1 : tunables    0    0    0 : slabdata      0      0      0
lsm_file_cache      1386   1536     16  256    1 : tunables    0    0    0 : slabdata      6      6      0
uts_namespace         36     36    432   18    2 : tunables    0    0    0 : slabdata      2      2      0
nsproxy              112    112     72   56    1 : tunables    0    0    0 : slabdata      2      2      0
vm_area_struct      4556   4706    152   26    1 : tunables    0    0    0 : slabdata    181    181      0
files_cache          210    299    704   23    4 : tunables    0    0    0 : slabdata     13     13      0
signal_cache         155    252   1152   14    4 : tunables    0    0    0 : slabdata     18     18      0
sighand_cache        176    195   2112   15    8 : tunables    0    0    0 : slabdata     13     13      0
task_struct          139    139  12224    1    4 : tunables    0    0    0 : slabdata    139    139      0
cred_jar             483    483    192   21    1 : tunables    0    0    0 : slabdata     23     23      0
anon_vma_chain      2665   2752     64   64    1 : tunables    0    0    0 : slabdata     43     43      0
anon_vma            1599   1599    104   39    1 : tunables    0    0    0 : slabdata     41     41      0
pid                  425    480    128   32    1 : tunables    0    0    0 : slabdata     15     15      0
Acpi-Operand        1636   1736     72   56    1 : tunables    0    0    0 : slabdata     31     31      0
Acpi-State           204    204     80   51    1 : tunables    0    0    0 : slabdata      4      4      0
shared_policy_node    425    425     48   85    1 : tunables    0    0    0 : slabdata      5      5      0
numa_policy          121    180    272   15    1 : tunables    0    0    0 : slabdata     12     12      0
perf_event             0      0   1248   13    4 : tunables    0    0    0 : slabdata      0      0      0
trace_event_file    1974   1974     96   42    1 : tunables    0    0    0 : slabdata     47     47      0
ftrace_event_field   4161   4161     56   73    1 : tunables    0    0    0 : slabdata     57     57      0
maple_node          1174   1344    256   16    1 : tunables    0    0    0 : slabdata     84     84      0
radix_tree_node     5646   5656    584   14    2 : tunables    0    0    0 : slabdata    404    404      0
task_group            84     84    640   12    2 : tunables    0    0    0 : slabdata      7      7      0
mm_struct             80     80   1024   16    4 : tunables    0    0    0 : slabdata      5      5      0
vmap_area          83419  86272     64   64    1 : tunables    0    0    0 : slabdata   1348   1348      0
dma-kmalloc-8k         0      0   8192    4    8 : tunables    0    0    0 : slabdata      0      0      0
dma-kmalloc-4k         0      0   4096    8    8 : tunables    0    0    0 : slabdata      0      0      0
dma-kmalloc-2k         0      0   2048   16    8 : tunables    0    0    0 : slabdata      0      0      0
dma-kmalloc-1k         0      0   1024   16    4 : tunables    0    0    0 : slabdata      0      0      0
dma-kmalloc-512        0      0    512   16    2 : tunables    0    0    0 : slabdata      0      0      0
dma-kmalloc-256        0      0    256   16    1 : tunables    0    0    0 : slabdata      0      0      0
dma-kmalloc-192        0      0    192   21    1 : tunables    0    0    0 : slabdata      0      0      0
dma-kmalloc-128        0      0    128   32    1 : tunables    0    0    0 : slabdata      0      0      0
dma-kmalloc-96         0      0     96   42    1 : tunables    0    0    0 : slabdata      0      0      0
dma-kmalloc-64         0      0     64   64    1 : tunables    0    0    0 : slabdata      0      0      0
dma-kmalloc-32         0      0     32  128    1 : tunables    0    0    0 : slabdata      0      0      0
dma-kmalloc-16         0      0     16  256    1 : tunables    0    0    0 : slabdata      0      0      0
dma-kmalloc-8          0      0      8  512    1 : tunables    0    0    0 : slabdata      0      0      0
kmalloc-rcl-8k         0      0   8192    4    8 : tunables    0    0    0 : slabdata      0      0      0
kmalloc-rcl-4k         0      0   4096    8    8 : tunables    0    0    0 : slabdata      0      0      0
kmalloc-rcl-2k         0      0   2048   16    8 : tunables    0    0    0 : slabdata      0      0      0
kmalloc-rcl-1k         0      0   1024   16    4 : tunables    0    0    0 : slabdata      0      0      0
kmalloc-rcl-512        0      0    512   16    2 : tunables    0    0    0 : slabdata      0      0      0
kmalloc-rcl-256        0      0    256   16    1 : tunables    0    0    0 : slabdata      0      0      0
kmalloc-rcl-192       42     42    192   21    1 : tunables    0    0    0 : slabdata      2      2      0
kmalloc-rcl-128      128    128    128   32    1 : tunables    0    0    0 : slabdata      4      4      0
kmalloc-rcl-96       126    126     96   42    1 : tunables    0    0    0 : slabdata      3      3      0
kmalloc-rcl-64       704    704     64   64    1 : tunables    0    0    0 : slabdata     11     11      0
kmalloc-rcl-32         0      0     32  128    1 : tunables    0    0    0 : slabdata      0      0      0
kmalloc-rcl-16         0      0     16  256    1 : tunables    0    0    0 : slabdata      0      0      0
kmalloc-rcl-8          0      0      8  512    1 : tunables    0    0    0 : slabdata      0      0      0
kmalloc-cg-8k          0      0   8192    4    8 : tunables    0    0    0 : slabdata      0      0      0
kmalloc-cg-4k         40     40   4096    8    8 : tunables    0    0    0 : slabdata      5      5      0
kmalloc-cg-2k        334    352   2048   16    8 : tunables    0    0    0 : slabdata     22     22      0
kmalloc-cg-1k         73     96   1024   16    4 : tunables    0    0    0 : slabdata      6      6      0
kmalloc-cg-512        80     80    512   16    2 : tunables    0    0    0 : slabdata      5      5      0
kmalloc-cg-256        32     32    256   16    1 : tunables    0    0    0 : slabdata      2      2      0
kmalloc-cg-192        84     84    192   21    1 : tunables    0    0    0 : slabdata      4      4      0
kmalloc-cg-128        64     64    128   32    1 : tunables    0    0    0 : slabdata      2      2      0
kmalloc-cg-96         84     84     96   42    1 : tunables    0    0    0 : slabdata      2      2      0
kmalloc-cg-64        128    128     64   64    1 : tunables    0    0    0 : slabdata      2      2      0
kmalloc-cg-32        865   1280     32  128    1 : tunables    0    0    0 : slabdata     10     10      0
kmalloc-cg-16        512    512     16  256    1 : tunables    0    0    0 : slabdata      2      2      0
kmalloc-cg-8        1024   1024      8  512    1 : tunables    0    0    0 : slabdata      2      2      0
kmalloc-8k            36     36   8192    4    8 : tunables    0    0    0 : slabdata      9      9      0
kmalloc-4k           222    232   4096    8    8 : tunables    0    0    0 : slabdata     29     29      0
kmalloc-2k           583    592   2048   16    8 : tunables    0    0    0 : slabdata     37     37      0
kmalloc-1k          3493   4832   1024   16    4 : tunables    0    0    0 : slabdata    302    302      0
kmalloc-512         1315   2224    512   16    2 : tunables    0    0    0 : slabdata    139    139      0
kmalloc-256         2336   2336    256   16    1 : tunables    0    0    0 : slabdata    146    146      0
kmalloc-192         2037   2037    192   21    1 : tunables    0    0    0 : slabdata     97     97      0
kmalloc-128         2152   2208    128   32    1 : tunables    0    0    0 : slabdata     69     69      0
kmalloc-96          2225   2520     96   42    1 : tunables    0    0    0 : slabdata     60     60      0
kmalloc-64         28156  28160     64   64    1 : tunables    0    0    0 : slabdata    440    440      0
kmalloc-32         19708  19712     32  128    1 : tunables    0    0    0 : slabdata    154    154      0
kmalloc-16         36028  36096     16  256    1 : tunables    0    0    0 : slabdata    141    141      0
kmalloc-8          10108  10240      8  512    1 : tunables    0    0    0 : slabdata     20     20      0
kmem_cache_node      256    256     64   64    1 : tunables    0    0    0 : slabdata      4      4      0
kmem_cache           192    192    256   16    1 : tunables    0    0    0 : slabdata     12     12      0
[ec2-user@ip-192-168-0-36 ~]$
```

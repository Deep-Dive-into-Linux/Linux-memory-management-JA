malloc を利用して virtual memory、physical memory の関係性を見てみよう。

# 検証

## 検証準備

- allocate.c
  メモリを確保し、更新するプログラム。
  以下実行し、allocate という名前の ELF を作成し、実行する。
  gcc allocate.c -o allocate

- capture.sh
  ps コマンドでプロセス単体の vsz, rsz などの推移を確認する。

- 検証実行環境は以下

```sh
[ec2-user@ip-192-168-0-178 work]$ uname -a
Linux ip-192-168-0-178.ec2.internal 5.14.0-427.20.1.el9_4.x86_64 #1 SMP PREEMPT_DYNAMIC Thu May 23 16:37:13 EDT 2024 x86_64 x86_64 x86_64 GNU/Linux
[ec2-user@ip-192-168-0-178 work]$ cat /etc/os-release
NAME="Red Hat Enterprise Linux"
VERSION="9.4 (Plow)"
ID="rhel"
ID_LIKE="fedora"
VERSION_ID="9.4"
PLATFORM_ID="platform:el9"
PRETTY_NAME="Red Hat Enterprise Linux 9.4 (Plow)"
ANSI_COLOR="0;31"
LOGO="fedora-logo-icon"
CPE_NAME="cpe:/o:redhat:enterprise_linux:9::baseos"
HOME_URL="https://www.redhat.com/"
DOCUMENTATION_URL="https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/9"
BUG_REPORT_URL="https://bugzilla.redhat.com/"
REDHAT_BUGZILLA_PRODUCT="Red Hat Enterprise Linux 9"
REDHAT_BUGZILLA_PRODUCT_VERSION=9.4
REDHAT_SUPPORT_PRODUCT="Red Hat Enterprise Linux"
REDHAT_SUPPORT_PRODUCT_VERSION="9.4"
[ec2-user@ip-192-168-0-178 work]$
```

## 検証実行

main を実行しているプロセス ID (以下では 5034) を確認し、その memory maps を調べる。

```sh
[ec2-user@ip-192-168-0-178 work]$ ps aux | grep allocate
ec2-user    5034  0.0  0.0   2632  1280 pts/0    S+   03:18   0:00 ./allocate
ec2-user    5040  0.0  0.0   6408  2176 pts/1    S+   03:18   0:00 grep --color=auto allocate
```

上記より、stack は 7ffc1713c000-7ffc1715d000 の範囲で virtual address が割り当てられる。
heap は 02283000-022a4000 の範囲で割り当てられる。

```sh
[ec2-user@ip-192-168-0-178 work]$ cat /proc/5034/maps
00400000-00401000 r--p 00000000 103:04 870381                            /home/ec2-user/work/allocate
00401000-00402000 r-xp 00001000 103:04 870381                            /home/ec2-user/work/allocate
00402000-00403000 r--p 00002000 103:04 870381                            /home/ec2-user/work/allocate
00403000-00404000 r--p 00002000 103:04 870381                            /home/ec2-user/work/allocate
00404000-00405000 rw-p 00003000 103:04 870381                            /home/ec2-user/work/allocate
02283000-022a4000 rw-p 00000000 00:00 0                                  [heap]
7fdaf4600000-7fdaf4628000 r--p 00000000 103:04 25175605                  /usr/lib64/libc.so.6
7fdaf4628000-7fdaf479d000 r-xp 00028000 103:04 25175605                  /usr/lib64/libc.so.6
7fdaf479d000-7fdaf47f5000 r--p 0019d000 103:04 25175605                  /usr/lib64/libc.so.6
7fdaf47f5000-7fdaf47f9000 r--p 001f5000 103:04 25175605                  /usr/lib64/libc.so.6
7fdaf47f9000-7fdaf47fb000 rw-p 001f9000 103:04 25175605                  /usr/lib64/libc.so.6
7fdaf47fb000-7fdaf4808000 rw-p 00000000 00:00 0
7fdaf487f000-7fdaf4882000 rw-p 00000000 00:00 0
7fdaf488a000-7fdaf488c000 rw-p 00000000 00:00 0
7fdaf488c000-7fdaf488e000 r--p 00000000 103:04 25175601                  /usr/lib64/ld-linux-x86-64.so.2
7fdaf488e000-7fdaf48b5000 r-xp 00002000 103:04 25175601                  /usr/lib64/ld-linux-x86-64.so.2
7fdaf48b5000-7fdaf48c0000 r--p 00029000 103:04 25175601                  /usr/lib64/ld-linux-x86-64.so.2
7fdaf48c0000-7fdaf48c2000 r--p 00034000 103:04 25175601                  /usr/lib64/ld-linux-x86-64.so.2
7fdaf48c2000-7fdaf48c4000 rw-p 00036000 103:04 25175601                  /usr/lib64/ld-linux-x86-64.so.2
7ffc1713c000-7ffc1715d000 rw-p 00000000 00:00 0                          [stack]
7ffc171fa000-7ffc171fe000 r--p 00000000 00:00 0                          [vvar]
7ffc171fe000-7ffc17200000 r-xp 00000000 00:00 0                          [vdso]
ffffffffff600000-ffffffffff601000 --xp 00000000 00:00 0                  [vsyscall]
[ec2-user@ip-192-168-0-178 work]$ free -h
               total        used        free      shared  buff/cache   available
Mem:            30Gi       675Mi        29Gi       8.0Mi       539Mi        29Gi
Swap:             0B          0B          0B
[ec2-user@ip-192-168-0-178 work]$ sar -r 1
Linux 5.14.0-427.20.1.el9_4.x86_64 (ip-192-168-0-178.ec2.internal) 	12/09/2024 	_x86_64_	(16 CPU)
03:19:51 AM kbmemfree   kbavail kbmemused  %memused kbbuffers  kbcached  kbcommit   %commit  kbactive   kbinact   kbdirty
03:19:52 AM  31040884  31183588    216152      0.68      2916    466600    279148      0.88    240776    302848         0
03:19:53 AM  31037396  31180124    219012      0.69      2916    466632    276652      0.87    237948    302872         4
03:19:54 AM  31037144  31179872    219232      0.69      2916    466632    276652      0.87    238008    302872         4
03:19:55 AM  31036892  31179620    219484      0.69      2916    466632    276652      0.87    238092    302872         4
03:19:56 AM  31036640  31179368    219732      0.69      2916    466632    276652      0.87    238164    302872         4
^C
Average:     31037791  31180514    218722      0.69      2916    466626    277151      0.87    238598    302867         3
```

malloc 直後には、heap が拡張されている。
heap は 7fdaee1ff000-7fdaf4600000 が割り当てられている。
char 型 つまり 1 byte (仕様として決められたものではないが、慣例的に多くのシステムで 1 byte) の 100 x 1024 x 1024 bytes を確保、つまり 100 MiB 確保できてるはず。
0x7fdaf4600000 - 0x7fdaee1ff000 = 104,861,696
なので、100 MiB は確保できている。

領域は割り当てられているが、物理メモリの使用量的には増えていない。

```sh
[ec2-user@ip-192-168-0-178 work]$ cat /proc/5034/maps
00400000-00401000 r--p 00000000 103:04 870381                            /home/ec2-user/work/allocate
00401000-00402000 r-xp 00001000 103:04 870381                            /home/ec2-user/work/allocate
00402000-00403000 r--p 00002000 103:04 870381                            /home/ec2-user/work/allocate
00403000-00404000 r--p 00002000 103:04 870381                            /home/ec2-user/work/allocate
00404000-00405000 rw-p 00003000 103:04 870381                            /home/ec2-user/work/allocate
02283000-022a4000 rw-p 00000000 00:00 0                                  [heap]
7fdaee1ff000-7fdaf4600000 rw-p 00000000 00:00 0
7fdaf4600000-7fdaf4628000 r--p 00000000 103:04 25175605                  /usr/lib64/libc.so.6
7fdaf4628000-7fdaf479d000 r-xp 00028000 103:04 25175605                  /usr/lib64/libc.so.6
7fdaf479d000-7fdaf47f5000 r--p 0019d000 103:04 25175605                  /usr/lib64/libc.so.6
7fdaf47f5000-7fdaf47f9000 r--p 001f5000 103:04 25175605                  /usr/lib64/libc.so.6
7fdaf47f9000-7fdaf47fb000 rw-p 001f9000 103:04 25175605                  /usr/lib64/libc.so.6
7fdaf47fb000-7fdaf4808000 rw-p 00000000 00:00 0
7fdaf487f000-7fdaf4882000 rw-p 00000000 00:00 0
7fdaf488a000-7fdaf488c000 rw-p 00000000 00:00 0
7fdaf488c000-7fdaf488e000 r--p 00000000 103:04 25175601                  /usr/lib64/ld-linux-x86-64.so.2
7fdaf488e000-7fdaf48b5000 r-xp 00002000 103:04 25175601                  /usr/lib64/ld-linux-x86-64.so.2
7fdaf48b5000-7fdaf48c0000 r--p 00029000 103:04 25175601                  /usr/lib64/ld-linux-x86-64.so.2
7fdaf48c0000-7fdaf48c2000 r--p 00034000 103:04 25175601                  /usr/lib64/ld-linux-x86-64.so.2
7fdaf48c2000-7fdaf48c4000 rw-p 00036000 103:04 25175601                  /usr/lib64/ld-linux-x86-64.so.2
7ffc1713c000-7ffc1715d000 rw-p 00000000 00:00 0                          [stack]
7ffc171fa000-7ffc171fe000 r--p 00000000 00:00 0                          [vvar]
7ffc171fe000-7ffc17200000 r-xp 00000000 00:00 0                          [vdso]
ffffffffff600000-ffffffffff601000 --xp 00000000 00:00 0                  [vsyscall]
[ec2-user@ip-192-168-0-178 work]$ free -h
               total        used        free      shared  buff/cache   available
Mem:            30Gi       677Mi        29Gi       8.0Mi       539Mi        29Gi
Swap:             0B          0B          0B
[ec2-user@ip-192-168-0-178 work]$ sar -r 1
Linux 5.14.0-427.20.1.el9_4.x86_64 (ip-192-168-0-178.ec2.internal) 	12/09/2024 	_x86_64_	(16 CPU)
03:20:11 AM kbmemfree   kbavail kbmemused  %memused kbbuffers  kbcached  kbcommit   %commit  kbactive   kbinact   kbdirty
03:20:12 AM  31041300  31184324    214848      0.67      2916    466928    378672      1.19    237852    302912        12
03:20:13 AM  31040296  31183320    215844      0.68      2916    466928    378672      1.19    238240    302912        12
03:20:14 AM  31040296  31183320    215860      0.68      2916    466928    378672      1.19    238512    302912        12
03:20:15 AM  31040904  31183928    215252      0.68      2916    466928    378672      1.19    237880    302912        12
03:20:16 AM  31039920  31182944    216228      0.68      2916    466928    378672      1.19    237824    302912        12
^C
Average:     31040543  31183567    215606      0.68      2916    466928    378672      1.19    238062    302912        12
```

heap 領域へ書き込みが行われると、物理メモリ使用量も増加していることがわかる。

```sh
[ec2-user@ip-192-168-0-178 work]$ sar -r 1
Linux 5.14.0-427.20.1.el9_4.x86_64 (ip-192-168-0-178.ec2.internal) 	12/09/2024 	_x86_64_	(16 CPU)
03:20:21 AM kbmemfree   kbavail kbmemused  %memused kbbuffers  kbcached  kbcommit   %commit  kbactive   kbinact   kbdirty
03:20:22 AM  31042328  31185352    213852      0.67      2916    466928    378672      1.19    237496    302912        12
03:20:23 AM  31042328  31185352    213852      0.67      2916    466928    378672      1.19    237468    302912        12
03:20:24 AM  31042328  31185352    213852      0.67      2916    466928    378672      1.19    237800    302912        12
03:20:25 AM  31029536  31172560    226644      0.71      2916    466928    378672      1.19    248256    302912        12
03:20:26 AM  31016996  31160020    239200      0.75      2916    466928    378672      1.19    258552    302912        12
03:20:27 AM  31004708  31147732    251488      0.79      2916    466928    378672      1.19    268768    302912        12
03:20:28 AM  30996516  31139540    259680      0.81      2916    466928    378672      1.19    279484    302912        12
03:20:29 AM  30984228  31127252    271984      0.85      2916    466928    378672      1.19    290096    302912        12
03:20:30 AM  30976036  31119060    280176      0.88      2916    466928    378672      1.19    300428    302912        12
03:20:31 AM  30963244  31106268    292968      0.92      2916    466928    378672      1.19    310788    302912        12
03:20:32 AM  30960264  31103288    295956      0.93      2916    466928    378672      1.19    321036    302912        12
03:20:33 AM  30946980  31090004    309232      0.97      2916    466928    378672      1.19    331504    302912        12
03:20:34 AM  31036588  31179612    219624      0.69      2916    466928    275884      0.87    238252    302912        12
^C
Average:     31003237  31146261    252962      0.79      2916    466928    370765      1.16    273841    302912        12
[ec2-user@ip-192-168-0-178 work]$
```

### プログラム実行ログ

```sh
[ec2-user@ip-192-168-0-178 work]$ ./allocate
-----------------------stack-----------------------
stack: 0x7ffc1715b170: 0
stack: 0x7ffc1715b174: 1
stack: 0x7ffc1715b178: 2
stack: 0x7ffc1715b17c: 3
stack: 0x7ffc1715b180: 4
stack: 0x7ffc1715b184: 5
stack: 0x7ffc1715b188: 6
stack: 0x7ffc1715b18c: 7
stack: 0x7ffc1715b190: 8
stack: 0x7ffc1715b194: 9
-----------------------stack-----------------------
-----------------------heap-----------------------
Will allocate memory
Succeeded to allocate memory: address = 0x7fdaee1ff010; size = 0x6400000 ***
Before write values to physical memory
Mon Dec  9 03:20:24 2024: touched 10MB
Mon Dec  9 03:20:25 2024: touched 20MB
Mon Dec  9 03:20:26 2024: touched 30MB
Mon Dec  9 03:20:27 2024: touched 40MB
Mon Dec  9 03:20:28 2024: touched 50MB
Mon Dec  9 03:20:29 2024: touched 60MB
Mon Dec  9 03:20:30 2024: touched 70MB
Mon Dec  9 03:20:31 2024: touched 80MB
Mon Dec  9 03:20:32 2024: touched 90MB
[ec2-user@ip-192-168-0-178 work]$
```

### capture.sh (ps コマンドを使用)

プロセス単体で見ると以下のようになる。

- `Mon Dec  9 03:20:01 AM UTC 2024:    5034 allocate        105036  1280      0     83`
  vsz つまり virtual メモリの割合が 2632 から 105036 に一気に増えている。しかし、rss physical memory の値は変化なし。
- `Mon Dec  9 03:20:25 AM UTC 2024:    5034 allocate        105036 11880      0     93`
  physical memory の値が一気に 1280 から 11880 に増えた。ここから physical memory 使用量は上昇し続ける。
  また、min_flt つまり minor fault が発生している。page fault のうち、page in によってストレージデバイスへのアクセスが発生したものを major fault、そうでないものを minor fault と呼ぶ。どちらもカーネル内処理が走るため影響はあるが、major fault はストレージデバイスへのアクセスが発生するため、負荷は大きい。

```sh
[ec2-user@ip-192-168-0-178 work]$ . capture.sh
Mon Dec  9 03:19:44 AM UTC 2024:    5034 allocate          2632  1280      0     82
Mon Dec  9 03:19:45 AM UTC 2024:    5034 allocate          2632  1280      0     82
Mon Dec  9 03:19:46 AM UTC 2024:    5034 allocate          2632  1280      0     82
Mon Dec  9 03:19:47 AM UTC 2024:    5034 allocate          2632  1280      0     82
Mon Dec  9 03:19:48 AM UTC 2024:    5034 allocate          2632  1280      0     82
Mon Dec  9 03:19:49 AM UTC 2024:    5034 allocate          2632  1280      0     82
Mon Dec  9 03:19:50 AM UTC 2024:    5034 allocate          2632  1280      0     82
Mon Dec  9 03:19:51 AM UTC 2024:    5034 allocate          2632  1280      0     82
Mon Dec  9 03:19:52 AM UTC 2024:    5034 allocate          2632  1280      0     82
Mon Dec  9 03:19:53 AM UTC 2024:    5034 allocate          2632  1280      0     82
Mon Dec  9 03:19:54 AM UTC 2024:    5034 allocate          2632  1280      0     82
Mon Dec  9 03:19:55 AM UTC 2024:    5034 allocate          2632  1280      0     82
Mon Dec  9 03:19:56 AM UTC 2024:    5034 allocate          2632  1280      0     82
Mon Dec  9 03:19:57 AM UTC 2024:    5034 allocate          2632  1280      0     82
Mon Dec  9 03:19:58 AM UTC 2024:    5034 allocate          2632  1280      0     82
Mon Dec  9 03:19:59 AM UTC 2024:    5034 allocate          2632  1280      0     82
Mon Dec  9 03:20:00 AM UTC 2024:    5034 allocate          2632  1280      0     82
Mon Dec  9 03:20:01 AM UTC 2024:    5034 allocate        105036  1280      0     83
Mon Dec  9 03:20:02 AM UTC 2024:    5034 allocate        105036  1280      0     83
Mon Dec  9 03:20:03 AM UTC 2024:    5034 allocate        105036  1280      0     83
Mon Dec  9 03:20:04 AM UTC 2024:    5034 allocate        105036  1280      0     83
Mon Dec  9 03:20:05 AM UTC 2024:    5034 allocate        105036  1280      0     83
Mon Dec  9 03:20:06 AM UTC 2024:    5034 allocate        105036  1280      0     83
Mon Dec  9 03:20:07 AM UTC 2024:    5034 allocate        105036  1280      0     83
Mon Dec  9 03:20:08 AM UTC 2024:    5034 allocate        105036  1280      0     83
Mon Dec  9 03:20:09 AM UTC 2024:    5034 allocate        105036  1280      0     83
Mon Dec  9 03:20:10 AM UTC 2024:    5034 allocate        105036  1280      0     83
Mon Dec  9 03:20:11 AM UTC 2024:    5034 allocate        105036  1280      0     83
Mon Dec  9 03:20:13 AM UTC 2024:    5034 allocate        105036  1280      0     83
Mon Dec  9 03:20:14 AM UTC 2024:    5034 allocate        105036  1280      0     83
Mon Dec  9 03:20:15 AM UTC 2024:    5034 allocate        105036  1280      0     83
Mon Dec  9 03:20:16 AM UTC 2024:    5034 allocate        105036  1280      0     83
Mon Dec  9 03:20:17 AM UTC 2024:    5034 allocate        105036  1280      0     83
Mon Dec  9 03:20:18 AM UTC 2024:    5034 allocate        105036  1280      0     83
Mon Dec  9 03:20:19 AM UTC 2024:    5034 allocate        105036  1280      0     83
Mon Dec  9 03:20:20 AM UTC 2024:    5034 allocate        105036  1280      0     83
Mon Dec  9 03:20:21 AM UTC 2024:    5034 allocate        105036  1280      0     83
Mon Dec  9 03:20:22 AM UTC 2024:    5034 allocate        105036  1280      0     83
Mon Dec  9 03:20:23 AM UTC 2024:    5034 allocate        105036  1280      0     83
Mon Dec  9 03:20:24 AM UTC 2024:    5034 allocate        105036  1280      0     83
Mon Dec  9 03:20:25 AM UTC 2024:    5034 allocate        105036 11880      0     93
Mon Dec  9 03:20:26 AM UTC 2024:    5034 allocate        105036 22124      0     98
Mon Dec  9 03:20:27 AM UTC 2024:    5034 allocate        105036 32364      0    103
Mon Dec  9 03:20:28 AM UTC 2024:    5034 allocate        105036 42604      0    108
Mon Dec  9 03:20:29 AM UTC 2024:    5034 allocate        105036 52844      0    113
Mon Dec  9 03:20:30 AM UTC 2024:    5034 allocate        105036 63084      0    118
Mon Dec  9 03:20:31 AM UTC 2024:    5034 allocate        105036 73324      0    123
Mon Dec  9 03:20:32 AM UTC 2024:    5034 allocate        105036 83564      0    128
Mon Dec  9 03:20:33 AM UTC 2024:    5034 allocate        105036 93804      0    133
Mon Dec  9 03:20:34 AM UTC 2024: target process seems to be finished
[ec2-user@ip-192-168-0-178 work]$
```

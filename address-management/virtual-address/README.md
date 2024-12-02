- 全カーネルパラメータ一覧表示
  sysctl -a
- virtual memory 関連のカーネルパラメータ
  /proc/sys/vm ディレクトリ配下に表示される。

## プロセスのメモリマップを確認する

以下ファイルを作成。
main という名前で ELF を作成し、実行する。
gcc main.c -o main

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))
#endif

int main(int argc, char const *argv[])
{
    int32_t arrray[10]= {0,1,2,3,4,5,6,7,8,9};
    int32_t *heap = (int32_t *)calloc(10, sizeof(int32_t));

    while (1)
    {
        for (size_t i = 0; i < ARRAY_LENGTH(arrray); i++)
        {
            heap[i] = i;
            printf("stack: %p: %d\n", &arrray[i], arrray[i]);
            printf("heap:  %p: %d\n", &heap[i], heap[i]);
            sleep(1);
        }
    }
    return 0;
}
```

main を実行しているプロセス ID (以下では 27652) を確認し、その memory maps を調べる。

```sh
[ec2-user@ip-192-168-32-147 ~]$ ps aux | grep main
ec2-user   27652  0.0  0.0   2632   944 pts/0    S+   19:44   0:00 ./main
ec2-user   27654  0.0  0.0 222316  2192 pts/1    S+   19:44   0:00 grep --color=auto main
[ec2-user@ip-192-168-32-147 ~]$ cat /proc/27652/maps
00400000-00401000 r--p 00000000 103:01 8764467                           /home/ec2-user/work/main
00401000-00402000 r-xp 00001000 103:01 8764467                           /home/ec2-user/work/main
00402000-00403000 r--p 00002000 103:01 8764467                           /home/ec2-user/work/main
00403000-00404000 r--p 00002000 103:01 8764467                           /home/ec2-user/work/main
00404000-00405000 rw-p 00003000 103:01 8764467                           /home/ec2-user/work/main
08fd8000-08ff9000 rw-p 00000000 00:00 0                                  [heap]
7f80ed000000-7f80ed028000 r--p 00000000 103:01 8525642                   /usr/lib64/libc.so.6
7f80ed028000-7f80ed19d000 r-xp 00028000 103:01 8525642                   /usr/lib64/libc.so.6
7f80ed19d000-7f80ed1f5000 r--p 0019d000 103:01 8525642                   /usr/lib64/libc.so.6
7f80ed1f5000-7f80ed1f9000 r--p 001f5000 103:01 8525642                   /usr/lib64/libc.so.6
7f80ed1f9000-7f80ed1fb000 rw-p 001f9000 103:01 8525642                   /usr/lib64/libc.so.6
7f80ed1fb000-7f80ed208000 rw-p 00000000 00:00 0
7f80ed318000-7f80ed31b000 rw-p 00000000 00:00 0
7f80ed31f000-7f80ed321000 rw-p 00000000 00:00 0
7f80ed321000-7f80ed323000 r--p 00000000 103:01 8525638                   /usr/lib64/ld-linux-x86-64.so.2
7f80ed323000-7f80ed34a000 r-xp 00002000 103:01 8525638                   /usr/lib64/ld-linux-x86-64.so.2
7f80ed34a000-7f80ed355000 r--p 00029000 103:01 8525638                   /usr/lib64/ld-linux-x86-64.so.2
7f80ed355000-7f80ed357000 r--p 00034000 103:01 8525638                   /usr/lib64/ld-linux-x86-64.so.2
7f80ed357000-7f80ed359000 rw-p 00036000 103:01 8525638                   /usr/lib64/ld-linux-x86-64.so.2
7ffcfbfcb000-7ffcfbfec000 rw-p 00000000 00:00 0                          [stack]
7ffcfbff4000-7ffcfbff8000 r--p 00000000 00:00 0                          [vvar]
7ffcfbff8000-7ffcfbffa000 r-xp 00000000 00:00 0                          [vdso]
ffffffffff600000-ffffffffff601000 --xp 00000000 00:00 0                  [vsyscall]
[ec2-user@ip-192-168-32-147 ~]$
```

上記より、stack は 7ffcfbfcb000-7ffcfbfec000 の範囲で virtual address が割り当てられる。
heap は 08fd8000-08ff9000 の範囲で割り当てられる。

```sh
[ec2-user@ip-192-168-32-147 ~]$ cat /proc/self/maps
55d0aa11e000-55d0aa120000 r--p 00000000 103:01 486896                    /usr/bin/cat
55d0aa120000-55d0aa124000 r-xp 00002000 103:01 486896                    /usr/bin/cat
55d0aa124000-55d0aa126000 r--p 00006000 103:01 486896                    /usr/bin/cat
55d0aa126000-55d0aa127000 r--p 00007000 103:01 486896                    /usr/bin/cat
55d0aa127000-55d0aa128000 rw-p 00008000 103:01 486896                    /usr/bin/cat
55d0e2312000-55d0e2333000 rw-p 00000000 00:00 0                          [heap]
7f3052e00000-7f3060330000 r--p 00000000 103:01 2495                      /usr/lib/locale/locale-archive
7f3060400000-7f3060428000 r--p 00000000 103:01 8525642                   /usr/lib64/libc.so.6
7f3060428000-7f306059d000 r-xp 00028000 103:01 8525642                   /usr/lib64/libc.so.6
7f306059d000-7f30605f5000 r--p 0019d000 103:01 8525642                   /usr/lib64/libc.so.6
7f30605f5000-7f30605f9000 r--p 001f5000 103:01 8525642                   /usr/lib64/libc.so.6
7f30605f9000-7f30605fb000 rw-p 001f9000 103:01 8525642                   /usr/lib64/libc.so.6
7f30605fb000-7f3060608000 rw-p 00000000 00:00 0
7f306073b000-7f306075d000 rw-p 00000000 00:00 0
7f306075d000-7f30607b2000 r--p 00000000 103:01 8525625                   /usr/lib/locale/C.utf8/LC_CTYPE
7f30607b2000-7f30607b3000 r--p 00000000 103:01 8525630                   /usr/lib/locale/C.utf8/LC_NUMERIC
7f30607b3000-7f30607b4000 r--p 00000000 103:01 8525633                   /usr/lib/locale/C.utf8/LC_TIME
7f30607b4000-7f30607b5000 r--p 00000000 103:01 8525624                   /usr/lib/locale/C.utf8/LC_COLLATE
7f30607b5000-7f30607b6000 r--p 00000000 103:01 8525628                   /usr/lib/locale/C.utf8/LC_MONETARY
7f30607b6000-7f30607b7000 r--p 00000000 103:01 2791                      /usr/lib/locale/C.utf8/LC_MESSAGES/SYS_LC_MESSAGES
7f30607b7000-7f30607b8000 r--p 00000000 103:01 8525631                   /usr/lib/locale/C.utf8/LC_PAPER
7f30607b8000-7f30607b9000 r--p 00000000 103:01 8525629                   /usr/lib/locale/C.utf8/LC_NAME
7f30607b9000-7f30607c0000 r--s 00000000 103:01 8525698                   /usr/lib64/gconv/gconv-modules.cache
7f30607c0000-7f30607c3000 rw-p 00000000 00:00 0
7f30607c3000-7f30607c4000 r--p 00000000 103:01 8525623                   /usr/lib/locale/C.utf8/LC_ADDRESS
7f30607c4000-7f30607c5000 r--p 00000000 103:01 8525632                   /usr/lib/locale/C.utf8/LC_TELEPHONE
7f30607c5000-7f30607c6000 r--p 00000000 103:01 8525627                   /usr/lib/locale/C.utf8/LC_MEASUREMENT
7f30607c6000-7f30607c7000 r--p 00000000 103:01 8525626                   /usr/lib/locale/C.utf8/LC_IDENTIFICATION
7f30607c7000-7f30607c9000 rw-p 00000000 00:00 0
7f30607c9000-7f30607cb000 r--p 00000000 103:01 8525638                   /usr/lib64/ld-linux-x86-64.so.2
7f30607cb000-7f30607f2000 r-xp 00002000 103:01 8525638                   /usr/lib64/ld-linux-x86-64.so.2
7f30607f2000-7f30607fd000 r--p 00029000 103:01 8525638                   /usr/lib64/ld-linux-x86-64.so.2
7f30607fd000-7f30607ff000 r--p 00034000 103:01 8525638                   /usr/lib64/ld-linux-x86-64.so.2
7f30607ff000-7f3060801000 rw-p 00036000 103:01 8525638                   /usr/lib64/ld-linux-x86-64.so.2
7ffedf0f6000-7ffedf117000 rw-p 00000000 00:00 0                          [stack]
7ffedf171000-7ffedf175000 r--p 00000000 00:00 0                          [vvar]
7ffedf175000-7ffedf177000 r-xp 00000000 00:00 0                          [vdso]
ffffffffff600000-ffffffffff601000 --xp 00000000 00:00 0                  [vsyscall]
[ec2-user@ip-192-168-32-147 ~]$
```

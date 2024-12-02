64 bits の x86_64 CPU では、linear address, logical address, physical address の 3 種類がある。

参考: https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.pdf?page=89

1. Logical address (far pointer)
   ↓
   Segmentation
   ↓
2. Linear address (virtual address)
   ↓
   Paging
   ↓
3. Physical address

Linux カーネルを含む 64 bits プログラムでは segmentation は実施的に使われないため、logical address については特別記述しない。

# Linear address

Linear address の範囲は、ページングで利用するページングテーブルの段数によって異なる。
ページサイズが 4 KiB の場合ページテーブルの段数は 4 段、Ice Lake (microprocessor) 以降だと 5 段に設定することが可能。

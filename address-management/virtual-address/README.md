- 全カーネルパラメータ一覧表示
  sysctl -a
- virtual memory 関連のカーネルパラメータ
  /proc/sys/vm ディレクトリ配下に表示される。

カーネル上のソースコードを追う。

- struct task_struct - include/linux/sched.h
  https://github.com/torvalds/linux/blob/v6.12/include/linux/sched.h#L778-L1610

- struct mm_struct - include/linux/mm_types.h
  https://github.com/torvalds/linux/blob/v6.12/include/linux/mm_types.h#L790-L1040

- struct vm_area_struct - include/linux/mm_types.h
  https://github.com/torvalds/linux/blob/v6.12/include/linux/mm_types.h#L667-L773


前言

由于是自学, 无法使用 online-judge, 因此可能有 bug 没测出来


---

M1

实现一个 CLI 二维迷宫游戏

检查是否连通的时候用到了 dfs 算法, 经过的位置标记一下, 最后检查有无没标记过的即可

测试需要指定 `bash` 环境

```shell
TK_RUN=1 bash -c ./labyrinth
```

---

M2

从头实现一个 pstree (打印进程树工具)

原理: 扫描 `proc/` 文件夹

特点: 子进程只有一个父进程, 从 pid=1 开始

note: `print_tree` 的实现好像在 cs106x 见过, 需要记录 `prefix` , 递归打印
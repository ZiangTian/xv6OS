# Notes

> I have finished this lab in October, 2023 but I did not keep a copy of the code on github. Therefore this is a revisit and all the code is hosted and showcased in this repo.

## Locking scheme used here

For a bcache divided into `N` buckets, it is not sufficient to assign a bucket lock to each group. When one group runs out of LRU block, it needs to steal from other groups. Here I adopted a scheme where the group releases its own bucket first, acquires a global lock `steallock`, and then tries to steal the lock of other groups while hold that global `steallock`.   


## Test passed

![](https://raw.githubusercontent.com/ZiangTian/img-bed/main/20241003224504.png)

## Debugger steps

- one terminal

```
$ make qemu-gdb
```

- another terminal

```
$ gdb-multiarch
(gdb) target remote localhost:<port>
(gdb) set architecture riscv:rv64
(gdb) file kernel/kernel
```
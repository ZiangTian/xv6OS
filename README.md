# Notes

> I have finished this lab in October, 2023 but I did not keep a copy of the code on github. Therefore this is a revisit and all the code is hosted and showcased in this repo.

An important matter to consider in this lab is the passing of information between the kernel space and the user space. The kernel space has a lot of interfaces (`vm.c`, `proc.c`) and they have different accesses to the information sent from the user space. XV6 is designed and organized in such a way that `sysproc.c` handles the arguments passed by the user. It is also responsible for ***passing the pagetable of the current user process*** to other kernel modules so that they can navigate user addresses.

## Test passed

![](https://raw.githubusercontent.com/ZiangTian/img-bed/main/20241001133520.png)
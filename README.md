# Notes

> I have finished this lab in October, 2023 but I did not keep a copy of the code on github. Therefore this is a revisit and all the code is hosted and showcased in this repo.

## Xargs

Xargs is tricky to first understand. A common format in which it's used is the following:

```
COMMAND_1 [command list 1] | xargs COMMAND_2 [command list 2]
```

Suppose the `COMMAND_1` outputs several lines to the stdin, the pipe operator feeds the stdin as the input of xargs, making the command in effect equivalent to:

```
COMMAND_2 [command list 2] line_1
COMMAND_2 [command list 2] line_2
...
COMMAND_2 [command list 2] line_n
```

## Test passed

![](https://raw.githubusercontent.com/ZiangTian/img-bed/main/3dfd92d1ec90efda68282d03bd1a1af.png)

# Notes

> I have finished this lab in October, 2023 but I did not keep a copy of the code on github. Therefore this is a revisit and all the code is hosted and showcased in this repo.

## About `RDT`, `RDH`, `TDT`, and `TDH`

The Software developer's manual explains the head pointers `RDH` and `TDH`:

> This register contains the head pointer for the transmit/receive0 descriptor ring. It holds a value that is an
offset from the base, and indicates the in–progress descriptor. It points to a 16-byte datum.
***Hardware*** controls this pointer. *The only time that software should write to this register is after a
reset (TCTL.RST or CTRL.RST) and before enabling the transmit function (TCTL.EN)*. If
software were to write to this register while the transmit function was enabled, the on-chip
descriptor buffers can be invalidated and indeterminate operation can result. Reading the transmit
descriptor head to determine which buffers have been used (and can be returned to the memory
pool) is not reliable.

This means we should not use or modify these two pointers in our code. For what this lab covers, both transmitting and receiving depend on the tail pointers, which denote the next available buf in the ring, whether we use it for transmitting a new packet or receiving a new packet.

## Locking in RECV

It seems that since `e1000_recv` is interrupt based, we do not need to lock in here. if we acquire the lock prior to modifying the buffers, we get acquire errors. 


## Test passed

![](https://raw.githubusercontent.com/ZiangTian/img-bed/main/20241004114801.png)
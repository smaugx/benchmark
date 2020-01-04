# guide
using similar struct and similar fields as xtopchain to do bench-test;

bench target is msgpack, protobuf and xbase(xdata)

we will do three times for each target, and each time set different chain-binary-data(300B, 1KB, 10KB, 50KB)

# usage

```
$ make
$ ./bench
```


or if you can not make, just run:

```
$ ./bench_bak
```


# test result
bench test in 4 core 8G machine.

![](./bench_msgpack_pro_xdata_1.png)
![](./bench_msgpack_pro_xdata_2.png)

# conclusion

## alloc memory

msgpack  <  protobuf < xdataunit

**msgpack is best**, but almost close, little difference!

## cpu
statictis not very precise, just using linux command `top`.

msgpack, protobuf, xdataunit  cpu 100%

## performance

msgpack < protobuf <  xdataunit

**xdata is best**, little difference with protobuf, msgpack is worst! 

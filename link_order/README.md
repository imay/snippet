# readme

这是一个用于测试`gcc`链接顺序的小程序。

其中`func1.c`以及`func2.c`都实现了同样的函数，在main.c中有调用。

`make link_1_first`: 会生成先连接`func1`的可执行文件

`make link_2_first`: 会生成先连接`func2`的可执行文件

`make link_obj`: 这会报错，如果直接连接的是对象文件，那么会报重名的问题

`make clean`: 清空所有生成的内容

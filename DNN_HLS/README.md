# 工程笔记

[toc]

## 1. mobile net 框架
    
mobile net 基于 google net （inception）发展而来，主要创新点是采用**深度分离卷积**



### 深度可分离卷积
主要是有4个figure的递进介绍：
主要介绍来源：[深度可分离卷积详解](https://www.jianshu.com/p/38dc74d12fcf)

1.     inception 经典模型：每个**分支**都与**所有的in_channel 进行卷积**，具有通道相关性和空间（FM的HxW）相关性的联合映射。
2.     简化了inception，剩余**3分支**，去掉了maxpool等。（还是通道加空间）
3.     对2做了一些规范化，做了一些卷积核整合，基本没什么变化
4.     认为**通道相关性和空间相关性是完全可分的**，3x3xoutchannel 卷积核的个数与1x1的outchannel个数一致。（**一对一，没有多个3x3核对同一个点卷积后累加的操作，只做各自自己的卷积，通道分离**）

### mobile net 
mobile net 基于exception做了一点调整，先3x3 (depth wise), 后使用1x1（Point wise 标准卷积），**后面的通道1x1提供通道聚合的作用**
具体可以参考这个的后半段介绍
[深度可分离卷积详解](https://www.jianshu.com/p/38dc74d12fcf)
[模型进一步介绍](https://blog.csdn.net/mzpmzk/article/details/82976871)
    

## 2. proj Hls 代码阅读

### 框架

1. 先编程基础模块（conv，pool，loading，copy）（在top function中声明实例硬件生成数量为1）
2. 设定buf
2. 先loading bias，weight，pool（上一层的FM暂存在ddr）
3. conv3 relu， conv1 （重复几遍，计算几层）**减少ddr访问次数**
4. copy to ddr
5. ddr load data
6. 3-4。

### 存储设计
#### HLS接口设计

现在hls函数外定义数组buf，作为fpga内存缓存。
**buf包括：weight ，bios，FM，data_buf_copy_to_ddr**

ddr读写数据，定义function的形参（即硬件接口，in，out，inout自动识别），内部derective定义为axi总线实现对应接口。

**load操作，对接axi接口，以数组的形式读数（r,h,w），即只取数组中的一部分，故数据存储时需要做好数据安排，设计好最小存储块的大小。**

#### 存储操作
1. ddr load to buf，对同一层（3x3+1x1+pooling+relu），采用buf0，buf1，乒乓结构缓存，**切分通道成几组**代码有先后顺序，在一个for循环unroll后，采用case，switch的方式，设定每一组对应buf0还是buf1（最后会有一半的组分配到buf0，那么就根据组的不同进行顺序操作，**前面的对buf0的操作未完成，后面的处于等待状态，应该是转换成了状态机**）。（**实体硬件只例化了一个**）
2. 设计中，先load 3x3的数据，计算完conv3x3后，存结果到buf，然后，**弄完所有的3x3后，再进行1x1,（应该是因为1x1是标准卷积，需要全部channel的信息，所以需要等待所有的3x3）**
3. ddr中数据具体怎么存放，根据存储块大小和每次存储的数据，直接设置多维数组，对具体与地址相关的东西，不做考虑。
### TIPS

1. 多加法可以写成tree的结构
2. pading采用存储空间留空，（22x42=20x40）

### 问题
1. conv卷积子函数中for顺序怎么反了（没区别，最后都是累加，而且没有通道合并，先分通道算还是分fm的点算都行，**这里这样的话，就不体现出加法操作了，体现在最后的累加操作中）**
2. for row 和for col 什么用处
3. conv 3后才统一 conv1，是否能流水：不能，conv3是dw conv，conv1是标准conv，需要映射所有通道，那么就得等待所有的介绍。
4. **conv1结果都存到buf15，（同位置）**：通道间的卷积结果是需要累加的（多通道聚合），这样累加到buf15是可以的，类似于1，



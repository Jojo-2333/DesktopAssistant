# Desktop Assistant

## 组件一、记账本Ledger

记账本具有

### 1.文件操作

Ledger记账本的文件基础是ledger.csv逗号分隔值文件。CSV文件设计有四列，分别储存日期、描述、类别、金额。

| Date | Description | Category | Amount |
| ---- | ----------- | -------- | ------ |
|      |             |          |        |

<img src="C:\Users\TangCheng\AppData\Roaming\Typora\typora-user-images\image-20250427161519816.png" alt="image-20250427161519816" style="zoom:50%;" />

ledger.csv储存在程序目录下的account book文件夹内。

程序使用iostream对文件进行读写操作，具体操作会在后文提到。同时使用了filesystem库检测路径是否存在等，提高了程序的健壮性。

### 2.日期类/class date[已弃用]

日期类储存三个整型成员变量（year,month,day），一个字符串成员变量（strdate）。支持构造函数重载，输入(2025,04,25)或(2025-04-25)参数均能正确初始化。

更重要的是，date类支持运算符重载，对date类使用><、=运算符均能正确比较日期，这为后续的账目item排序功能打下基础。

但是，开发过程中我发现string类型的日期只要严格遵照YYYY-MM-DD格式，即使不重载也能正确比较大小。因为string类比较大小是根据ASCII码逐个比较大小的，读取顺序在前面的字符优先级更高，这与日期字符串比较规则恰好相符。因此没有必要再新建一个date类对象进行重载运算符了。

### 3.账目类/class account

账目类设计四个成员变量储存日期、描述、类别、金额（double类型）。其构造函数支持重载，若输入为(string date, string description, string category, double amount) 格式，则直接根据参量新建account对象；若为文件流string格式，则根据","为分隔符进行读取。

此外还有edit函数，会在终端与用户交互获取修改后的账目。

支持各种错误输入检测，健壮性良好。

### 4.账本类/class accounts

账本类包含一个account的vector数组。

账本类的核心文件设计包含在构造与析构函数中。构造函数从文件流ifstream中通过getline读取账目并赋值给vector中的new account对象，并在获取全部条目后进行按日期自动排序。析构函数则将vector数组中每一条写入ledger.csv文件中。

此外，账本类还实现了添加账目、修改账目、删除账目、账目按日期排序、月度账本统计、表格化对齐输出等功能。上述功能全部作为账本类的成员函数实现。

同样，账本类支持各种类型的错误检测：包括路径不存在、文件不存在、自动创建带表头的csv文件等。

### 5.问题与感悟

问题一、getline老是莫名其妙出错，读取不到正确字符串等。

经过排查发现，只需要清除缓冲区就能解决各类疑难杂症。

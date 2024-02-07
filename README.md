# 简介

在嵌入式裸机开发中，经常有任务(Task)和消息队列(Message)的管理需求，直接加入OS又太过复杂了，所以本项目提供适用于嵌入式裸机环境需要进行Task和Message功能需求的人群。

项目开发过程中参考FreeRTOS的Heap管理和[osal: 这是仿照TI的OSAL重写的一个基于事件驱动的框架，主要用于适配资源比较紧张的MCU。 (gitee.com)](https://gitee.com/sqqdfny/osal)的List管理。




# 代码结构

代码结构如下所示：

- **bare**：驱动库，主要包含heap管理，list管理，msg管理和task管理几个部分。
- **main.c**：测试例程。
- **build.mk**和**Makefile**：Makefile编译环境。
- **README.md**：说明文档

```shell
bare_task_msg
 ├── bare
 │   ├── bare_common.h
 │   ├── bare_heap.c
 │   ├── bare_heap.h
 │   ├── bare_list.c
 │   ├── bare_list.h
 │   ├── bare_msg.c
 │   ├── bare_msg.h
 │   ├── bare_task.c
 │   └── bare_task.h
 ├── build.mk
 ├── main.c
 ├── Makefile
 └── README.md
```





# 移植说明

因为涉及到队列管理，项目中也会有前台和后台（中断）等同时使用场景，所以需要进入临界区保护。

`bare_common.h`定义了进出临界区的接口，`__bare_disable_isr()`和`__bare_enable_isr()`。如果应用不在中断中调用，这两个宏可以为空，不然需要根据具体平台情况适配。







# 使用说明

具体如何使用直接看例程就行，非常简单，看函数名和变量名即可。

## Heap管理

代码在`bare_heap.c/.h`中，直接移植的FreeRTOS的Heap实现，实际项目可以换成自己项目的代码。主要实现内存分配与回收，回收内存时，会合并相邻的内存块。



## List管理

代码在`bare_list.c/.h`中，一个双向列表的队列实现。直接拿的这个项目的list实现，[osal: 这是仿照TI的OSAL重写的一个基于事件驱动的框架，主要用于适配资源比较紧张的MCU。 (gitee.com)](https://gitee.com/sqqdfny/osal)。



## msg管理

代码在`bare_msg.c/.h`中，就是一些消息的申请/释放管理，非常简单。

`struct bare_msg`结构体包含消息的基本信息，消息ID`id`，参数长度`param_len`，和消息数据`param`。`node`用于msg队列管理。

```c
typedef struct bare_msg
{
    struct bare_list_head node;

    uint16_t id;
    uint16_t param_len;
    uint8_t param[];   ///< Parameter embedded struct. Must be word-aligned.
} bare_msg_t;
```



## task管理

代码在`bare_task.c/.h`中，就是一些任务的申请/释放管理，每个任务独立维护一个消息队列，所有任务放在任务列表下管理，方便不同task使用需求。

`struct bare_task`结构体包含一个消息队列`msg_list`和一个任务执行函数`func`，`node`用于task队列管理。

```c
typedef int (*bare_task_func_t)(struct bare_msg *msg);

typedef struct bare_task
{
    struct bare_list_head node;

    struct bare_list_head msg_list;

    bare_task_func_t func;
} bare_task_t;
```



# 测试说明

## 环境搭建

目前测试暂时只支持Windows编译，最终生成exe，可以直接在PC上跑。

目前需要安装如下环境：
- GCC环境，笔者用的msys64+mingw，用于编译生成exe，参考这个文章安装即可。[Win7下msys64安装mingw工具链 - Milton - 博客园 (cnblogs.com)](https://www.cnblogs.com/milton/p/11808091.html)。


## 编译说明

本项目都是由makefile组织编译的，编译整个项目只需要执行`make all`即可。


也就是可以通过如下指令来编译工程：

```shell
make all
```

而后运行执行`make run`即可运行例程，例程中实现了2个Task的消息管理，并实现了Task Save的相关逻辑。

```shell
PS D:\workspace\github\bare_task_msg> make run
Building   : "output/main.exe"
Start Build Image.
objcopy -v -O binary output/main.exe output/main.bin
copy from `output/main.exe' [pei-i386] to `output/main.bin' [binary]
objdump --source --all-headers --demangle --line-numbers --wide output/main.exe > output/main.lst
Print Size
   text    data     bss     dec     hex filename
  41452    7040    2644   51136    c7c0 output/main.exe
./output/main.exe
Heap Remain Size: 0xff8
Task Start Work!
user_task1(), id: 0x1, len: 0
BARE_TASK_HDL_SAVED
user_task2(), id: 0x8, len: 0
user_task2(), id: 0x9, len: 0
user_task2(), id: 0x50, len: 20
0x0:0x1:0x2:0x3:0x4:0x5:0x6:0x7:0x8:0x9:0xa:0xb:0xc:0xd:0xe:0xf:0x10:0x11:0x12:0x13:
BARE_TASK_HDL_SAVED
user_task1(), id: 0x1, len: 0
user_task1(), id: 0x2, len: 0
user_task1(), id: 0x10, len: 10
0x0:0x1:0x2:0x3:0x4:0x5:0x6:0x7:0x8:0x9:
user_task1(), id: 0x11, len: 10
0x10:0x11:0x12:0x13:0x14:0x15:0x16:0x17:0x18:0x19:
user_task2(), id: 0x50, len: 20
0x0:0x1:0x2:0x3:0x4:0x5:0x6:0x7:0x8:0x9:0xa:0xb:0xc:0xd:0xe:0xf:0x10:0x11:0x12:0x13:
user_task2(), id: 0x51, len: 20
0x10:0x11:0x12:0x13:0x14:0x15:0x16:0x17:0x18:0x19:0x1a:0x1b:0x1c:0x1d:0x1e:0x1f:0x20:0x21:0x22:0x23:
user_task2(), id: 0x51, len: 100
0x50:0x51:0x52:0x53:0x54:0x55:0x56:0x57:0x58:0x59:0x5a:0x5b:0x5c:0x5d:0x5e:0x5f:0x60:0x61:0x62:0x63:0x64:0x65:0x66:0x67:0x68:0x69:0x6a:0x6b:0x6c:0x6d:0x6e:0x6f:0x70:0x71:0x72:0x73:0x74:0x75:0x76:0x77:0x78:0x79:0x7a:0x7b:0x7c:0x7d:0x7e:0x7f:0x80:0x81:0x82:0x83:0x84:0x85:0x86:0x87:0x88:0x89:0x8a:0x8b:0x8c:0x8d:0x8e:0x8f:0x90:0x91:0x92:0x93:0x94:0x95:0x96:0x97:0x98:0x99:0x9a:0x9b:0x9c:0x9d:0x9e:0x9f:0xa0:0xa1:0xa2:0xa3:0xa4:0xa5:0xa6:0xa7:0xa8:0xa9:0xaa:0xab:0xac:0xad:0xae:0xaf:0xb0:0xb1:0xb2:0xb3:
Task End Work!
Heap Remain Size: 0xff8
Executing 'run: all' complete!
```

可以看到，2个task同时支持消息处理，当有Save的场景会pending消息，先执行另外一个task，task执行完所有消息后，heap保持不变。




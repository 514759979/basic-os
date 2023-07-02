# BasicOS项目说明

邮箱：**event-os@outlook.com**，微信号：**Event-OS**，QQ群：**667432915**。兄弟项目：[EventOS Nano](https://gitee.com/event-os/eventos-nano.git)

-------
### 一、BasicOS是什么？
**BasicOS**，是一个简洁、创新的协作式内核。它的特点有二：一是协作式，不抢占，按优先级轮询，当前任务不释放CPU控制权，其他任务得不到CPU，二是超级轻量（ROM 968字节，RAM 64字节，-O3）。目前提供出来的功能非常基础，主要有两部分，一是任务功能，二是软定时器功能。
任务功能如下：
+ 任务创建。
+ 任务退出，
+ 任务内延时。
+ 获取系统时间。

软定时器功能如下：
+ 软定时器的创建
+ 软定时器暂停
+ 软定时器继续
+ 软定时器重启（重新计时）
+ 软定时器删除

这个代码，功能虽然少，但对于任务并发来说，已经完全足够了。由于是协作内核，任务之间不抢占，完全可以十分放心的使用全局变量，进行任务间通信和信息共享。

之所以启动**BasicOS**这个项目，是因为[EventOS Nano](https://gitee.com/event-os/eventos-nano.git)项目释放出来后，我发现事件驱动的概念，对一部分嵌入式工程师来说，还是有些不习惯。**EventOS Nano**实际上引入了多个概念，比如面向对象、事件驱动、控制反转等等，这些概念交织在**EventOS Nano**的源码里，很难一下让人接受下来。**BasicOS**就诞生了。**BasicOS**在传统的任务机制和**EventOS Nano**的事件机制之间，提供了一个过渡。

另外，**BasicOS**还要启动一个新的开发模式，那就是它从一开始，只提供最小功能，然后充分听取用户的意见，按照大多数用户的意见开发。这样做的目的，是为将来的**EventOS**的技术走向，收集需求。

但无论怎么样，有两点是没有办法变的：
1. **协作，不抢占**。抢占的内核，FreeRTOS、uC/OS-II、RTX和RT-Thread等，已经做的足够好了。**BasicOS**不重复制造轮子。
1. **事件功能**。事件，是**EventOS**项目的灵魂，是核心，也是**EventOS**的名字与招牌。添加事件功能，不意味着，回到事件驱动上。所谓事件驱动，用户代码在有事件发生时才会执行，没有事件的驱动，就不产生执行。**BasicOS**采用传统的任务机制，显然不是这样的。在**BasicOS**下，即使有了事件功能，也仅仅是任务间的通信机制，用户自己决定什么时候接收事件。没有事件的驱动，用户代码依然在任务中运行。当然，用户也完全可以不使用事件功能，甚至裁剪掉事件功能。

这样，**BasicOS**、**EventOS Nano**和**EventOS**三个版本各自承担的就比较清晰了：
+ **BasicOS**支持传统的任务模式，以协作内核、简洁和超轻量为特色，不涉及事件驱动，不强制面向对象，不增加复杂特性。定位于解决简单应用的并发问题。
+ **EventOS Nano**支持事件驱动、面向对象、状态机框架，以事件驱动和轻量为特色。定位于熟悉事件驱动与状态机框架，或者感兴趣的用户。
+ **EventOS**的技术方向，到底是任务模式，还是事件驱动，完全取决于**Basic**和**Nano**那个更受欢迎。**EventOS**支持抢占内核，支持硬实时，支持多个组件（设备框架、Shell、Log、嵌入式数据库、文件系统和网络），以分布式特性、跨平台开发为技术特色。定位于复杂的大型MCU项目的应用。

### 二、BasicOS未来可能扩展的功能
+ 事件桥（与**EventOS**连接构成分布式应用）

线程锁、信号量，都没有必要在**BasicOS**增加，因为**BasicOS**是协作式，非抢占的内核。
邮箱与消息队列等线程间通信功能，可以对全局变量进行数据结构的合理组织，很方便的实现。

### 三、代码例程

使用时，要遵循以下注意点。
+ 不要使用死循环阻塞代码，这样的话，其他任务会得不到CPU。
+ 要注意，在合适的时候，经常释放CPU。
+ 任务间的全局变量可以放心使用，但要注意对全局变量的数据结构进行良好的设计和定义；中断与任务间的全局变量，仍需关中断来保护数据的完整性。

任务启动的过程如下所示：
``` C
/* data --------------------------------------------------------------------- */
static uint64_t stack_led[32];
eos_task_t led;

static uint64_t stack_count[32];
eos_task_t count;

static uint64_t stack_test_exit[32];
eos_task_t test_exit;

/* main function ------------------------------------------------------------ */
int main(void)
{
    if (SysTick_Config(SystemCoreClock / 1000) != 0)
        while (1);
    
    static uint64_t stack_idle[32];
    eos_init(stack_idle, sizeof(stack_idle));       // EventOS初始化
    
    // 启动LED闪烁任务
    eos_task_start(&led, task_entry_led, 1, stack_led, sizeof(stack_led));

    // 启动计数任务
    eos_task_start(&count, task_entry_count, 2, stack_count, sizeof(stack_count));

    eos_run();                                      // EventOS启动

    return 0;
}
```

任务函数的实现如下。在任务里也可以启动其他任务。
``` C
uint8_t led_status = 0;
static void task_entry_led(void)
{
    eos_task_start(&test_exit, task_entry_test_exit, 3, stack_test_exit, sizeof(stack_test_exit));
    
    while (1) {
        led_status = 0;
        eos_delay_ms(500);
        led_status = 1;
        eos_delay_ms(500);
    }
}
```

任务退出时，其应用如下所示。
``` C
uint32_t count_num_exit = 0;
static void task_entry_test_exit(void)
{
    for (int i = 0; i < 10 ; i ++) {
        count_num_exit ++;
        eos_delay_ms(10);
    }
    
    eos_exit();
}
```

系统滴答函数，放在定时器中断里，最好放在SysTick中断里。
``` C
void SysTick_Handler(void)
{
    eos_tick();
}
```

在目前的移植中，M0 - M7全系列的ARM Cortex-M的例程已经齐备，已经支持MDK AC5和AC6。但有一个需要注意的点，也就是现在的移植，仍然不支持FPU。需要在MDK的工程设置里，关闭浮点数单元的使用。如图所示。在没有用户明确提出对FPU进行支持的前提下，**BasicOS**将不会考虑对FPU的支持。

![avatar](/documentation/fpu_disable.png)

### 四、代码结构
#### **核心代码**
+ **eventos/eventos.c** **BasicOS**内核源码
+ **eventos/eventos.h** 头文件

#### **例程代码**
+ **stm32f030** 对ARM Cortex-M0芯片例程。
+ **stm32f103** 对ARM Cortex-M3芯片例程。
+ **stm32f429** 对ARM Cortex-M4芯片例程。
+ **stm32h743** 对ARM Cortex-M7芯片例程。

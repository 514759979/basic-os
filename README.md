# BasicOS项目说明

邮箱：**event-os@outlook.com**，微信号：**Event-OS**，QQ群：**667432915**。兄弟项目：[EventOS Nano](https://gitee.com/event-os/eventos-nano.git)

-------
## 一、BasicOS是什么？
**BasicOS**，是一个简洁、创新的协作式内核。它最大的技术特点，就是共享栈，也就是所有的任务共享一个任务栈，同时又能提供完整线程锁模型。**BasicOS**定位于小RAM的MCU（8K RAM）。需要说明的一点，**BasicOS**不是一个抢占式的RTOS，它遵循几个重要原则，简单，易用，低资源占用，因此它取名为**Basic**。

#### 为什么要为**BasicOS**开发共享栈这个特性？

在开发**BasicOS**这个项目之前，狗哥先开发了**EventOS**和**EventOS Nano**这个项目。[EventOS Nano](https://gitee.com/event-os/eventos-nano.git)这个项目，其实就是超低资源占用的，它占用的资源，甚至比**BasicOS**还要低不少。我发现事件驱动的概念，对一部分嵌入式工程师来说，入门的门槛还是高了些。**EventOS Nano**实际上引入了多个概念，比如面向对象、事件驱动、控制反转等等，这些概念交织在**EventOS Nano**的源码里，很难一下让人接受下来。即使是入了门，事件驱动对于编写间接且阅读友好的代码，依然相当不利。

但传统的RTOS中，任务会消耗大量的RAM，对于8KB-20KB RAM这个量级的MCU来说，使用RTOS多有不便。狗哥受到网友光哥的启发，决定开发一个共享栈的RTOS。基于上述的原因，**BasicOS**就诞生了。

目前这个共享栈的特性，在32位ARM芯片的RTOS中，还是独一无二的。后面有对这个技术的原理，做详细阐述。这个也正是**BasicOS**的核心技术与核心特性，会被大力发展下去。

#### BasicOS为什么是协作式内核？
关于这一点，原因很简单。狗哥做嵌入式开发那么多年，发现一件事，硬实时的产品，所占比例，真的很少。

首先什么是硬实时，也就是，响应时间在毫秒，甚至毫秒以下，才能称为硬实时。绝大部分的嵌入式产品，都没有这么高的实时性要求。协作式内核，在被合理使用的前提下，达到10ms甚至更快地响应，非常容易，能够满足大部分产品的需求。

而使用抢占式内核，却带来不少的负面问题。并发问题，与共享资源的竞争问题，是多任务（多线程）编程中的一个难点问题。很多BUG产生在这些问题上，而且调试解决非常不便。而**BasicOS**是协作式内核，CPU的释放和获取，完成由用户（工程师）决定，也就是没有资源竞争的问题。这样，代码的可靠性，在底层机制上，就得到了保障。

### 二、共享栈技术
多个任务如何共享一个栈呢？

![avatar](/documentation/BasicOS-Stack.png)

如图所示。当一个任务运行时，它被赋予了最大的堆栈空间。 发生任务切换时，前一个任务堆栈将收缩并重新定位，并且当前任务的堆栈将展开并重新定位。因此，**BaiscOS**的任务切换时间，是大于一般的RTOS的。因为**BasicOS**不仅要进行上一个任务的入栈和下一个任务的出栈，还要对任务的栈数据进行移动。

### 三、BasicOS未来的技术走向
#### 1. 持续改进共享栈技术

共享栈技术，是**BasicOS**坚持发展的核心技术特性和技术特色，也是**BasicOS**定位于小RAM MCU的根本所在。**BasicOS**将持续改进所使用的共享栈技术，将RAM占用降至更低。

#### 2. 与EventOS进行结合
**EventOS**采用事件驱动技术，虽然有占用RAM低的优点，但也有一些缺点。最大的缺点，是不支持线程内阻塞。但如果引入RTOS作为**EventOS**的调度内核，则会破坏**EventOS**占用RAM低的优点。**BasicOS**则完美的解决了这一问题。将**EventOS**和**BasicOS**结合后，**EventOS**的性能和灵活性将得到进一步增强。

#### 3. 加入其他RTOS的基础设施

RTOS常见的基础设施，如消息队列，信号量等，也会被**BasicOS**所支持。

#### 4. 加入eLab项目，并支持CMSIS-RTOS标准


### 四、代码例程

使用时，要遵循以下注意点。
+ 不要使用死循环阻塞代码，这样的话，其他任务会得不到CPU。
+ 要注意，在合适的时候，经常释放CPU。
+ 任务间的全局变量可以放心使用，但要注意对全局变量的数据结构进行良好的设计和定义；中断与任务间的全局变量，仍需关中断来保护数据的完整性。

任务启动的过程如下所示：
``` C
/* main function ------------------------------------------------------------ */
int main(void)
{
    bsp_init();

    static uint8_t stack[4096];
    basic_os_init(stack, sizeof(stack));            /* BasicOS初始化 */
    basic_os_run();                                 /* BasicOS启动 */

    return 0;
}
```

为了极限压缩RAM的占用，**BasicOS**采用export机制进行任务的创建，每个任务仅仅占用16个字节。
``` C
static void task_entry_led(void)
{
    uint8_t led_status = 0;

    while (1)
    {
        led_status = 0;
        bos_delay_ms(500);
        led_status = 1;
        bos_delay_ms(500);
    }
}
bos_task_export(led, task_entry_led, 2, NULL);
```

系统滴答函数，放在定时器中断里，最好放在SysTick中断里。
``` C
void SysTick_Handler(void)
{
    bos_tick();
}
```

在目前的移植中，M0 - M7全系列的ARM Cortex-M的例程已经齐备，已经支持MDK AC5和AC6。但有一个需要注意的点，也就是现在的移植，仍然不支持FPU。原因在于，如果支持硬件FPU的话，**BasicOS**定位于小RAM芯片，这类基于小RAM芯片开发的应用，一般没有大量的浮点数操作，如果支持硬件FPU的话，会导致**BasicOS**占用的RAM大大增加，这与**BasicOS**的初衷不匹配。

需要在MDK的工程设置里，关闭浮点数单元的使用。如图所示。在没有用户明确提出对FPU进行支持的前提下，**BasicOS**将不会考虑对FPU的支持。

![avatar](/documentation/fpu_disable.png)

### 五、代码结构
#### **核心代码**
+ **BasicOS/basic_os.c** **BasicOS**内核源码
+ **BasicOS/basic_os.h** 头文件

#### **例程代码**
+ **01_basic_os_iar** 对**IAR ARM Cortex-M0**芯片例程。
+ **02_basic_os_mdk** 对**MDK ARM Cortex-M0**芯片例程。

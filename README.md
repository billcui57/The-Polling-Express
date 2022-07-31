# The Polling Express

<img width="1061" alt="image" src="https://user-images.githubusercontent.com/30924631/182046343-393f66cc-725e-450d-ad99-cfd1d8da68a3.png">

## Architecture

<img width="708" alt="image" src="https://user-images.githubusercontent.com/30924631/182046411-c08b01ab-6441-4a12-a604-b8ad0447d39f.png">

One part of the codebase is a small, real-time-capable operating system that runs on stand-alone computers and provides a low-level interface to control Märklin electric trains. The second part is concerned with building a control application for the trains.

The kernel and user programs are built for the TS-7200 single-board computer based on the EP9302 system-on-a-chip (SoC) that uses an ARM 920T processor. The processor implements ARM architecture version ARMv4T.

## CS 452/652 Spring 2022 - Kernel Description

Programming Interface
---------------------

### Task Creation

*   int Create(int priority, void (\*function)())
    
    allocates and initializes a task descriptor, using the given priority, and the given function pointer as a pointer to the entry point of executable code, essentially a function with no arguments and no return value. When Create returns, the task descriptor has all the state needed to run the task, the task’s stack has been suitably initialized, and the task has been entered into its ready queue so that it will run the next time it is scheduled.
    
    _Return Value_  
    
    tid
    
    the positive integer task id of the newly created task. The task id must be unique, in the sense that no other active task has the same task id.
    
    \-1
    
    invalid priority.
    
    \-2
    
    kernel is out of task descriptors.
    
    _Comment_: If newly created tasks need to receive configuration parameters, these generally need to be provided via message passing. 
    
*   int MyTid()
    
    returns the task id of the calling task.
    
    _Return Value_  
    
    tid
    
    the positive integer task id of the task that calls it.
    
    _Comment_: Errors should be impossible!
    
*   int MyParentTid( )
    
    returns the task id of the task that created the calling task. This will be problematic only if the task has exited or been destroyed, in which case the return value is implementation-dependent.
    
    _Return Value_  
    
    tid
    
    the task id of the task that created the calling task.
    
    _Comment_: The return value is implementation-dependent, if the parent has exited, has been de- stroyed, or is in the process of being destroyed.
    
*   void Yield()
    
    causes a task to pause executing. The task is moved to the end of its priority queue, and will resume executing when next scheduled.
    
    _Comment_: Like every entry to the kernel, Yield() reschedules.
    
*   void Exit()
    
    causes a task to cease execution permanently. It is removed from all priority queues, send queues, receive queues and event queues. Resources owned by the task, primarily its memory and task descriptor, are not reclaimed.
    
    _Comment_: Exit does not return at all. If a point occurs where all tasks have exited, the kernel should return cleanly to RedBoot.
    
*   void Destroy()
    
    imples Exit(), but also releases the task's memory and task descriptor.
    
    _Comment_: This functionality is not implemented. Re-using resources is complicated.
    

### Message Passing

*   int Send(int tid, const char \*msg, int msglen, char \*reply, int rplen)
    
    sends a message to another task and receives a reply. The message, in a buffer in the sending task’s memory, is copied to the memory of the task to which it is sent by the kernel. Send() supplies a buffer into which the reply is to be copied, and the size of the reply buffer, so that the kernel can detect overflow. When Send() returns without error it is guaranteed that the message has been received, and that a reply has been sent, not necessarily by the same task. The kernel will not overflow the reply buffer. If the size of the reply set exceeds rplen, the reply is truncated and the buffer contains the first rplen characters of the reply. The caller is expected to check the return value and to act accordingly. There is no guarantee that Send() will return. If, for example, the task to which the message is directed never calls Receive(), Send() never returns and the sending task remains blocked forever. Send() has a passing resemblance, and no more, to a remote procedure call.
    
    _Return Value_  
    
    \>-1
    
    the size of the message returned by the replying task. The actual reply is less than or equal to the size of the reply buffer provided for it. Longer replies are truncated.
    
    \-1
    
    tid is not the task id of an existing task.
    
    \-2
    
    send-receive-reply transaction could not be completed.
    
*   int Receive(int \*tid, char \*msg, int msglen)
    
    blocks until a message is sent to the caller, then returns with the message in its message buffer and tid set to the task id of the task that sent the message. Messages sent before Receive() is called are retained in a send queue, from which they are received in first-come, first-served order. The argument msg must point to a buffer at least as large as msglen. The kernel will not overflow the message buffer. If the size of the message set exceeds msglen, the message is truncated and the buffer contains the first msglen characters of the message sent. The caller is expected to check the return value and to act accordingly.
    
    _Return Value_  
    
    \>-1
    
    the size of the message sent by the sender (stored in tid). The actual message is less than or equal to the size of the message buffer supplied. Longer messages are truncated.
    
*   int Reply(int tid, const char \*reply, int rplen)
    
    sends a reply to a task that previously sent a message. When it returns without error, the reply has been copied into the sender’s memory. The calling task and the sender return at the same logical time, so whichever is of higher priority runs first. If they are of the same priority, the sender runs first.
    
    _Return Value_  
    
    \>-1
    
    the size of the reply message transmitted to the original sender task. If this is less than the size of the reply message, the message has been truncated.
    
    \-1
    
    tid is not the task id of an existing task.
    
    \-2
    
    tid is not the task id of a reply-blocked task.
    

### Name Server

This section comprises the interface of the name server. Knowing how and where to start name resolution is generally referred to as _closure mechanism_ and must be implicit. Therefore this interface does not include a task id.

*   int RegisterAs(const char \*name)
    
    registers the task id of the caller under the given name. On return without error it is guaranteed that all WhoIs() calls by any task will return the task id of the caller until the registration is overwritten. If another task has already registered with the given name, its registration is overwritten. RegisterAs() is actually a wrapper covering a send to the name server.
    
    _Return Value_  
    
    0
    
    success.
    
    \-1
    
    invalid name server task id inside wrapper.
    
*   int WhoIs(const char \*name)
    
    asks the name server for the task id of the task that is registered under the given name. Whether WhoIs() blocks waiting for a registration or returns with an error, if no task is registered under the given name, is implementation-dependent. There is guaranteed to be a unique task id associated with each registered name, but the registered task may change at any time after a call to WhoIs(). WhoIs() is actually a wrapper covering a send to the name server.
    
    _Return Value_  
    
    tid
    
    task id of the registered task.
    
    \-1
    
    invalid name server task id inside wrapper.
    

### Interrupt Processing

Only one generic primitive is required for interrupt processing.

*   int AwaitEvent(int eventid)
    
    blocks until the event identified by eventid occurs then returns with volatile data, if any.
    
    _Return Value_  
    
    \>-1
    
    volatile data, in the form of a positive integer.
    
    \-1
    
    invalid event.
    
    \-2
    
    corrupted volatile data.
    

### Clock Server

The size of a clock tick is normally application dependent. In CS 452/652 it is 10 milliseconds, the time in which a train at top speed travels about 5-6 millimetres.

*   int Time(int tid)
    
    returns the number of ticks since the clock server was created and initialized. With a 10 millisecond tick and a 32-bit unsigned int for the time wraparound is almost 12,000 hours, plenty of time for our demo. Time is actually a wrapper for a send to the clock server. The argument is the tid of the clock server.
    
    _Return Value_  
    
    \>-1
    
    time in ticks since the clock server initialized.
    
    \-1
    
    tid is not a valid clock server task.
    
*   int Delay(int tid, int ticks)
    
    returns after the given number of ticks has elapsed. How long after is not guaranteed because the caller may have to wait on higher priority tasks. Delay() is (almost) identical to Yield() if ticks is zero. Delay() is actually a wrapper for a send to the clock server.
    
    _Return Value_  
    
    \>-1
    
    success. The current time returned (as in Time())
    
    \-1
    
    tid is not a valid clock server task.
    
    \-2
    
    negative delay.
    
*   int DelayUntil(int tid, int ticks)
    
    returns when the time since clock server initialization is greater or equal than the given number of ticks. How long after is not guaranteed because the caller may have to wait on higher priority tasks. Also, DelayUntil(tid, Time(tid) + ticks) may differ from Delay(tid, ticks) by a small amount. DelayUntil is actually a wrapper for a send to the clock server.
    
    _Return Value_  
    
    \>-1
    
    success. The current time returned (as in Time())
    
    \-1
    
    tid is not a valid clock server task.
    
    \-2
    
    negative delay.
    

### Input/Output

*   int Getc(int tid, int uart)
    
    returns next unreturned character from the given UART. The first argument is the task id of the appropriate server. How communication errors are handled is implementation-dependent. Getc() is actually a wrapper for a send to the appropriate server.
    
    _Return Value_  
    
    \>-1
    
    new character from the given UART.
    
    \-1
    
    tid is not a valid uart server task.
    
*   int Putc(int tid, int uart, char ch)
    
    queues the given character for transmission by the given UART. On return the only guarantee is that the character has been queued. Whether it has been transmitted or received is not guaranteed. How communication errors are handled is implementation-dependent. Putc() is actually a wrapper for a send to the appropriate server.
    
    _Return Value_  
    
    0
    
    success.
    
    \-1
    
    tid is not a valid uart server task.
    

Algorithms and Data Structures
------------------------------

All algorithms must be fast in the sense that we can bound execution time at a reasonable level. ‘Reasonable’ means on a time scale appropriate for a train application. All data must be either static or on the stack. There is no heap and thus no dynamic memory allocation.

### Task Descriptors

#### Basics

The most important data structure in the kernel is the array of task descriptors (TDs), which is allocated in kernel memory during kernel initialization. Every existing task has a TD allocated to it. A TD normally includes at least

1.  a task identifier (tid), which is unique among all active tasks,
2.  a pointer to the TD of the task that created it, its parent,
3.  the task’s priority,
4.  a pointer to the TD of the next task in the task’s ready queue,
5.  a point to the TD of the next task on the task’s send queue,
6.  the task’s current run state, and
7.  the task’s current stack pointer.

The remainder of the task’s state is saved either in the TD or on the stack. This includes

*   the task’s return value, and
*   the task’s SPSR.

#### Comments

1.  When Destroy() is not implemented the task id can be a simple array index because TDs are not re-used. When Destroy() is implemented a better task id model is needed.
2.  A task is in one of the following run states.
    1.  Active. The task that has just run, is running, or is about to run. Scheduling, which happens near the end of kernel processing, changes the active task. On a single processor only one task can be active at a time.
    2.  Ready. The task is ready to be activated.
    3.  Zombie. The task will never again run, but still retains it resources: memory, TD, etc.
    4.  Send-blocked. The task has executed Receive(), and is waiting for a task to sent to it.
    5.  Receive-blocked. The task has executed Send(), and is waiting for the message to be received.
    6.  Reply-blocked. The task has executed Send() and its message has been received, but it has not received a reply.
    7.  Event-blocked. The task has executed AwaitEvent(), but the event on which it is waiting has not occurred.
3.  The first three of these states are needed for task creation; the next three are needed for message passing; and the seventh is needed for hardware interrupts.
4.  The parent is the active task when a task is being created. This entails that the variable storing the active task is written only by the scheduler.
5.  A task has memory reserved for it when it is created, which it uses for its stack. The values of its registers are placed on the stack (or in the TD) when it is not executing.
6.  Each task has, when it is running, a program status register (CPSR), which is saved when it is interrupted and re-installed when the task is next activated.
7.  The task context switch also needs to save and restore other registers. These can be saved on the stack or in the TD.
8.  Tasks usually enter the kernel with requests for service. Many tasks must be provided with return values that indicate the result of the request. Because a task may not be rescheduled immediately the return value must be saved.

### Scheduling

#### Priorities

Scheduling is done using static priorities: higher priority tasks that are ready to execute are guaranteed to execute before lower priority ones.

1.  The number of priorities is implementation-dependent.
2.  There can be more than one task at any priority. As tasks become ready to run, they are put on the end of their ready queue. The next task to run is always the highest priority task that is ready. If more than one task is ready at the highest priority, then the one at the head of the ready queue runs.
3.  A task instance may not be at more than one priority.

#### Round-Robin

While a task can have only a single priority; several tasks may have the same priority. They are scheduled round-robin. Each time scheduling occurs at a priority, the least recently readied task is scheduled, requiring something like a first-in, first-out queue.

#### No Ready Tasks

Occasionally, or even frequently, all ready queues are empty with one or more tasks being Event-blocked. In this case we have an idle task at the lowest priority, which does anything we want while it waits to be interrupted

#### Exiting from the Kernel

The kernel should exit cleanly to RedBoot when there are no Event-blocked tasks and no tasks in the ready queues. Kernels do not normally exit.

Context Switching
-----------------

Those context switches into the kernel that occur when a running task requests a kernel service must be implemented using the ARM SWI instruction. Context switches generated by interrupts are defined by the architecture. The ARM architecture offers a variety of methods for exiting the kernel into a task.

Interrupts
----------

The kernel should run with interrupts disabled. Thus, as little as possible should be done by the kernel because the time of the slowest kernel primitive must be added to the worst case response time of every action.

AwaitEvent() requires a table of event ids, which is, in essence a catalogue of hardware events to which the kernel and servers are able to provide a response. This is an awkward intrusion of hardware configuration into an otherwise clean operating system abstraction. There are many different ways of handling this issue within operating system design. Fortunately for us, the hardware environment in which we work is circumscribed and well-defined, so that we can create a small registry of events, provide a reasonably consistent set of responses, and not think too much about the general problem.

Message Formats
---------------

Messages are character arrays containing the contents of a struct.





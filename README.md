# FreeRTOS-Sporadic-Server
RAF SRV 2020 Project - FreeRTOS Sporadic Server Implementation

This project is an implementation of a static sporadic server on top of the FreeRTOS port for Windows. Since Windows is not a suitable platform for real-time systems, task times could be erratic. 
The sporadic server is a proof of concept but most of the changes were done in tasks.c so porting to other platforms could be possible. Features implemented inside the kernel include:

-Periodic task support <br/>
-Rate Monotonic scheduling <br/>
-Hyperbolic schedulability bound testing <br/>
-Submitting tasks in a batch <br/>
-Queueing aperiodic tasks whose arrival times are further ahead <br/>

Configuring the simulated real-time environment is done through the command line. A GUI for setting up the environment is implemented in the <a href="https://github.com/Androoideka/Sporadic-Server-Batcher">Sporadic Server Batcher repository<a/> and includes a visual representation of actions the system is taking each tick. You can also operate the system manually using these commands:

add_task |name| |function| |function parameters| |arrival offset| - adds an aperiodic task to the batch with the given attributes <br/>
add_task_periodic |name| |function| |function parameters| |period| - adds a periodic task to the batch with the given attributes <br/>
stop_task |handle| - stops the task with the given handle and should only be done after the scheduler is started <br/>
configure_stats |period| - enables writing stats to a file at the given interval <br/>
get_max_server_capacity |period| - calculates the maximum capacity of the server given the period <br/>
initialise_server <capacity> |period| - attempts to start the scheduler with the tasks previously added to the batch and assigns the given period and capacity to the server <br/>
show_task_codes - prints out the available functions with worst-case running times that a task can execute <br/>
recalculate_wcet - recalculates worst-case running times for all available functions <br/>

|name| - string value with maximum length equal to configMAX_TASK_NAME_LEN which is set to 12 by default <br/>
|function| - string value with maximum length equal to FUNC_NAME_SIZE which is set to 17 by default. The string has to be equal to the name of one of the functions in function.c <br/>
|function parameters| - string value with maximum length equal to configMAX_PARAM_LEN which is set to 16 by default <br/>
|arrival offset| - number value that specifies how many ticks after the current tick will pass before the task arrives <br/>
|period| - number value strictly above 0 to avoid zero division <br/>
|handle| - pointer value for the task that should be stopped <br/>

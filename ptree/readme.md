This project adds a system call to the kernel.
Just "make" in ptree folder and then insmod in Android shell(insmod ptree.ko).
It will show the information of all process, and the relationships between parent and child processes by tabs.

In the initial program, 
the system call number is 273(line 12),
the max number of processes is 60000(line 13),
if it is needed, please change it(and the proper location in "test").

You can call it by "test".

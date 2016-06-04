This project is to test syscall "ptree".
Just "ndk-build" in jni folder and then run in Android shell(./ptree).
It will print the entire process tree (in DFS order)using tabs to indent children with respect to their parents.

In the initial program, 
the max number of processes is 60000(line 2),
the system call number is 273(line 18),
if it is needed, please change the maximum value(and the proper location in "ptree").

The "testscript" file contains one run of my program.

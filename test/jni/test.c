#include <stdio.h>
#define MAXBUF 60000
struct prinfo{
    pid_t parent_pid; 		    // process id of parent
    pid_t pid;			        // process id
    pid_t first_child_pid;	    // pid of youngest child
    pid_t next_sibling_pid; 	// pid of older sibling
    long state;			        // current state of process
    long uid;			        // user id of process owner
    char comm[64];		        // name of program executed
    int deep;
};

int main(){
  struct prinfo buf[MAXBUF];
  struct prinfo p;
  int nr;
  syscall(273,buf,&nr);
  printf("The process number is %d!\n",nr);

  int i = 0, j = 0;
  for (i = 0; i < nr; ++i)
  {
	// if (p.comm == NULL) continue;
    	p = buf[i];
        for (j = 0; j < p.deep; ++j)
	    printf("	");
	printf("%s, %d, %ld, %d, %d, %d, %ld\n", p.comm, p.pid, p.state,
	p.parent_pid, p.first_child_pid, p.next_sibling_pid, p.uid);
  }

  return 0;
}

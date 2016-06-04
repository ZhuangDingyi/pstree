#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/sched.h>         // for task_struct
#include<linux/unistd.h>
#include<linux/list.h>
#include<linux/string.h>
#include<linux/syscalls.h>
#include<linux/slab.h>          // for memory allocation
#include<linux/uaccess.h>       // for copy to user
MODULE_LICENSE("Dual BSD/GPL");
#define __NR_ptree 273
#define MAXBUF 60000
struct prinfo{
    pid_t parent_pid; 		    // process id of parent
    pid_t pid;			        // process id
    pid_t first_child_pid;	    // pid of youngest child
    pid_t next_sibling_pid; 	// pid of older sibling
    long state;			        // current state of process
    long uid;			        // user id of process owner
    char comm[64];		        // name of program executed
    int deep;			// for
};

int taskNum = 0;		// the number of copied tasks
struct prinfo tasks[MAXBUF];

// copy data from task_struct to prinfo with deep d 
void copyTask(struct task_struct *t, struct prinfo *b, int d)
{
    struct task_struct *curr;
    struct list_head *tl; 

    if (t->comm == NULL)
	return;
    ++taskNum; 

    b->parent_pid = t->parent->pid;
    b->pid = t->pid;
    b->state = t->state;
    b->uid = t->cred->uid;
    b->deep = d;
    get_task_comm(b->comm, t);
    
    // consider its first_child_pid in different situations
    if (list_empty(&t->children))
        b->first_child_pid = 0;
    else
    {
	// the first child is itself
        curr = list_entry((&t->children)->next, struct task_struct, sibling);
        if (curr == NULL)
            b->first_child_pid = 0;
        else
            b->first_child_pid = curr->pid;
    }
    
    // consider its next_sibling_pid
    b->next_sibling_pid = 0;
    if (list_empty(&t->sibling))
    {
	// ergodic its silbling
        list_for_each(tl, &t->parent->children)
        {
            curr = list_entry(tl->next, struct task_struct, sibling);
            if (curr->pid == t->pid)
            {
                curr = list_entry(tl->next, struct task_struct, sibling);
                if (curr == NULL)
                    b->next_sibling_pid = 0;		// b is the oldest in its brothers
                else
                    b->next_sibling_pid = curr->pid;
                break;
            }
        }
    }
    printk("Copy success: comm: %s, pid: %d, state: %ld, parent_pid: %d, first_child_pid: %d,      next_sibling_pid: %d, uid: %ld, deep: %d\n", b->comm, b->pid, b->state,
	b->parent_pid, b->first_child_pid, b->next_sibling_pid, b->uid, b->deep);
}

void dfs(struct task_struct *start, int deep)
{
    struct task_struct *curr;
    struct list_head *tl; 

    // printk("dfs!!\n");
    if (start == NULL) return;
    // copy from task_struct to tasks
    copyTask(start, &tasks[taskNum], deep);
    
    // ergodic its children
    if (!list_empty(&start->children))
        list_for_each(tl, &start->children)
        {
            curr = list_entry(tl, struct task_struct, sibling);
            dfs(curr, deep + 1);
        }
    return;
}

static int (*oldcall)(void);
static int ptree(struct prinfo *buf, int *nr)
{
    int i = 0;
    printk("begin to ptree!!\n");
    read_lock(&tasklist_lock);
    // begin to dfs from init_task
    dfs(&init_task, 0);
    read_unlock(&tasklist_lock);
    printk("ptree complete!!\n");
    
    *nr = taskNum;
    // copy from tasks[] to buf[]
    for (i = 0; i < taskNum; ++i)
    {
	strcpy(buf[i].comm, tasks[i].comm);
        buf[i].pid = tasks[i].pid;
        buf[i].state = tasks[i].state;
	buf[i].parent_pid = tasks[i].parent_pid;
	buf[i].first_child_pid = tasks[i].first_child_pid;
	buf[i].next_sibling_pid = tasks[i].next_sibling_pid;
	buf[i].uid = tasks[i].uid;
	buf[i].deep = tasks[i].deep;
    }
    return 0;
}

static int addsyscall_init(void)
{
    long *syscall = (long*)0xc000d8c4;
    oldcall = (int(*)(void))(syscall[__NR_ptree]);
    syscall[__NR_ptree] = (unsigned long)ptree;
    printk(KERN_INFO "module load!\n");
    return 0;
}
static void addsyscall_exit(void)
{
    long *syscall = (long*)0xc000d8c4;
    syscall[__NR_ptree] = (unsigned long)oldcall;
    printk(KERN_INFO "module exit!\n");
}

module_init(addsyscall_init);
module_exit(addsyscall_exit);

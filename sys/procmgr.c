//
// Created by robin manhas on 10/28/17.
//

#include <sys/procmgr.h>
#include <sys/kmalloc.h>
#include <sys/kprintf.h>
#include <sys/gdt.h>
#include <sys/vmm.h>
#include <sys/pmm.h>
#include <sys/idt.h>
#include <sys/kstring.h>
#include <sys/util.h>
#include <sys/mm.h>
#include <sys/terminal.h>



#define MAX_PROCESS 100
uint16_t processID = 0; // to keep track of the allocated process ID's to task struct
extern uint64_t kernel_rsp;
extern task_struct *kernel_idle_task; // this store the idle task struct. this task is run when no other active task are available.
task_struct* gReadyList = NULL;
task_struct* gBlockedList = NULL;
task_struct* gZombieList = NULL;
task_struct* gSleepList = NULL;
task_struct* waitList= NULL;

task_struct *currentTask=NULL, *prevTask=NULL;

task_struct* tasks_list[MAX_PROCESS];

task_struct* getCurrentTask(){
    return currentTask;
}

/* init function */
void runner(){
    while(1) {
        //kprintf("inside idle\n");
        schedule();
        __asm__ __volatile__ ("hlt");
    }
}

void userFunc(){
//    uint64_t ret =0;
//    uint64_t syscall = 1;
//    uint64_t arg3 = 5;
//    //__asm__ __volatile__ ("movq %1,%%rax;syscall" : "=r" (ret) : "0" (syscall):"memory");
//    char buff[]="hello";
//    uint64_t arg2=(uint64_t )buff;
//    uint64_t arg1 = 1;
//    __asm__ __volatile__("movq %1,%%rax;movq %2,%%rdi; movq %3,%%rsi; movq %4,%%rdx;syscall" : "=r" (ret):"0"(syscall), "g"(arg1), "g"(arg2) ,"g"(arg3) :"memory" );
//    //schedule();
//    while(1);
}

void initialiseUserProcess(task_struct *user_task){
    uint64_t userbase = VIRBASE;
    user_task->type = TASK_USER;
    user_task->no_of_children = 0;
    user_task->next = NULL;
    user_task->nextChild = NULL;
    user_task->cr3 = (uint64_t)kmalloc();

    user_task->fd[0]=create_terminal_IN();
    FD* filedec = create_terminal_OUT();
    user_task->fd[1]= filedec;
    user_task->fd[2]= filedec;

    uint64_t *userPtr,*kernPtr;
    userPtr = (uint64_t*)user_task->cr3;
    userPtr[PML4_REC_SLOT] = returnPhyAdd(user_task->cr3,KERNBASE_OFFSET,1);
    userPtr[PML4_REC_SLOT] |= (PTE_U_W_P);

//    map_virt_phys_addr(userbase,returnPhyAdd(user_task->cr3,KERNBASE_OFFSET,1),PTE_U_W_P);
//    map_user_virt_phys_addr(userbase,returnPhyAdd(user_task->cr3,KERNBASE_OFFSET,1),&userPtr,1);
//    userbase+=0x1000;



    // map stack
//    map_virt_phys_addr(userbase,returnPhyAdd((uint64_t)user_task->stack,KERNBASE_OFFSET,1),PTE_U_W_P);
//    map_user_virt_phys_addr(userbase,returnPhyAdd((uint64_t)user_task->stack,KERNBASE_OFFSET,1),&userPtr,1);
//    user_task->stack = (uint64_t*)userbase;
//    userbase+=0x1000;


    //kprintf("kernel start add %x\n",user_task->rsp);

    uint64_t userPage = (uint64_t)kmalloc();
    map_virt_phys_addr(userbase,returnPhyAdd(userPage,KERNBASE_OFFSET,1),PTE_U_W_P);
    map_user_virt_phys_addr(userbase,returnPhyAdd(userPage,KERNBASE_OFFSET,1),&userPtr,1);
    userbase+=0x1000;

    user_task->mm = (mm_struct*)userPage;
    user_task->mm->v_addr_pointer = userbase;
    user_task->mm->vma_cache = NULL;
	user_task->curr_dir = tarfs[0];
    // map kernel
    kernPtr = getKernelPML4();
    userPtr[511] = kernPtr[511];
    userPtr[511] |= (PTE_U_W_P);
}
void createUserProcess(task_struct *user_task){
    user_task->state = TASK_STATE_RUNNING;
//    user_task->cr3 = (uint64_t)kmalloc();
    user_task->stack = kmalloc();

    user_task->rsp = (uint64_t)&user_task->stack[510];
    initialiseUserProcess(user_task);

    //Need to check why we need this
    uint64_t curr_rsp;
    __asm__ __volatile__ ("movq %%rsp, %0;":"=r"(curr_rsp));
    curr_rsp = (curr_rsp>>12)<<12;
    kmemcpy(user_task->stack, (uint64_t *)curr_rsp, PAGE_SIZE);
    user_task->kernInitRSP = (uint64_t)&user_task->stack[510];

//    uint64_t userPage = (uint64_t)kmalloc();
//    uint64_t kernPage = (((uint64_t)&userFunc) & ADDRESS_SCHEME);
//    kmemcpy((void*)userPage,(void*)kernPage ,PAGE_SIZE);

    addTaskToReady(user_task,0);
}
void func1()
{
//    kprintf("Thread 1: Entry, task ID: %d\n",current->pid);
//    schedule();
//    kprintf("Thread 1: Returning from switch first time\n");
//    schedule();
//    kprintf("Thread 1: Returning from switch second time\n");
//    schedule();
//    kprintf("Thread 1: Returning from switch third time\n");
//    init_idt();
//    init_irq();
//    kprintf("Thread 1: init IDT and IRQ success\n");
    //createUserProcess();

//    init_timer();
//    init_keyboard();
//    __asm__ ("sti");
    kprintf("got fucn1\n");
    initialiseUserProcess(getCurrentTask());
    char * argv[]={"/bin/sbush","/temp", NULL};
    char * envp[]={"PATH=/bin:", "HOME=/root", "USER=root", NULL};
    load_elf_binary_by_name(getCurrentTask(),"bin/sbush",argv,envp);
    switch_to_user_mode(NULL,getCurrentTask());
    while(1);

}


void func2()
{
    kprintf("Thread 2: Entry, task ID: %d\n",currentTask->pid);
    schedule();
    kprintf("Thread 2: Returning from switch first time\n");
    schedule();
    kprintf("Thread 2: Returning from switch second time\n");
    schedule();
    kprintf("Thread 2: Returning from switch third time\n");
    //schedule();
    while(1);

}

void addTaskToReady(task_struct *readyTask, uint8_t addToFront){
    if(readyTask == NULL){
        kprintf("Error: invalid task in add to ready, returning\n");
        return;
    }

    readyTask->next = NULL;
    if(gReadyList == NULL)
    {
        gReadyList = readyTask;
    }
    else
    {

        if(addToFront){

            // add to front {gReadyList NULL checked before, here it has a value}
            readyTask->next = gReadyList;
            gReadyList = readyTask;
            return;
        }
        //RM: add to end of ready list
        task_struct *iter = gReadyList;
        if(iter == readyTask){ // checks being added as a process got pushed to list multiple times
            //kprintf("Error: ready task already exists, returning\n");
            return;
        }
        while(iter->next != NULL){
            if(iter == readyTask){
                //kprintf("Error: ready task already exists, returning\n");
                return;
            }
            iter = iter->next;
        }

        iter->next = readyTask;
    }
}

void addTaskToBlocked(task_struct *blockedTask){
    if(blockedTask == NULL){
        kprintf("Error: invalid task in add to blocked, returning\n");
        return;
    }

    blockedTask->next = NULL;
    if(gBlockedList == NULL)
    {
        gBlockedList = blockedTask;
    }
    else
    {
        //RM: add to end of blocked list
        task_struct *iter = gBlockedList;
        while(iter->next != NULL)
            iter = iter->next;

        iter->next = blockedTask;
    }
}

void addTaskToZombie(task_struct *zombieTask){
    if(zombieTask == NULL){
        kprintf("Error: invalid task in add to zombie, returning\n");
        return;
    }

    zombieTask->next = NULL;
    if(gZombieList == NULL)
    {
        gZombieList = zombieTask;
    }
    else
    {
        //RM: add to end of zombie list
        task_struct *iter = gZombieList;
        while(iter->next != NULL)
            iter = iter->next;

        iter->next = zombieTask;
    }
}
void addTaskToSleep(task_struct *sleepTask){
    if(sleepTask == NULL){
        kprintf("Error: invalid task in add to sleep, returning\n");
        return;
    }

    sleepTask->next = NULL;
    if(gSleepList == NULL)
    {
        gSleepList = sleepTask;
    }
    else
    {
        //RM: add to end of sleep list
        task_struct *iter = gSleepList;
        while(iter->next != NULL)
            iter = iter->next;

        iter->next = sleepTask;
    }
}

void addTaskToWait(task_struct *task){
    if(task == NULL){
        kprintf("Error: invalid task in add to wait, returning\n");
        return;
    }

    task->next = NULL;
    if(waitList == NULL) {
        waitList = task;
    }
    else {
        task_struct *iter = waitList;
        while(iter->next != NULL)
            iter = iter->next;

        iter->next = task;
    }
}

void switch_to(task_struct *current, task_struct *next)
{

    __asm__ __volatile__("pushq %rax");
    __asm__ __volatile__("pushq %rbx");
    __asm__ __volatile__("pushq %rcx");
    __asm__ __volatile__("pushq %rdx");
    __asm__ __volatile__("pushq %rdi");
    __asm__ __volatile__("pushq %rsi");
    __asm__ __volatile__("pushq %rbp");
    __asm__ __volatile__("pushq %r8");
    __asm__ __volatile__("pushq %r9");
    __asm__ __volatile__("pushq %r10");
    __asm__ __volatile__("pushq %r11");
    __asm__ __volatile__("pushq %r12");

    __asm__ __volatile__("movq %%rsp, %0":"=r"(current->rsp));
    __asm__ __volatile__("movq %0, %%rsp":: "r"(next->rsp));

    if(next->init == 1){
        next->init = 0;
    }
    else{
        __asm__ __volatile__("popq %r12");
        __asm__ __volatile__("popq %r11");
        __asm__ __volatile__("popq %r10");
        __asm__ __volatile__("popq %r9");
        __asm__ __volatile__("popq %r8");
        __asm__ __volatile__("popq %rbp");
        __asm__ __volatile__("popq %rsi");
        __asm__ __volatile__("popq %rdi");
        __asm__ __volatile__("popq %rdx");
        __asm__ __volatile__("popq %rcx");
        __asm__ __volatile__("popq %rbx");
        __asm__ __volatile__("popq %rax");
    }
}

void schedule(){
    if(currentTask != NULL /*gReadyList != NULL*/) {
        prevTask = currentTask;
        currentTask = gReadyList;
        if (currentTask == NULL)
            currentTask = kernel_idle_task; // TODO: currently does not switch properly
        else
            gReadyList = gReadyList->next;

        // add prevTask task switched to end of ready list
        switch (prevTask->state) {
            case TASK_STATE_RUNNING: {
                addTaskToReady(prevTask,0);
                break;
            }
            case TASK_STATE_BLOCKED: {
                addTaskToBlocked(prevTask);
                break;
            }
            case TASK_STATE_ZOMBIE: {
                addTaskToZombie(prevTask);
                break;
            }
            case TASK_STATE_SLEEP: {
                addTaskToSleep(prevTask);
                break;
            }
            case TASK_STATE_IDLE:
            case TASK_STATE_KILLED: {
                // don't add idle task or killed task to any queue.
                break;
            }
            case TASK_STATE_WAIT:{
                addTaskToWait(prevTask);
                break;
            }
            default: {
                kprintf("unhandled task state in scheduler\n");
                break;
            }
        }

//        if(current->type == TASK_KERNEL)
//            switch_to(prev,current);
//        else if(current->type == TASK_USER)
//            switch_to_user_mode(prev,current);


        if (prevTask != currentTask) {
            //kprintf("switching to user task:changing cr3 \n");
            setCR3((uint64_t *) currentTask->cr3);
        }

        if(currentTask->type == TASK_USER)
        {
            set_tss_rsp((uint64_t *) (ALIGN_UP(currentTask->rsp, PAGE_SIZE) - 16));
            kernel_rsp = ALIGN_UP(currentTask->rsp, PAGE_SIZE) - 16;
        }
        switch_to(prevTask, currentTask);

    }

}

uint16_t getFreePID()
{
    return processID++; // 0 will be our initial kernel task
}

task_struct* getFreeTask()
{
    task_struct *task = (task_struct*)kmalloc();
    task->pid = getFreePID();
    tasks_list[task->pid] = task;
    return task;
}

void createKernelInitProcess(task_struct *ktask, task_struct *startFuncTask){

    // this task struct is for main thread, just that instead of initial stack we assign a stack to this too
    startFuncTask = kmalloc();
    startFuncTask->init = 1;
    startFuncTask->pid = -1;
    startFuncTask->stack = kmalloc();
    startFuncTask->rsp = (uint64_t)&startFuncTask->stack[510];
    startFuncTask->cr3 = (uint64_t )getKernelPML4();
    startFuncTask->type = TASK_KERNEL;
    startFuncTask->state = TASK_STATE_IDLE;
    startFuncTask->no_of_children = 0;
    startFuncTask->next = NULL;
    startFuncTask->nextChild = NULL;

    // this is our kernel init task, it runs first of all other task, and keeps running when no runnable task exists. it waits on an interrupt
    ktask->stack = kmalloc();
    ktask->init = 1;
    ktask->stack[510] = (uint64_t)runner;
    ktask->rsp = (uint64_t)&ktask->stack[510];
    ktask->cr3 = (uint64_t )getKernelPML4();
    ktask->user_rip = (uint64_t) &runner;
    ktask->type = TASK_KERNEL;
    ktask->state = TASK_STATE_RUNNING;
    ktask->no_of_children = 0;
    ktask->next = NULL;
    ktask->nextChild = NULL;
    currentTask = startFuncTask;
    addTaskToReady(ktask,0);
}

void createKernelTask(task_struct *task, void (*func)(void)){

    task->stack = kmalloc();
    task->init = 1;
    task->stack[510] = (uint64_t)func;
    task->rsp = (uint64_t)&task->stack[510];
    task->user_rip = (uint64_t)func;
    task->cr3 = (uint64_t )getKernelPML4();
    task->no_of_children = 0;
    task->next = NULL;
    task->nextChild = NULL;
    task->type = TASK_KERNEL;
    task->state = TASK_STATE_RUNNING;
    addTaskToReady(task,0);
}



void switch_to_user_mode(task_struct *oldTask, task_struct *user_task)
{
    __asm__ volatile("cli");
//    __asm__ __volatile__("pushq %rax");
//    __asm__ __volatile__("pushq %rbx");
//    __asm__ __volatile__("pushq %rcx");
//    __asm__ __volatile__("pushq %rdx");
//    __asm__ __volatile__("pushq %rdi");
//    __asm__ __volatile__("pushq %rsi");
//    __asm__ __volatile__("pushq %rbp");
//    __asm__ __volatile__("pushq %r8");
//    __asm__ __volatile__("pushq %r9");
//    __asm__ __volatile__("pushq %r10");
//    __asm__ __volatile__("pushq %r11");
//    __asm__ __volatile__("pushq %r12");
//
//    __asm__ __volatile__("movq %%rsp, %0":"=r"(oldTask->rsp));

    uint64_t ret;
    __asm__ __volatile__ ("movq %%rsp, %0;":"=r"(ret));

    set_tss_rsp((uint64_t *)ALIGN_UP(ret, PAGE_SIZE) - 16);
    kernel_rsp = ALIGN_UP(ret, PAGE_SIZE) - 16;
    //set_tss_rsp((void*)(user_task->kernInitRSP - 16));
    //kernel_rsp = (user_task->kernInitRSP - 16);

    setCR3((uint64_t*)user_task->cr3);
    __asm__ volatile("mov $0x23, %%ax"::);
    __asm__ volatile("mov %%ax, %%ds"::);
    __asm__ volatile("mov %%ax, %%es"::);
    __asm__ volatile("mov %%ax, %%fs"::);
    __asm__ volatile("mov %%ax, %%gs"::);

    __asm__ volatile("movq %0, %%rax"::"r"(user_task->user_rsp));
    __asm__ volatile("pushq $0x23");
    __asm__ volatile("pushq %rax");
    __asm__ volatile("pushfq");
    __asm__ volatile("popq %rax");
    __asm__ volatile("or $0x200, %%rax;":::);
    __asm__ volatile("pushq %rax");
    __asm__ volatile("pushq $0x2B");
    __asm__ volatile("pushq %0"::"r"(user_task->user_rip));
    __asm__ volatile("iretq");
}



// use next child link instead of next
void removeChildFromParent(task_struct *parent, task_struct*child){
    task_struct* ptr = parent->child_list;
    task_struct* prevptr = NULL;
    while(ptr){
        if(ptr->pid == child->pid)
        {
            if(prevptr == NULL)
                parent->child_list = ptr->nextChild;
            else
                prevptr->nextChild = ptr->nextChild;
            --parent->no_of_children;
            return;
        }
        prevptr = ptr;
        ptr = ptr->nextChild;
    }
}

// add all children of given parent task to init list
void addChildrenToInitTask(task_struct *parentTask){
    if(parentTask == NULL){
        return;
    }

    task_struct* taskChildPtr = parentTask->child_list;

    while(taskChildPtr){
        if(kernel_idle_task->child_list == NULL)
            kernel_idle_task->child_list = taskChildPtr;
        else {
            taskChildPtr->nextChild = kernel_idle_task->child_list;
            kernel_idle_task->child_list = taskChildPtr;
        }
        ++kernel_idle_task->no_of_children;
        taskChildPtr = taskChildPtr->nextChild;
    }
}

void removeTaskFromRunList(task_struct *task){

    if(task == NULL)
        return;

    // remove from active
    task_struct *readyListPtr = gReadyList;
    task_struct* prevptr = NULL;
    while(readyListPtr){
        if(readyListPtr->pid == task->pid)
        {
            if(prevptr == NULL)
                gReadyList = readyListPtr->next;
            else
                prevptr->next = readyListPtr->next;
            return;
        }
        prevptr = readyListPtr;
        readyListPtr = readyListPtr->next;
    }

    // remove from blocked
    task_struct *blockedListPtr = gBlockedList;
    prevptr = NULL;
    while(blockedListPtr){
        if(blockedListPtr->pid == task->pid)
        {
            if(prevptr == NULL)
                gBlockedList = blockedListPtr->next;
            else
                prevptr->next = blockedListPtr->next;
            return;
        }
        prevptr = blockedListPtr;
        blockedListPtr = blockedListPtr->next;
    }
}

// this function is used to remove a task from ready queue and blocked queue, and add it to zombie queue
void moveTaskToZombie(task_struct *task){
    if(task == NULL)
        return;

    removeTaskFromRunList(task);
    // add to zombie
    task->state = TASK_STATE_ZOMBIE;
    addTaskToZombie(task);
}

void destroy_task(task_struct *task){
    // freeing pages allocated to child
    free_all_vma_pages(task);

    // traverse parent page tables and update permissions
    if(task->parent != NULL)
        updateParentCOWInfo(task->parent);

    // if children, add them to init task
    if(task->child_list){
        addChildrenToInitTask(task);
    }

    // remove task from parent
    if(task->parent){
        removeChildFromParent(task->parent,task);

        if(task->parent->state == TASK_STATE_WAIT) {
            task->parent->state = TASK_STATE_RUNNING;
            addTaskToReady(task->parent,0);
        }
    }
}

void killTask(task_struct *task){

    if(task == NULL || task->state == TASK_STATE_ZOMBIE)
        return;

    // user process can't kill kernel task
    if(task->type == TASK_KERNEL && currentTask->type != TASK_KERNEL) {
        return;
    }

    kprintf("before kill task:");printPageCountStats(); // printed in single line on terminal
    destroy_task(task);
    kprintf("after kill task:");printPageCountStats();
    // make currentTask active as zombie, add to zombie taken care in schedule
    if(task == currentTask){
        task->state = TASK_STATE_KILLED;
        schedule();
    }

}

void removeTaskFromBlocked(task_struct* task){
    if(task == NULL)
        return;

    task_struct* prevptr = NULL;
    task_struct *blockedListPtr = gBlockedList;

    while(blockedListPtr){
        if(blockedListPtr->pid == task->pid) {
            if(prevptr == NULL)
                gBlockedList = blockedListPtr->next;
            else
                prevptr->next = blockedListPtr->next;
            return;
        }
        prevptr = blockedListPtr;
        blockedListPtr = blockedListPtr->next;
    }

}
//run only when 1 millisec has elapsed
void reduceSleepTime(){
    task_struct *sleepListPtr = gSleepList;
    task_struct* prevptr = NULL;
    while(sleepListPtr){
        sleepListPtr->sleepTime--;
        if(sleepListPtr->sleepTime == 0) {
            if(prevptr==NULL){
                gSleepList = sleepListPtr->next;
                addTaskToReady(sleepListPtr,0);
                sleepListPtr = gSleepList;
            }else{
                prevptr->next = sleepListPtr->next;
                addTaskToReady(sleepListPtr,0);
                sleepListPtr = prevptr->next;
            }
            continue;
        }
        prevptr = sleepListPtr;
        sleepListPtr = sleepListPtr->next;
    }
}


void removeTaskFromSleep(task_struct* task){
    if(task == NULL)
        return;

    task_struct* prevptr = NULL;
    task_struct *sleepListPtr = gSleepList;

    while(sleepListPtr){
        if(sleepListPtr->pid == task->pid) {
            if(prevptr == NULL)
                gSleepList = sleepListPtr->next;
            else
                prevptr->next = sleepListPtr->next;
            return;
        }
        prevptr = sleepListPtr;
        sleepListPtr = sleepListPtr->next;
    }

}

uint8_t  get_ps(char *buf, uint8_t length) {
    if(buf == NULL || length == 0)
        return -1;

    memset(buf,0,length);
    task_struct *task;
    uint8_t size_written = 0;
    uint8_t name_len=0;
    uint8_t id_len = 0;
    char s_id[10];

    //starting from 1, as 0 is always kernel process
    for(int i =1; i <MAX_PROCESS;i++) {
        task = tasks_list[i];
        if(task == NULL)
            break;
        if(task->type == TASK_USER && task->state != TASK_STATE_KILLED){
            name_len = kstrlen(task->name);
            id_len = ktostring(s_id,task->pid);
            if(size_written+ name_len +id_len +2 > length)
                return size_written;
            kstrcat(buf,s_id);
            kstrcat(buf,":");
            kstrcat(buf,task->name);
            kstrcat(buf,":");

            size_written += id_len+name_len+2;

        }

    }
    return size_written;
}

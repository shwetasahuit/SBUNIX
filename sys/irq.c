// References: http://www.osdever.net/bkerndev/Docs/isrs.htm

#include <sys/defs.h>
#include <sys/idt.h>
#include <sys/kprintf.h>
#include <sys/util.h>
#include <sys/vmm.h>
#include <sys/mm.h>
#include <sys/procmgr.h>
#include <sys/pmm.h>
#include <sys/kstring.h>


#define PRES 		0x80
#define DPL_0 		0x00
#define DPL_1 		0x20
#define DPL_2 		0x40
#define DPL_3 		0x60
#define S 		    0x00
#define INTR_GATE 	0x0E

void _irq0();
void _irq1();
void isr0();
void isr14();
void syscall();
void ISR_HANDLER_0();
void ISR_HANDLER_1();

void ISR_HANDLER_2();

void ISR_HANDLER_3();

void ISR_HANDLER_4();

void ISR_HANDLER_5();
void ISR_HANDLER_6();

void ISR_HANDLER_7();

void ISR_HANDLER_8();

void ISR_HANDLER_9();
void ISR_HANDLER_10();

void ISR_HANDLER_11();

void ISR_HANDLER_12();
void ISR_HANDLER_13();
void ISR_HANDLER_16();
void ISR_HANDLER_17();

void ISR_HANDLER_18();

void ISR_HANDLER_19();
void ISR_HANDLER_20();
void ISR_HANDLER_21();
void ISR_HANDLER_22();
void ISR_HANDLER_23();

void ISR_HANDLER_24();

void ISR_HANDLER_25();
void ISR_HANDLER_26();

extern void syscall_handler();
void handle_page_fault(struct regs* reg);

void *irqs[16] =
        {
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0
        };

void irq_install_handler(int irq, void (*handler)())
{
    irqs[irq] = handler;

}


void irq_uninstall_handler(int irq)
{
    irqs[irq] = 0;
}




void irq_remap()
{

    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);


}


void init_irq()
{
    unsigned char ring0Attr = PRES | DPL_0 | S | INTR_GATE;
    unsigned char ring3Attr = PRES | DPL_3 | S | INTR_GATE;
    irq_remap();
    idt_set_gate(32, (long)_irq0, 0x08, ring0Attr);
    idt_set_gate(33, (long)_irq1, 0x08, ring0Attr);
    idt_set_gate(0, (long)ISR_HANDLER_0, 0x08, ring0Attr);
    idt_set_gate(1, (long)ISR_HANDLER_1, 0x08, ring0Attr);
    idt_set_gate(2, (long)ISR_HANDLER_2, 0x08, ring0Attr);
    idt_set_gate(3, (long)ISR_HANDLER_3, 0x08, ring0Attr);
    idt_set_gate(4, (long)ISR_HANDLER_4, 0x08, ring0Attr);
    idt_set_gate(5, (long)ISR_HANDLER_5, 0x08, ring0Attr);
    idt_set_gate(6, (long)ISR_HANDLER_6, 0x08, ring0Attr);
    idt_set_gate(7, (long)ISR_HANDLER_7, 0x08, ring0Attr);
    idt_set_gate(8, (long)ISR_HANDLER_8, 0x08, ring0Attr);
    idt_set_gate(9, (long)ISR_HANDLER_9, 0x08, ring0Attr);
    idt_set_gate(10, (long)ISR_HANDLER_10, 0x08, ring0Attr);
    idt_set_gate(11, (long)ISR_HANDLER_11, 0x08, ring0Attr);
    idt_set_gate(12, (long)ISR_HANDLER_12, 0x08, ring0Attr);
    idt_set_gate(13, (long)ISR_HANDLER_13, 0x08, ring0Attr);
    idt_set_gate(14, (long)isr14, 0x08, ring0Attr);
    idt_set_gate(15, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(16, (long)ISR_HANDLER_16, 0x08, ring0Attr);
    idt_set_gate(17, (long)ISR_HANDLER_17, 0x08, ring0Attr);
    idt_set_gate(18, (long)ISR_HANDLER_18, 0x08, ring0Attr);
    idt_set_gate(19, (long)ISR_HANDLER_19, 0x08, ring0Attr);


    idt_set_gate(20, (long)ISR_HANDLER_20, 0x08, ring0Attr);
    idt_set_gate(21, (long)ISR_HANDLER_21, 0x08, ring0Attr);
    idt_set_gate(22, (long)ISR_HANDLER_22, 0x08, ring0Attr);
    idt_set_gate(23, (long)ISR_HANDLER_23, 0x08, ring0Attr);
    idt_set_gate(24, (long)ISR_HANDLER_24, 0x08, ring0Attr);
    idt_set_gate(25, (long)ISR_HANDLER_25, 0x08, ring0Attr);
    idt_set_gate(26, (long)ISR_HANDLER_26, 0x08, ring0Attr);


    idt_set_gate(26, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(28, (long)isr0, 0x08, ring0Attr);


    idt_set_gate(27, (long)isr0, 0x08, ring0Attr);


    idt_set_gate(29, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(30, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(31, (long)isr0, 0x08, ring0Attr);
    idt_set_gate(128, (long)syscall, 0x08, ring3Attr);

    //outb(0x21,0xFD);   // to disable irq lines : 0xFD=11111101 enable only keyboard(0 to enable)
}


void _irq_handler(struct regs* reg)
{
    if(reg->int_no==128){
        kprintf("syscall interrupt received, %d\n",reg->rax);
    }
    else if(reg->int_no<32 && reg->int_no!=14){
        kprintf("got interrupt no %d\n",reg->int_no);
        kprintf("got error no %d\n",reg->err_code);
    }else if(reg->int_no==14) {
        handle_page_fault(reg);
    }
    else {
        long num = (reg->int_no) - 32;

        void (*handler)();

        handler = irqs[num];
        if (handler) {
            handler();
        }
    }


//    if (num >= 40)
//    {
//        outb(0xA0, 0x20);
//    }


    outb(0x20, 0x20);//for irq lines

}

void handle_page_fault(struct regs* reg){
    task_struct* current_task = getCurrentTask();
    kprintf("inside page fault\n");
    uint64_t faulty_addr;
    __asm__ __volatile__ ("movq %%cr2, %0;" : "=r"(faulty_addr));

    uint64_t err_code = reg->err_code;
    uint64_t new_page,new_vir;
    uint64_t * pml4_pointer = (uint64_t*)current_task->cr3;


    //err_code 0bit-> if set; then page is present
    if(err_code & 0x1){
        //get physical address
        uint64_t* phy_addr = (uint64_t*)returnPhyAdd(faulty_addr,KERNBASE_OFFSET,1);

        //not writable and cow set
        if(!(*phy_addr & PTE_W) && (*phy_addr & PTE_COW) ){
            //check if shared
            Page* page = get_page(*phy_addr);
            if(page != NULL && page->sRefCount >1){
                new_page = allocatePage();
                new_vir = current_task->mm->v_addr_pointer;
                current_task->mm->v_addr_pointer += 0x1000;
                map_user_virt_phys_addr(new_vir,new_page,&pml4_pointer);

                //copy contents from old page to new page
                memcpy((uint64_t *)new_vir,(uint64_t *)faulty_addr,PAGE_SIZE);

                *phy_addr = new_page|PTE_U_W_P;

            }else{
                //unset cow and set write bit
                *phy_addr = *phy_addr | PTE_W;
                *phy_addr = *phy_addr &(~PTE_COW);
            }
        }else{
            kprintf("reason for page fault is unknown \n");
        }

    }else{
        //page not present
        vm_area_struct* vma = find_vma(current_task->mm,faulty_addr);
        if(vma == NULL){
            kprintf("vma doesnt exist, reason for page fault is unknown");
            return;
        }
        allocate_pages_to_vma(vma,&pml4_pointer);

    }
    return;


}






#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/workqueue.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/unistd.h>

void **sys_call_table;

asmlinkage int (*original_call) (const char __user*, int, umode_t);

asmlinkage long our_sys_open(const char __user * file, int flags, umode_t mode)
{
   printk("File was opened : %s\n", file);
   return original_call(file, flags, mode);
}

void
disable_wp(void)
{
    unsigned long cr0;

    preempt_disable();
    cr0 = read_cr0();
    clear_bit(X86_CR0_WP_BIT, &cr0);
    write_cr0(cr0);
    preempt_enable();

    return;
}


void
enable_wp(void)
{
    unsigned long cr0;

    preempt_disable();
    cr0 = read_cr0();
    set_bit(X86_CR0_WP_BIT, &cr0);
    write_cr0(cr0);
    preempt_enable();

    return;
}


static int myinit_module(void);
static void mycleanup_module(void);

static int myinit_module(void)
{
    unsigned long long virt_addr;
    disable_wp();
    // sys_call_table address in System.map
    sys_call_table = (void*)0xffffffff81801400;
    original_call = (void*)sys_call_table[__NR_open];

   
    sys_call_table[__NR_open] = (unsigned long*)our_sys_open;
//ffffffffa01dc000  
//ffffffff81801410
//1801400
 
    printk("__NR_open = %llx\n", __NR_open);
    printk("sys_call_table[__NR_open]'s address = %llx\n", &(sys_call_table[__NR_open]));
    
    virt_addr = &(sys_call_table[__NR_open]);
    printk("sys_call_table[__NR_open]'s phys address = %llx\n", virt_to_phys(&virt_addr));
    
    enable_wp();
    return 0;
}

static void mycleanup_module(void)
{
   disable_wp();
   // Restore the original call
   sys_call_table[__NR_open] = original_call;

   enable_wp();
   return;
}

module_init(myinit_module);
module_exit(mycleanup_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nilushan Silva");
MODULE_DESCRIPTION("task_struct offset Finder");

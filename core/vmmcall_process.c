#include "vmmcall.h"
#include "initfunc.h"
#include "printf.h"
#include "panic.h"
#include "current.h"
#include "process.h"

struct msgdsc_data {
        int pid;
        int gen;
        int dsc;
        void *func;
};

struct process_data {
        bool valid;
        phys_t mm_phys;
        int gen;
        int running;
        struct msgdsc_data msgdsc[NUM_OF_MSGDSC];
        bool exitflag;
        bool restrict;
        int stacksize;
};


extern struct process_data process[NUM_OF_PID];

static void
listprocess (void)
{
  //ulong rbx;
 // printf("test vmmcall_test!\n");
 // current->vmctl.read_general_reg (GENERAL_REG_RBX, &rbx);
  //banaddr = rbx;
 // printf("banaddr is: %ul\n", rbx);
  
  //  panic(__func__);

	int i;
	printf("The Process List is:\n");
	for(i=0; i<NUM_OF_PID; i++) {
		struct process_data pd = process[i];
		printf("PID: %d\n", pd.msgdsc[0].pid);
	}


}

static void
vmmcall_process_init (void)
{
	vmmcall_register ("vmmcall_process", listprocess);
}

INITFUNC ("vmmcal0", vmmcall_process_init);

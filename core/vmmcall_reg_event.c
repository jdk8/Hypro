#include "vmmcall.h"
#include "initfunc.h"
#include "printf.h"
#include "panic.h"
#include "current.h"
#include "vt_ept.h"
#include "string.h"
#include "process.h"
#include "mm.h"
#include "cpu_mmu.h"
#include "sleep.h"
#include "vt_regs.h"
#include "config.h"
#include "tty.h"
#include "limits.h"
 

extern bool reg_event_read = false;
extern bool reg_event_write = false;
extern bool _reg_event_cr0_read= false;
extern bool _reg_event_cr0_write=false;

static void
reg_event_read1 (void){

  reg_event_read=true;
}

static void
reg_event_write1 (void){
  reg_event_write=true;
}


static void
reg_event_cr0_read (void){
  _reg_event_cr0_read=true;
}

static void
reg_event_cr0_write (void){
	 	printf("set cr0 write event.......\n");
  _reg_event_cr0_write=true;
}


static void
vmmcall_reg_event_read_init (void){
    vmmcall_register ("vmmcall_reg_event_read", reg_event_read1);
}


static void
vmmcall_reg_event_cr0_read_init (void){
    vmmcall_register ("vmmcall_reg_event_cr0_read", reg_event_cr0_read);
}


static void
vmmcall_reg_event_write_init (void){
    vmmcall_register ("vmmcall_reg_event_write", reg_event_write1);
}

static void
vmmcall_reg_event_cr0_write_init (void){
    vmmcall_register ("vmmcall_reg_event_cr0_write", reg_event_cr0_write);
}


INITFUNC ("vmmcal0", vmmcall_reg_event_read_init);
INITFUNC ("vmmcal0", vmmcall_reg_event_write_init);
INITFUNC ("vmmcal0", vmmcall_reg_event_cr0_read_init);
INITFUNC ("vmmcal0", vmmcall_reg_event_cr0_write_init);

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
#include "vt_regs.h"
#include "config.h"
#include "tty.h"

struct memdump_data {
  u64 physaddr;
  u64 cr0, cr3, cr4, efer;
  ulong virtaddr;
  int sendlen;
};

static void 
get_control_regs(ulong * cr0, ulong * cr3, ulong * cr4, u64 * efer){
    current->vmctl.read_control_reg (CONTROL_REG_CR0, cr0);
    current->vmctl.read_control_reg (CONTROL_REG_CR3, cr3);
    current->vmctl.read_control_reg (CONTROL_REG_CR4, cr4);
    current->vmctl.read_msr (MSR_IA32_EFER, efer);
}

u64
virt_to_phys(ulong virtaddr){
    struct memdump_data data;
    u64 ent[5];
    int r, levels;
    memset(ent, 0, sizeof(ent));
    memset(&data, 0, sizeof(struct memdump_data));
    data.virtaddr = virtaddr;
    get_control_regs((ulong *)&data.cr0, (ulong *)&data.cr3, (ulong *)&data.cr4, &data.efer);
    r = cpu_mmu_get_pte(data.virtaddr, (ulong)data.cr0, (ulong)data.cr3, (ulong)data.cr4, data.efer, false, false, false, ent, &levels);
    if (r == VMMERR_SUCCESS) { 
        data.physaddr = (ent[0] & PTE_ADDR_MASK64) | ((data.virtaddr) & 0xFFF);
        return data.physaddr;
    }
    return -1;
}

static int
get_virt_val(ulong virtaddr, int nr_bytes, void * value){
    struct memdump_data data;
    u64 ent[5];
    int r, levels;
    memset(ent, 0, sizeof(ent));
    memset(&data, 0, sizeof(struct memdump_data));
    data.virtaddr = virtaddr;
    get_control_regs((ulong *)&data.cr0, (ulong *)&data.cr3, (ulong *)&data.cr4, &data.efer);
    r = cpu_mmu_get_pte(data.virtaddr, (ulong)data.cr0, (ulong)data.cr3, (ulong)data.cr4, data.efer, false, false, false, ent, &levels);
    if (r == VMMERR_SUCCESS) { 
        data.physaddr = (ent[0] & PTE_ADDR_MASK64) | ((data.virtaddr) & 0xFFF);
        if(nr_bytes == 4) {
        read_hphys_l(data.physaddr, value, 0);
        }
        if(nr_bytes == 8) {
       read_hphys_q(data.physaddr, value, 0);
        }
        return 0;
    }
    return -1;
}

extern u64 gp = 0xffffffffffffffff;

static void
memory_event (void){
    // Following codes try to modify the process list's address
    gp = virt_to_phys(0xffffffff81c15480+0x270);
    printf("gp = %llx\n", gp);
    //void read_hphys_l (u64 phys, void *data, u32 attr);
    //void write_hphys_l (u64 phys, u32 data, u32 attr);
    u64 val;
    //read_hphys_q(gp, &val, 0);
    get_virt_val(0xffffffff81c15480+0x270, 8, &val);
    printf("val = %llx\n", val);
    write_hphys_q(gp, 0x0fffffff, 0);
    get_virt_val(0xffffffff81c15480+0x270, 8, &val);
    printf("new val = %llx\n", val);



}

static void
vmmcall_memory_event_init (void){
    vmmcall_register ("vmmcall_memory_event", memory_event);
}

INITFUNC ("vmmcal0", vmmcall_memory_event_init);
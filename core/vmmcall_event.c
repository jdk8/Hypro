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
 

 typedef enum status{
     VMI_SUCCESS = 0,
     VMI_FAILURE = 1
    } status_t;

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

static void memcpy_to_usespace(char *buf, ulong buflen)
{
    ulong rbx, rcx;  /* virtaddr: rbx, len: rcx*/
    int i = 0, ret;

    u64 efer, physaddr;
    ulong cr0, cr3, cr4;
    u64 ent[5];
    int levels;

    current->vmctl.read_general_reg (GENERAL_REG_RBX, &rbx);
    current->vmctl.read_general_reg (GENERAL_REG_RCX, &rcx);
    current->vmctl.read_control_reg (CONTROL_REG_CR0, &cr0);
    current->vmctl.read_control_reg (CONTROL_REG_CR3, &cr3);
    current->vmctl.read_control_reg (CONTROL_REG_CR4, &cr4);
    current->vmctl.read_msr (MSR_IA32_EFER, &efer);

    for (i=0; i < buflen ; i++)
    {
        ret = cpu_mmu_get_pte(rbx+i, cr0, cr3, cr4, efer, true, false, false, ent, &levels);
    if (ret == VMMERR_SUCCESS)
    {
            physaddr = (ent[0] & PTE_ADDR_MASK64) | ((rbx+i) & 0xFFF);
            write_hphys_b(physaddr, *(buf+i), 0);
    }
    else
            break;
    }

    return;
}


static u64 gvirt_addr_arr[100] = {};
static u64 gphys_addr_arr[100] = {};
static u64 gmm_val_arr[100] = {};
static int len = 0;

static void
memory_event (void){
  //print_msr = true;
    u64 next_module, list_head;
    next_module = 0xffffffff81c52ff0;
    list_head = next_module;

    while (1) {
        u64 tmp_next = 0;     
        virt_memcpy(next_module, 8, &tmp_next);
        gvirt_addr_arr[len]=next_module;
        gphys_addr_arr[len]=virt_to_phys(next_module);
        gmm_val_arr[len]=tmp_next;
        len++;
        if (list_head == tmp_next) {
            break;
        }
        next_module = tmp_next;
    }
 
}


static void
memory_event1 (void){
    int i;
    for(i=0;i<len;i++){
        u64 vaddr = gvirt_addr_arr[i];
        u64 val;
        virt_memcpy(vaddr, 8, &val);
        if (val!=gmm_val_arr[i]){
            printf("vaddr: %llx, old_val: %llx, new_val: %llx\n", vaddr, gmm_val_arr[i], val);
            gmm_val_arr[i]=val;
        }
    }
}

static void
vmmcall_memory_event_init (void){
    vmmcall_register ("vmmcall_memory_event", memory_event);
}

static void
vmmcall_memory_event1_init (void){
    vmmcall_register ("vmmcall_memory_event1", memory_event1);
}


INITFUNC ("vmmcal0", vmmcall_memory_event_init);
INITFUNC ("vmmcal0", vmmcall_memory_event1_init);
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

typedef enum status{
  VMI_SUCCESS = 0,
  VMI_FAILURE = 1
} status_t;

u64 htoi(char * str)
{
    u64 res=0;
    int i;
    for(i=2;str[i]!='\0';i++) {
        int t = str[i]>='a'?str[i]-'a'+10:str[i]-'0';
        res = res*16+t;
    }
    return res;
}

u64 atoi(char * str)
{
    u64 res=0;
    int i;
    for( i=0;str[i]!='\0';i++) {
        res=res*10+ str[i]-'0';
    }
    return res;
}

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

u64 vmi_translate_ksym2v(char * ksym)
{
    u64 addr = 0;
    if ( !strcmp(ksym, "init_task") ) {
        addr  = htoi(config.vmi.init_task);
    }
    return addr;
}

int
virt_memcpy(ulong virtaddr, int nr_bytes, void * value){
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

char *
read_str_va(u64 vaddr)
{
    char * ret_val = NULL;
    char * ret_val1 = NULL;
    ret_val = (char *)alloc(24 * sizeof(char));
    ret_val1 = (char *)alloc(24 * sizeof(char));
    u64 x= 0x0ULL, y=0x0ULL, z=0x0ULL;
    struct memdump_data data;
    u64 ent[5];
    int i, r, levels;
    memset(ent, 0, sizeof(ent));
    memset(&data, 0, sizeof(struct memdump_data));
    data.virtaddr = vaddr;
    get_control_regs((ulong *)&data.cr0, (ulong *)&data.cr3, (ulong *)&data.cr4, &data.efer);
    r = cpu_mmu_get_pte(data.virtaddr, (ulong)data.cr0, (ulong)data.cr3, (ulong)data.cr4, data.efer, false, false, false, ent, &levels);
    int c=0;
    if (r == VMMERR_SUCCESS) {
        data.physaddr = (ent[0] & PTE_ADDR_MASK64) | ((data.virtaddr) & 0xFFF);
        read_hphys_q(data.physaddr, &x, 0);
        read_hphys_q(data.physaddr+8, &y, 0);
        read_hphys_q(data.physaddr+16, &z, 0);
        for(i=0;i<8;i++) {
            int t=x&0xff;
            ret_val[c++] = t;
            x=x>>8;
        }
        for(i=0;i<8;i++) {
            int t=y&0xff;
            ret_val[c++] = t;
            y=y>>8;
        }
        for(i=0;i<8;i++) {
            int t=z&0xff;
            ret_val[c++] = t;
            z=z>>8;
        }
    }
    ret_val[c] = '\0';
    c=0;
    for(i=0;i<24;i++) {
        if (ret_val[c]==' ') {
            break;
        }
        ret_val1[c] = ret_val[c];
        c=c+1;
    }
    ret_val1[c]='\0';

    return ret_val1;
}

static void
listprocess (void){
    u64 current_process;
    unsigned long tasks_offset, pid_offset, name_offset;
    u64 list_head = 0, next_list_entry = 0;
    u32 pid = 0;

    tasks_offset = htoi(config.vmi.linux_tasks);
    pid_offset = htoi(config.vmi.linux_pid);
    name_offset = htoi(config.vmi.linux_name);

    list_head = vmi_translate_ksym2v("init_task") + tasks_offset;

    next_list_entry = list_head;
    do {
        current_process = next_list_entry - tasks_offset;
        virt_memcpy(current_process + pid_offset, 4, &pid);
        printf("[PID:%5d] ",pid);
        char *procname = NULL;
        procname = read_str_va(current_process + name_offset);
        printf("process name: %s", procname);
        printf("  (struct addr:%llx\t", current_process);
        virt_memcpy(next_list_entry, 8, &next_list_entry);
        printf("val is %llx\n", next_list_entry);
        free(procname);
     } while (next_list_entry != list_head);
}

static void dump_memory()
{
    const int MAX_PAGE_NUM = 100;

    int i=0,j=0;
    u64 addr = 0;
    u64 tmp = -1;
    for(i=0; i<MAX_PAGE_NUM; i++) {
        read_hphys_q(addr, &tmp, 0);
        printf("%llx:", addr);
        addr = addr + 8;
        for(j=0;j<8;j++) {
            int t=tmp&0xff;
            if (t>=16) {
                printf(" %x", t);
            } else {
                printf(" 0%x", t);
            }
            tmp=tmp>>8;
        }
        printf("\n");
    }
}

static int
memdump_hpa_to_gva(struct memdump_data * des, char * src){
    int i = 0, r, width = 8;
    u64 ent[5];
    u64 physaddr;
    int levels;
    char str[width];
    memset(&str, 0, sizeof str);
    get_control_regs(&des->cr0, &des->cr3, &des->cr4, &des->efer);

    for (i = 0; i < des->sendlen; i++) {
        r = cpu_mmu_get_pte(des->virtaddr+i, (ulong)des->cr0, (ulong)des->cr3, (ulong)des->cr4,
            des->efer, true, false, false, ent, &levels);
        if (r == VMMERR_SUCCESS) {
            physaddr = (ent[0] & PTE_ADDR_MASK64) | ((des->virtaddr+i) & 0xFFF);
            write_hphys_b(physaddr, src[i], 0);
        } else{
            printf("error: r = %d\n", r);
            break;
        }
    }
    return r;
}


static int
get_vmmlog(){
    ulong rbx, rcx;
    struct memdump_data data;
    current->vmctl.read_general_reg (GENERAL_REG_RBX, &rbx);
    current->vmctl.read_general_reg (GENERAL_REG_RCX, &rcx);

    data.virtaddr = rbx;
    data.sendlen = rcx;

    if (memdump_hpa_to_gva(&data, log) == VMI_FAILURE) {
        printf("error when copy log to string!\n");
        return VMI_FAILURE;
    }

  return VMI_SUCCESS;
}


static int
listmodule(){
    u64 next_module, list_head;
    next_module = 0xffffffff81c52ff0;
    list_head = next_module;
    
    while (1) {
        u64 tmp_next = 0;
        virt_memcpy(next_module, 8, &tmp_next);
     
        if (list_head == tmp_next) {
            break;
        }

        char *modname = NULL;
        modname = read_str_va(next_module + 16);
        printf("kernel module name: %s", modname);
        printf("\t(struct addr:%llx)\n", tmp_next);
        free(modname);
        next_module = tmp_next;
    }

  return VMI_SUCCESS;
}


static void
vmmcall_listprocess_init (void){
    vmmcall_register ("vmmcall_listprocess", listprocess);
}

static void
vmmcall_printlog_init(){
    vmmcall_register ("vmmcall_getvmmlog", get_vmmlog);
}

static void
vmmcall_dump_memory_init(){
    vmmcall_register ("vmmcall_dump_memory", dump_memory);
}

static void
vmmcall_listmodule_init(){
  vmmcall_register ("vmmcall_listmodule", listmodule);
}

INITFUNC ("vmmcal0", vmmcall_listprocess_init);
INITFUNC ("vmmcal0", vmmcall_printlog_init);
INITFUNC ("vmmcal0", vmmcall_dump_memory_init);
INITFUNC ("vmmcal0", vmmcall_listmodule_init);


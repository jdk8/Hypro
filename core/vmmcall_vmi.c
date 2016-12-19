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
#include "limits.h"
#include "assert.h"
#include "constants.h"
#include "cpu_mmu.h"
#include "current.h"
#include "gmm_access.h"
#include "mm.h"
#include "panic.h"
#include "printf.h"
#include "string.h"

#define NUM_OF_EPTBL    1024
#define EPTE_READ   0x1
#define EPTE_READEXEC   0x5
#define EPTE_WRITE  0x2
#define EPTE_ATTR_MASK  0xFFF
#define EPTE_MT_SHIFT   3
#define EPT_LEVELS  4

struct vt_ept {
    int cnt;
    void *ncr3tbl;
    phys_t ncr3tbl_phys;
    void *tbl[NUM_OF_EPTBL];
    phys_t tbl_phys[NUM_OF_EPTBL];
};

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

u64 virt_pte(ulong virtaddr){
    struct memdump_data data;
    u64 ent[5];
    int r, levels;
    memset(ent, 0, sizeof(ent));
    memset(&data, 0, sizeof(struct memdump_data));
    data.virtaddr = virtaddr;
    get_control_regs((ulong *)&data.cr0, (ulong *)&data.cr3, (ulong *)&data.cr4, &data.efer);
    r = cpu_mmu_get_pte(data.virtaddr, (ulong)data.cr0, (ulong)data.cr3, (ulong)data.cr4, data.efer, false, false, false, ent, &levels);
    
    if (r ==  VMMERR_SUCCESS){
        return ent[0];
    }

    return 0;
}


u64 virt_phys(ulong virtaddr){
    struct memdump_data data;
    u64 ent[5];
    int r, levels;
    memset(ent, 0, sizeof(ent));
    memset(&data, 0, sizeof(struct memdump_data));
    data.virtaddr = virtaddr;
    get_control_regs((ulong *)&data.cr0, (ulong *)&data.cr3, (ulong *)&data.cr4, &data.efer);
    r = cpu_mmu_get_pte(data.virtaddr, (ulong)data.cr0, (ulong)data.cr3, (ulong)data.cr4, data.efer, false, false, false, ent, &levels);
    
    if (r ==  VMMERR_SUCCESS){
        data.physaddr = (ent[0] & PTE_ADDR_MASK64) | ((data.virtaddr) & 0xFFF);
        return data.physaddr;
    }

    return 0;
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

int
virt_memcpy1(ulong virtaddr,  u64 value){
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
       
      
       write_hphys_q(data.physaddr, value, 0);
 
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

extern bool print_mm = false;

static void
set_pte (void){

   vt_ept_map_page (false, 0x1c52ff0, 0, true);

}

static void
listprocess (void){
   print_mm = true;
    u64 current_process;
    unsigned long tasks_offset, pid_offset, name_offset;
    u64 list_head = 0, next_list_entry = 0;
    u32 pid = 0;
   u64 pte;


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
        printf("  (struct addr:%llx\n", current_process);
   
        virt_memcpy(next_list_entry, 8, &next_list_entry);
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


 
static int listmodule()
{
    ulong rcx = 0, bufsize;  //
    u64 next_module, list_head = next_module = 0xffffffff81a27fd0;

    current->vmctl.read_general_reg (GENERAL_REG_RCX, &rcx);//
    bufsize = rcx < USHRT_MAX ? rcx : USHRT_MAX;//
    char *buf = alloc(bufsize);//
    if (!buf || bufsize == 0)//
        return VMI_FAILURE;//

    memset(buf, 0, bufsize);//

    snprintf(buf, bufsize, "%-36s    Struct Addr\n", "Kernel Module");//

    virt_memcpy(list_head, 8, &next_module);
 
    while (list_head != next_module)
    {
        char *modname = NULL;
        int bufusedsize, namesize;//
        modname = read_str_va(next_module + 16);
        bufusedsize = strlen(buf);//
        namesize = strlen(modname);//
        if (bufsize > bufusedsize + namesize + 80) // 80 is arbitary value include next_module and so on
        {//
            snprintf(&buf[bufusedsize], bufsize - bufusedsize, "%-36s    0x%lX\n", modname, next_module);//
        }//
        free(modname);

        virt_memcpy(next_module, 8, &next_module);
    }

    memcpy_to_usespace(buf, bufsize);//

    free(buf);//
    return VMI_SUCCESS;
}

static void
vmmcall_listprocess_init (void){
    vmmcall_register ("vmmcall_listprocess", listprocess);
}



static void
vmmcall_set_pte_init (void){
    vmmcall_register ("vmmcall_set_pte", set_pte);
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
INITFUNC ("vmmcal0", vmmcall_set_pte_init);
INITFUNC ("vmmcal0", vmmcall_printlog_init);
INITFUNC ("vmmcal0", vmmcall_dump_memory_init);
INITFUNC ("vmmcal0", vmmcall_listmodule_init);


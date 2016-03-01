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
#include "tty.h"


#define VMMSIZE_ALL             (128 * 1024 * 1024)

typedef enum status{
  VMI_SUCCESS,
  VMI_FAILURE
} status_t;

u64 htoi(char * str)
{
  u64 res=0;
  int i;
  for(i=2;str[i]!='\0';i++) {
    //printf("%c", str[i]);
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

struct memdump_hvirt_data {
  u64 *virtsrc, *virtdes;
  int sendlen;
};


static void 
printhex(unsigned char * data, int length, u64 addr){
  int i, j;
  unsigned char * tmp = data;
  
  for(j = 1; j <= length/8; ++j){
    //print address
    printf("0x%08llx ", addr);
    //print data in that address
    for(i=0; i < 8; ++i){
      printf(" %02X", tmp[i]);
    }
    
    for (i = 0; i < 8; ++i)
      { 
	//if (isprint(tmp[i]))
	//{
	//  printf(" %c", tmp[i]);
	// }
	//else
	// printf(" .");
      }
    
    printf("\n");
    addr+=0x8;
    tmp = data+8*j;
  }
}

void
memdump_hphys(u64 hphys){
  u8 * phphys;
  int length = 128;
  phphys = mapmem_hphys (hphys, length, 0);
  static char * hpdes; 
  hpdes = alloc(length);
  memset(hpdes, 0, length);
  memcpy(hpdes, phphys, length);
  printhex(hpdes, length, (u64)hpdes);
}

static void 
get_control_regs(ulong * cr0, ulong * cr3, ulong * cr4, u64 * efer){
  current->vmctl.read_control_reg (CONTROL_REG_CR0, cr0);
  current->vmctl.read_control_reg (CONTROL_REG_CR3, cr3);
  current->vmctl.read_control_reg (CONTROL_REG_CR4, cr4);
  current->vmctl.read_msr (MSR_IA32_EFER, efer);
}

void memdump_gphys(long gphys, int type, void * value){
  char data[16];
  memset(data, 0, sizeof(data));
  
  if (type == 0) {
    read_gphys_b(gphys, (u32 *)value, 0);
  } else if (type == 1) {
    read_gphys_b(gphys, (u64 *)value, 0);
  }
  //printhex(data, 16, gphys);
  
}

void convert_gvirt_to_gphys(long gvirt, long * gphys){
  struct memdump_data data;
  memset(&data, 0, sizeof(struct memdump_data));
  data.virtaddr = gvirt;
  int i, r;
  u64 ent[5];
  int levels;
  u64 physaddr;
    
  memset(ent, 0, sizeof(ent));
  get_control_regs((ulong *)&data.cr0, (ulong *)&data.cr3, (ulong *)&data.cr4, &data.efer);
  r = cpu_mmu_get_pte(data.virtaddr, (ulong)data.cr0, (ulong)data.cr3,
                      (ulong)data.cr4,
		      data.efer, true, false, false, ent, 
		      &levels);
  if (r == 0)
    {
      physaddr = (ent[0] & PTE_ADDR_MASK64) | ((data.virtaddr) & 0xFFF);
      *gphys = physaddr;
    }
  else{
    printf("error!: r = %d\n", r);
  }
   
}


int 
memdump_gvirt(struct memdump_data * dumpdata, void * value){
  long gvirt = 0xffffffff81c0e000ULL - (0xffffffff81000000ULL - 0x0000000001000000ULL);
  struct memdump_data * data = dumpdata;
  int i, r;
  u64 ent[5];
  int levels;
  u64 physaddr;
  int width = 8;
  char str[width];
  memset(ent, 0, sizeof(ent));
  get_control_regs((ulong *)&data->cr0, (ulong *)&data->cr3, (ulong *)&data->cr4, &data->efer);
  
  for (i = 0; i < data->sendlen; i += width)
    {
      r = cpu_mmu_get_pte(data->virtaddr+i, (ulong)data->cr0, (ulong)data->cr3, (ulong)data->cr4, 
		      data->efer, true, false, false, ent, 
		      &levels);
      if (r == VMMERR_SUCCESS)
	{
	  physaddr = (ent[0] & PTE_ADDR_MASK64) | ((data->virtaddr+i) & 0xFFF);
	  read_hphys_q(physaddr, str, 0);
	  memcpy(value+i, str, width);
	  printhex(str, width, physaddr);
	  memset(str, 0, width);
	}
      else{
	printf("error: r = %d\n", r);
	break;
      }
    }
  return r;
  
}

u64 vmi_translate_ksym2v(char * ksym)
{
  u64 addr = 0;
  if ( !strcmp(ksym, "init_task") ) {
    addr  = htoi(config.vmi.init_task);
  }
  return addr;
}

u64 vmi_get_offset(char *offset_name)
{
  if (strcmp(offset_name, "linux_tasks") == 0) {
    return 0x270;
  } else  if (strcmp(offset_name, "linux_pid") == 0) {
    return 0x2e4;
  } else  if (strcmp(offset_name, "linux_name") == 0) {
    return 0x4a8;
  }
  return 0x0;
}


void virt_memdump32(u64 vaddr, u32 *value)
{
  struct memdump_data * data;
  data = alloc(sizeof(struct memdump_data));
  memset(data, 0, sizeof(struct memdump_data));
  data->virtaddr = vaddr;
  data->sendlen = 4;
  memdump_gvirt(data, value);
    
  return ;
}

int virt_memdump64(u64 vaddr, u64 *value)
{
  struct memdump_data * data;
  data = alloc(sizeof(struct memdump_data));
  memset(data, 0, sizeof(struct memdump_data));
  data->virtaddr = vaddr;
  data->sendlen = 8;
  int type = 1;
  int status = memdump_gvirt(data, value);
  return status;
}

char * virt_memdumpstr(u64 vaddr)
{
  char *value = NULL;
  return value;
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
  //printf("[%s r=%d 0x%016llx]\n", __func__, r, virtaddr);
  if (r == VMMERR_SUCCESS)
    {
      
      data.physaddr = (ent[0] & PTE_ADDR_MASK64) | ((data.virtaddr) & 0xFFF);
      if(nr_bytes == 4)
	read_hphys_l(data.physaddr, value, 0);
      if(nr_bytes == 8)
	read_hphys_q(data.physaddr, value, 0);
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
 // printf("[%s r=%d 0x%016llx]\n", __func__, r, vaddr);
  int c=0; // parport_pcpc
  if (r == VMMERR_SUCCESS)
  {
    data.physaddr = (ent[0] & PTE_ADDR_MASK64) | ((data.virtaddr) & 0xFFF);
    read_hphys_q(data.physaddr, &x, 0);
    read_hphys_q(data.physaddr+8, &y, 0);
    read_hphys_q(data.physaddr+16, &z, 0);
    for(i=0;i<8;i++) {
      int t=x&0xff;
      //  if (t==0) {
       //   break;
      //  }
      //  printf("%c", t);
        ret_val[c++] = t;
        x=x>>8;
    }
    //printf("\t\t\t\t%lx\t\t\t\t\t",y);
    for(i=0;i<8;i++) {
      int t=y&0xff;
      //  if (t==0) {
       //   break;
      //  }
      //  printf("%c", t);
        ret_val[c++] = t;
        y=y>>8;
    }
    for(i=0;i<8;i++) {
      int t=z&0xff;
      //  if (t==0) {
       //   break;
      //  }
      //  printf("%c", t);
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
test (void){
  u64 current_process;
  unsigned long tasks_offset, pid_offset, name_offset;
  u64 list_head = 0, current_list_entry = 0, next_list_entry = 0;
  u32 pid = 0;
  char *procname = NULL;
  int i;

//  printf("CONFFFFFFFFFFFFFFFFFFF %lx \t\t %lx \t\t %lx\n", htoi(config.vmi.linux_tasks), htoi(config.vmi.linux_pid),  htoi(config.vmi.linux_name));

  tasks_offset = htoi(config.vmi.linux_tasks);//
            // vmi_get_offset("linux_tasks"); 
  pid_offset = htoi(config.vmi.linux_pid);//
            // vmi_get_offset("linux_pid");
  name_offset = htoi(config.vmi.linux_name);//
            // vmi_get_offset("linux_name");

  list_head = vmi_translate_ksym2v("init_task") + tasks_offset;


  printf("list_head: %lx\n",  tasks_offset);
 printf("list_head: %lx\n",  pid_offset);
  printf("list_head: %lx\n",  name_offset);


 // printf("list_head: %lx\n",  vmi_translate_ksym2v("init_task"));
 // printf("list_head: %lx\n",  vmi_translate_ksym2v("init_task"));

  next_list_entry = list_head;
  do {

      current_process = next_list_entry - tasks_offset;
      //virt_memdump32(current_process + pid_offset, &pid); // int-32bits
      virt_memcpy(current_process + pid_offset, 4, &pid);
      printf("[PID:%5d] ",pid);
      //printf("process name: ");
      char *modname = NULL;
      modname = read_str_va(current_process + name_offset);
      printf("process name: %s", modname);
      printf("  (struct addr:%lx)\n", current_process);
      //printf("[PID:%5d] process name: %s (struct addr:%lx)\n", pid, procname, current_process);
      //int status = virt_memdump64(next_list_entry, &next_list_entry);
      virt_memcpy(next_list_entry, 8, &next_list_entry);
      /* if (status != VMMERR_SUCCESS) */
      /* 	{ */
      /* 	  printf("failed to read next pointer in loop at %x\n", next_list_entry); */
      /* 	  return; */
      /* 	} */
      free(modname);
     } while (next_list_entry != list_head);
}

static void dump_memory()
{
    const int MAX_PAGE_NUM = 100;
    const int PAGE_SIZE = 4096;
    unsigned char * memory = (unsigned char *)alloc(PAGE_SIZE*sizeof(unsigned char));
    int i=0,j=0;
    u64 addr = 0;
    u64 tmp = -1;
    //read_gphys_b(gphys, (u32 *)value, 0);
    for(i=0; i<MAX_PAGE_NUM; i++) {
      //printf("Page Number %d\n", i);
      read_hphys_q(addr, &tmp, 0);
      printf("%lx:", addr);
      addr = addr + 8;
      for(j=0;j<8;j++) {
      int t=tmp&0xff;
        //if (t==0) {
        //  break;
       // }
        if (t>=16) {
            printf(" %x", t);
        } else {
            printf(" 0%x", t);
        }
        tmp=tmp>>8;
       }
  
      //read_gphys_b(addr, memory, 0);
      /*for(j=0;j<PAGE_SIZE;j++) {
         printf("%lx", memory[j]);
         //if (j%50==0) {
         //   printf("\n");
         //}
      }
      addr = addr + PAGE_SIZE; */
      printf("\n");/*
      free(memory);
        memory = (unsigned char *)alloc(PAGE_SIZE*sizeof(unsigned char));*/
    }
}

static int
memdump_hpa_to_gva(struct memdump_data * des, char * src){
  int i, j = 0, r, width = 8;
  u64 ent[5];
  u64 physaddr;
  int levels;
  char str[width];
  memset(&str, 0, sizeof str);
  get_control_regs(&des->cr0, &des->cr3, &des->cr4, &des->efer);

  for (i = 0; i < des->sendlen; i++)
    {
      r = cpu_mmu_get_pte(des->virtaddr+i, (ulong)des->cr0, (ulong)des->cr3, (ulong)des->cr4,
                      des->efer, true, false, false, ent,
                      &levels);
      if (r == VMMERR_SUCCESS)
        {
          physaddr = (ent[0] & PTE_ADDR_MASK64) | ((des->virtaddr+i) & 0xFFF);
          write_hphys_b(physaddr, src[i], 0);
          /* memcpy(str, des+i, width); */
          /* for (j = 0; j < width; ++i) */
          /*   { */
          /*     write_hphys_q(physaddr+j, str[j], 0); */
          /*   } */
          /* memcpy(&str, des+i, width); */
          /* printf("%llx", str); */
          /* write_hphys_q(physaddr, str, 0); */
          /* if (src[i] != NULL) */
          /*   { */
          /*     write_hphys_b(physaddr, , 0); */
          /*   } */
          /* else{ */
          /*   write_hphys_b(physaddr,'\0', 0); */
          /*   return; */
          /* } */

        }
      else{
        printf("error: r = %d\n", r);
        break;
      }
    }
  return r;
}


static int
get_vmmlog(){
  ulong rbx, rcx;
  u64 gva;
  int sendlen;
  struct memdump_data data;
  current->vmctl.read_general_reg (GENERAL_REG_RBX, &rbx);
  current->vmctl.read_general_reg (GENERAL_REG_RCX, &rcx);

  data.virtaddr = rbx;
  data.sendlen = rcx;

 if (memdump_hpa_to_gva(&data, log) == VMI_FAILURE)
    {
      printf("error when copy log to string!\n");
      return VMI_FAILURE;
    }
  return VMI_SUCCESS;
}


static int
listmodel(){
  printf("error when copy log to string!\n");
    u64 next_module, list_head;
    //    vmi_read_addr_ksym(vmi, "modules", &next_module);
    next_module = 0xffffffff81c52ff0;
    list_head = next_module;
    

/* walk the module list */
    while (1) {

        /* follow the next pointer */
        u64 tmp_next = 0;
        virt_memcpy(next_module, 8, &tmp_next);
      //  printf("  (struct addr:%lx)", tmp_next);

        /* if we are back at the list head, we are done */
        if (list_head == tmp_next) {
            break;
        }

        /* print out the module name */

        /* Note: the module struct that we are looking at has a string
         * directly following the next / prev pointers.  This is why you
         * can just add the length of 2 address fields to get the name.
         * See include/linux/module.h for mode details */

         char *modname = NULL;
         modname = read_str_va(next_module + 16);
         printf("kernel module name: %s", modname);
         printf("\t(struct addr:%lx)\n", tmp_next);
         free(modname);
         next_module = tmp_next;
    }


  return VMI_SUCCESS;
}


static void
vmmcall_test_init (void){
  vmmcall_register ("vmmcall_test", test);
//   vmmcall_register ("vmmcall_test", dump_memory);
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
vmmcall_listmodel_init(){
  vmmcall_register ("vmmcall_listmodel", listmodel);
}

INITFUNC ("vmmcal0", vmmcall_test_init);
INITFUNC ("vmmcal0", vmmcall_printlog_init);
INITFUNC ("vmmcal0", vmmcall_dump_memory_init);
INITFUNC ("vmmcal0", vmmcall_listmodel_init);


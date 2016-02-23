#include "../common/call_vmm.h"
#include <stdio.h> 

static int
vmmcall_listmodel (void)
{
      printf("Hello World!\n");
         printf("%s\n", __func__);
     call_vmm_function_t f;
     call_vmm_arg_t a;
     call_vmm_ret_t r;

    CALL_VMM_GET_FUNCTION ("vmmcall_listmodel", &f);
    //CALL_VMM_GET_FUNCTION ("vmmcall_dump_memory", &f);
 
    if (!call_vmm_function_callable (&f)){
             printf("error!\n");
         return -1;
     }
     a.rbx = (long)65536;
     call_vmm_call_function (&f, &a, &r);
     return 0;
}

static void listmodel()
{
   if(vmmcall_listmodel())
     return;
}

int
main()
{
   printf("1111111111111\n");
   printf("%s\n", __func__);
   listmodel();
}


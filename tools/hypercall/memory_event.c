#include "../common/call_vmm.h"
#include <stdio.h> 

static int
vmmcall_memory_event (void)
{
    printf("memory_event!\n");
    printf("%s\n", __func__);
    call_vmm_function_t f;
    call_vmm_arg_t a;
    call_vmm_ret_t r;

    CALL_VMM_GET_FUNCTION ("vmmcall_memory_event", &f);
 
    if (!call_vmm_function_callable (&f)){
        printf("error!\n");
        return -1;
    }
    a.rbx = (long)65536;
    call_vmm_call_function (&f, &a, &r);

    return 0;
}

static void memory_event()
{
    if(vmmcall_memory_event())
        return;
}

int
main()
{
   printf("%s\n", __func__);
   memory_event();
}
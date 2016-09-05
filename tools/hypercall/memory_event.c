#include "../common/call_vmm.h"
#include <stdio.h> 
#include <limits.h>
#include <string.h>
 
static int
vmmcall_memory_event (void)
{
    char buf[USHRT_MAX];//
    printf("memory_event!\n");
    printf("%s\n", __func__);
    call_vmm_function_t f;
    call_vmm_arg_t a;
    call_vmm_ret_t r;
    memset(buf, 0, USHRT_MAX);//

    CALL_VMM_GET_FUNCTION ("vmmcall_memory_event", &f);
 
    if (!call_vmm_function_callable (&f)){
        printf("error!\n");
        return -1;
    }

    memset(&a, 0, sizeof(call_vmm_arg_t));//
    memset(&r, 0, sizeof(call_vmm_ret_t));//

    a.rbx = (unsigned long long)buf;//
    a.rcx = USHRT_MAX;//
       
    call_vmm_call_function (&f, &a, &r);
    printf("%s", buf);//

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
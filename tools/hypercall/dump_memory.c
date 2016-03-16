#include "../common/call_vmm.h"
#include <stdio.h> 

static int
vmmcall_dump_memory (void)
{
    printf("dump_memory!\n");
    printf("%s\n", __func__);
    call_vmm_function_t f;
    call_vmm_arg_t a;
    call_vmm_ret_t r;

    CALL_VMM_GET_FUNCTION ("vmmcall_dump_memory", &f);

    if (!call_vmm_function_callable (&f)){
        printf("error!\n");
        return -1;
    }
    a.rbx = (long)65536;
    call_vmm_call_function (&f, &a, &r);
    return 0;
}

static void dump_memory()
{
    if(vmmcall_dump_memory())
        return;
}

int
main()
{
    printf("%s\n", __func__);
    dump_memory();
}


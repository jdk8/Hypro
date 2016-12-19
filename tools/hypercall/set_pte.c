#include "../common/call_vmm.h"
#include <stdio.h> 

static int
vmmcall_set_pte (void)
{
    printf("set_pte!\n");
    printf("%s\n", __func__);
    call_vmm_function_t f;
    call_vmm_arg_t a;
    call_vmm_ret_t r;

    CALL_VMM_GET_FUNCTION ("vmmcall_set_pte", &f);
 
    if (!call_vmm_function_callable (&f)){
        printf("error!\n");
        return -1;
    }
    a.rbx = (long)65536;
    call_vmm_call_function (&f, &a, &r);

    return 0;
}

static void set_pte()
{
    if(vmmcall_set_pte())
        return;
}

int
main()
{
   printf("%s\n", __func__);
   set_pte();
}

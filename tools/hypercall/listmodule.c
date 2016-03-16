#include "../common/call_vmm.h"
#include <stdio.h> 

static int
vmmcall_listmodule (void)
{
    printf("listmodule!\n");
    printf("%s\n", __func__);
    call_vmm_function_t f;
    call_vmm_arg_t a;
    call_vmm_ret_t r;

    CALL_VMM_GET_FUNCTION ("vmmcall_listmodule", &f);

    if (!call_vmm_function_callable (&f)) {
        printf("error!\n");
        return -1;
    }
    a.rbx = (long)65536;
    call_vmm_call_function (&f, &a, &r);
    return 0;
}

static void listmodule()
{
    if(vmmcall_listmodule())
        return;
}

int
main()
{
    printf("%s\n", __func__);
    listmodule();
}


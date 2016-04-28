#include "../common/call_vmm.h"
#include <stdio.h> 
#include <limits.h>
#include <string.h>
 
int
main()
{
        char buf[USHRT_MAX];//
    call_vmm_function_t f;
    call_vmm_arg_t a;
    call_vmm_ret_t r;
    memset(buf, 0, USHRT_MAX);//

    CALL_VMM_GET_FUNCTION ("vmmcall_listmodule", &f);
    if (!call_vmm_function_callable (&f))
    {
        printf("call_vmm_function_callable failed in %s:%d!\n", __func__, __LINE__);
        return -1;
    }

    memset(&a, 0, sizeof(call_vmm_arg_t));//
    memset(&r, 0, sizeof(call_vmm_ret_t));//

    a.rbx = (unsigned long long)buf;//
    a.rcx = USHRT_MAX;//
    call_vmm_call_function(&f, &a, &r);

    printf("%s", buf);//

    return 0;
}

 
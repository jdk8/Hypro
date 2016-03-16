#include "../common/call_vmm.h"
#include <stdio.h> 

static int
vmmcall_print_log (void)
{
    printf("%s\n", __func__);
    char log[65536];
    unsigned long long int addr;
    call_vmm_function_t f;
    call_vmm_arg_t a;
    call_vmm_ret_t r;

    memset(log, 0, sizeof (log));
    CALL_VMM_GET_FUNCTION ("vmmcall_getvmmlog", &f);
 
    if (!call_vmm_function_callable (&f)){
        printf("error!\n");
        return -1;
    }

    memset(&a, 0, sizeof (call_vmm_arg_t));
    memset(&r, 0, sizeof (call_vmm_ret_t));

    addr = (unsigned long long int )log;
    a.rbx = addr;
    a.rcx = sizeof log;
    printf("%llx\n", addr);
    call_vmm_call_function (&f, &a, &r);
    printf("log is: %s\n", log);
    return 0;
}


int
main()
{
    printf("%s\n", __func__);
    vmmcall_print_log();
}


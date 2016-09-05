#include "../common/call_vmm.h"
#include <stdio.h> 
#include <string.h>

static int
vmmcall_reg_event_read (void)
{
    printf("%s\n", __func__);
    call_vmm_function_t f;
    call_vmm_arg_t a;
    call_vmm_ret_t r;

    CALL_VMM_GET_FUNCTION ("vmmcall_reg_event_read", &f);
 
    if (!call_vmm_function_callable (&f)){
        printf("error!\n");
        return -1;
    }
    a.rbx = (long)65536;
    call_vmm_call_function (&f, &a, &r);

    return 0;
}


static int
vmmcall_reg_event_cr0_read (void)
{
    printf("%s\n", __func__);
    call_vmm_function_t f;
    call_vmm_arg_t a;
    call_vmm_ret_t r;

    CALL_VMM_GET_FUNCTION ("vmmcall_reg_event_cr0_read", &f);
 
    if (!call_vmm_function_callable (&f)){
        printf("error!\n");
        return -1;
    }
    a.rbx = (long)65536;
    call_vmm_call_function (&f, &a, &r);

    return 0;
}

static void reg_event_read(char *reg)
{
    if (strcmp(reg,"msr")==0){
        if(vmmcall_reg_event_read())
        return;
    }else if (strcmp(reg,"cr0")==0){
        if(vmmcall_reg_event_cr0_read())
        return;
    }
    
}

static int
vmmcall_reg_event_write (void)
{
    printf("%s\n", __func__);
    call_vmm_function_t f;
    call_vmm_arg_t a;
    call_vmm_ret_t r;

    CALL_VMM_GET_FUNCTION ("vmmcall_reg_event_write", &f);
 
    if (!call_vmm_function_callable (&f)){
        printf("error!\n");
        return -1;
    }
    a.rbx = (long)65536;
    call_vmm_call_function (&f, &a, &r);

    return 0;
}


static int
vmmcall_reg_event_cr0_write (void)
{
    printf("%s\n", __func__);
    call_vmm_function_t f;
    call_vmm_arg_t a;
    call_vmm_ret_t r;

    CALL_VMM_GET_FUNCTION ("vmmcall_reg_event_cr0_write", &f);
 
    if (!call_vmm_function_callable (&f)){
        printf("error!\n");
        return -1;
    }
    a.rbx = (long)65536;
    call_vmm_call_function (&f, &a, &r);

    return 0;
}

static void reg_event_write(char *reg)
{
    if (strcmp(reg,"msr")==0){
        if(vmmcall_reg_event_write())
            return;
    } else if (strcmp(reg,"cr0")==0){
        if(vmmcall_reg_event_cr0_write())
        return;
    }
}

int
main()
{
   printf("%s\n", __func__);
   //reg_event_read("cr0");
   reg_event_write("cr0");
}

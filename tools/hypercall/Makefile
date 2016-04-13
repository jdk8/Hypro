.PHONY : all
all : listprocess listmodule dump_memory printlog memory_event

.PHONY : clean
clean :
	$(RM) listprocess listmodule dump_memory printlog memory_event

listprocess : listprocess.c ../common/call_vmm.c ../common/call_vmm.h
	$(CC) -g -s -o listprocess listprocess.c ../common/call_vmm.c

listmodule : listmodule.c ../common/call_vmm.c ../common/call_vmm.h
	$(CC) -g -s -o listmodule listmodule.c ../common/call_vmm.c

dump_memory : dump_memory.c ../common/call_vmm.c ../common/call_vmm.h
	$(CC) -g -s -o dump_memory dump_memory.c ../common/call_vmm.c

printlog : printlog.c ../common/call_vmm.c ../common/call_vmm.h
	$(CC) -g -s -o printlog printlog.c ../common/call_vmm.c

memory_event : memory_event.c ../common/call_vmm.c ../common/call_vmm.h
	$(CC) -g -s -o memory_event memory_event.c ../common/call_vmm.c

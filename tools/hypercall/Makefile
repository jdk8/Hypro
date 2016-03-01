.PHONY : all
all : test pl listmodel printlog

.PHONY : clean
clean :
	$(RM) test pl listmodel printlog

test : test.c ../common/call_vmm.c ../common/call_vmm.h
	$(CC) -g -s -o test test.c ../common/call_vmm.c

pl : pl.c ../common/call_vmm.c ../common/call_vmm.h
	$(CC) -g -s -o pl pl.c ../common/call_vmm.c

listmodel : listmodel.c ../common/call_vmm.c ../common/call_vmm.h
	$(CC) -g -s -o listmodel listmodel.c ../common/call_vmm.c

printlog : printlog.c ../common/call_vmm.c ../common/call_vmm.h
	$(CC) -g -s -o printlog printlog.c ../common/call_vmm.c

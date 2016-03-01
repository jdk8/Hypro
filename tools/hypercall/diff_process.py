import os
import string

os.system("./printlog > process.txt");
os.system("ps -ef > process1.txt")

patt = " process name"
pids = []
for line in open('process.txt'):
    line = line.strip('\n')
    if patt in line:
        procname = ''
        for i in range(6, line.find(patt)-1):
            procname = '%s%s' %(procname,line[i])
        pids.append(string.atoi(procname))


vmpids = []
isfirstline = True
for line in open("process1.txt"):
    if isfirstline == False:
        line = line.strip('\n')
        pid = ''
        for i in range(line.find(" "),len(line)):
            if line[i]!=" ":
                pid = '%s%s' %(pid,line[i])
            if pid!='' and line[i]==" ":
                break
        vmpids.append(string.atoi(pid))
    isfirstline = False

for pid in pids:
    if pid not in vmpids:
        print pid

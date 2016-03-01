import os
import string

os.system("./printlog > mod.txt");
os.system("lsmod > mod1.txt")

patt = "kernel module name: "
patt1 = "\t(struct addr"
vmmkmnames = []
for line in open('mod.txt'):
    line = line.strip('\n')
    if patt in line:
        kmname = ''
        for i in range(20, line.find(patt1)):
            kmname = '%s%s' %(kmname, line[i])
        vmmkmnames.append(kmname)


l = 0
vmkmnames = []
for line in open('mod1.txt'):
    line = line.strip('\n')
    if l == 0:
        l = l+1
        continue
    kmname = ''
    for i in range(0,line.find(" ")):
        kmname = "%s%s" %(kmname, line[i])
    vmkmnames.append(kmname)


for kmname in vmmkmnames:
    if kmname not in vmkmnames:
        print kmname
    

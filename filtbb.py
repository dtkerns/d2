#!/usr/bin/env python3
import sys,os

def filtbb(filenames):
  lomf=["atan", "atan2", "cos", "log", "pow", "sin", "sqrt"]
  loopcount = {}
  sb = {}
  f1 = f2 = f3 = True
  for file in filenames:
    if f1 and file.endswith('loopdb'):
      f1 = False
      with open(file, "r") as f:
        data = f.read().splitlines()
        for d in data:
          flds = d.split(':')
          BB = '_'.join([flds[0], flds[2]])
          loopcount[BB] = flds[3]
    if f2 and file.endswith('bbsbdb'):
      f2 = False
      with open(file, "r") as f:
        data = f.read().splitlines()
        for d in data:
          flds = d.split(':')
          BB = '_'.join([flds[0], flds[1]])
          sb[BB] = ",".join(flds[3:])
    if f3 and file.endswith('bbdb'):
      f3 = False
      with open(file, "r") as f:
        data = f.read().splitlines()
        for d in data:
          flds = d.split(':')
          BB = '_'.join([flds[0], flds[3]])
          if not BB in loopcount: loopcount[BB] = 0
          if not BB in sb: sb[BB] = ""
          MF = 0
          for mf in flds[12].split(","):
            if mf in lomf:
              MF += 1
          for i in range(4,12):
            #print("%d %s" % (i, flds[i]), file=sys.stderr)
            MF += int(flds[i].split(' ')[1])
          print("%s:%s:%d:%d:%s" % (flds[0], flds[3], loopcount[BB], MF, sb[BB]))
  return 0

if __name__ == "__main__":
  if len(sys.argv) < 4:
    print("usage: %s loopdb bbsbdb bbdb" % sys.argv[0])
    sys.exit(1)
  filtbb(sys.argv[1:4])

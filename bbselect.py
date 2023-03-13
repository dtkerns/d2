#!/usr/bin/env python3
import sys,os,subprocess

def bbselect(filename):
  with open(filename, "r") as f:
    data = f.read().splitlines()
  n = 0
  wc = {}
  thresh = {}
  for i in range(len(data)-1):
    file = data[i]
    with open(file, "r") as f1:
      data1 = f1.read().splitlines()
    wc[file] = len(data1)
    thresh[file] = int(wc[file] * .8)
    for j in range(1, len(data)):
      cmd = ["comm", "-12", "--nocheck-order", "--total", file, data[j]]
      a = subprocess.run(cmd, capture_output=True, text=True).stdout.splitlines()[-1].lstrip().split()
      if int(a[2]) > thresh[file]:
        print("%s %s %d %s %s %s" % (file, data[j], wc[file], a[0], a[1], a[2]))
  return 0

if __name__ == "__main__":
  bbselect(sys.argv[1])

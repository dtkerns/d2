#!/usr/bin/env python3

import sys,os,re

def norm(file):
  p_lab = re.compile("(\d*):$")
  p_reg = re.compile("%(\d*),?$")
  with open(file, "r") as f:
    data = f.read().splitlines()
  fl = []
  reg = 1
  ereg = 1
  nlable = 0
  # pass 1, grab all the assigned regs (LHS)
  l = 0
  for d in data:
    l += 1
    flds = d.lstrip().split(' ')
    #print(flds[0], d)
    g = p_lab.match(flds[0])
    if g:
      fl.append(int(g[1]))
      nlable += 1
      if nlable > 1:
        print("unexpected label in file %s at line %d: %s" % (file, l, d))
        return 1
      #print(g[1])
    g = p_reg.match(flds[0])
    if g:
      fl.append(int(g[1]))
      #print(g[1])
    #print(d)
  #print(fl)
  # create a map of old reg to new reg
  fd = {}
  for idx in fl:
    fd["%" + "%d" % idx] = "%" + "%d" % reg
    reg += 1
  #print(fd)
  # pass 2, rename regs while looking for external regs
  l = 0
  print("1:")
  for d in data[1:]:
    l += 1
    outline = d
    #print(outline)
    flds = d.lstrip().split(' ')
    for fld in flds:
      g = p_reg.match(fld)
      if g:
        reg = "%" + "%s" % g[1]
        #print(reg, fd.keys(), (reg in fd))
        # if the reg to be substituted is not in the map, then it must be an external (to this block) register
        if reg not in fd:
          fd[reg] = "%E" + ("%d" % ereg)
          ereg += 1
          #print("new", reg, fd)
        outline = outline.replace(reg, fd[reg])
    print(outline)
  return 0

if __name__ == "__main__":
  sys.exit(norm(sys.argv[1]))

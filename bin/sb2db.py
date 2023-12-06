#!/usr/bin/env python3

import sys,os

def clean(s):
  return s.replace(";", "")

def sb2db(filename):
  ofile = filename[:-2] + "ll" # get back to .ll source filename
  with open(filename, "r") as f:
    data = f.read().splitlines()
  l = 0
  fnm = ""
  sb = 0 # 0 based name
  bb = {}
  for d in data:
    l += 1
    flds = d.lstrip().split(' ')
    if len(flds) == 0: continue
    # start of new sb
    if "digraph" in d:
      sbs = "%04d" % sb
      sb += 1
      continue
    # parse edges
    if len(flds) > 1 and flds[1] == "->":
      if flds[2] == "exit;": continue
      if flds[0] != "entry":
        cs = clean(flds[0])
        bbkey = ofile + ":" + cs
        if not bbkey in bb: bb[bbkey] = []
        if not sbs in bb[bbkey]: bb[bbkey].append(sbs)
      cs = clean(flds[2])
      bbkey = ofile + ":" + cs
      if not bbkey in bb: bb[bbkey] = []
      if not sbs in bb[bbkey]: bb[bbkey].append(sbs)
  # print records
  for bbkey in bb.keys():
    print("%s:%s" % (bbkey, ":".join(bb[bbkey])))
  return 0
  
if __name__ == "__main__":
  for f in sys.argv[1:]:
    sb2db(f)


#!/usr/bin/env python3

import sys,os,re

def ll2bb(file):
  p_op = re.compile("[(].*")
  pfix = file[:-2] + "bb"
  os.makedirs(pfix, exist_ok=True)
  with open(file, "r") as f:
    data = f.read().splitlines()
  l = 0
  EO = False
  nm = ""
  outf = None
  for d in data:
    l += 1
    flds = d.lstrip().split(' ')
    if len(flds) == 0: continue
    if flds[0] == "define":
      for f in flds:
        if f[0] == '@':
          nm = re.sub(p_op, "", f[1:])
          break
      cur = "entry_" + nm
      EO = True
      outfile = os.path.join(pfix, cur)
      outf = open(outfile, "w")
      print(outfile)
    if len(flds[0]) and flds[0][-1] == ":":
      if outf: outf.close()
      cur = "L_" + nm + "_" + flds[0][:-1]
      EO = True
      outfile = os.path.join(pfix, cur)
      outf = open(outfile, "w")
      print(outfile)
    if flds[0] == ";" and flds[1] == "label":
      if outf: outf.close()
      cur = "L_" + nm + "_" + flds[1][:-1]
      EO = True
      outfile = os.path.join(pfix, cur)
      outf = open(outfile, "w")
      print(outfile)
    if flds[0] == "}":
      if outf: outf.close()
      EO = False
      continue
    if EO:
      print(d, file=outf)
  return 0

if __name__ == "__main__":
  for f in sys.argv[1:]:
    ll2bb(f)

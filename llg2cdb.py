#!/usr/bin/env python3

import sys,os,re

p_subr0 = re.compile("[(].*")

def llg2cdb(filename):
  llfile = filename[:-1]
  with open(filename, "r") as f:
    data = f.read().splitlines()
  l = 0
  fnm = ""
  dbg = {}
  for d in data:
    l += 1
    flds = d.lstrip().split(' ')
    if len(flds) == 0: continue
    # start of function
    if flds[0] == "define":
      for f in flds:
        if f[0] == '@':
          fnm = re.sub(p_subr0, "", f[1:])
          break
      curBB = "entry_" + fnm
      dbg[curBB] = []
    # start of BB
    if len(flds[0]) and flds[0][-1] == ":":
      BB = flds[0][:-1]
      curBB = "L_" + fnm + "_" + BB
      dbg[curBB] = []
    if flds[0] == ";" and flds[1] == "label":
      curBB = "L_" + fnm + "_" + flds[1][:-1]
      dbg[curBB] = []
    if len(flds) > 3 and flds[-2] == "!dbg":
      if not flds[-1] in dbg[curBB]:
         dbg[curBB].append(flds[-1])
    if len(flds) > 3 and flds[0][0] == "!" and flds[1] == "=":
      if flds[0] == "!1":
        cfilename = flds[4].split('"')[1]
      if flds[0][1:].isdigit() and "line:" in flds:
        linemap[flds[0]] = flds[flds.index("line:")+1][:-1]
  for bb in dbg:
    po = []
    for pl in dbg[bb]:
      if pl in linemap:
        po.append(linemap[pl]) 
    print("%s:%s:%s:%s" % (llfile, bb, cfilename, ' '.join(po)))
  return 0

if __name__ == "__main__":
  for f in sys.argv[1:]:
    llg2cdb(f)


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
  linemap = {}
  curBB = None
  cfilename = None
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
      curBB = "L_entry_" + fnm
      dbg[curBB] = []
    # start of BB
    if len(flds[0]) and flds[0][-1] == ":":
      BB = flds[0][:-1]
      curBB = "L_" + fnm + "_" + BB
      dbg[curBB] = []
      #print("init1 BB %s" % curBB, file=sys.stderr)
    if flds[0] == ";" and "label" in flds[1]:
      curBB = "L_" + fnm + "_" + flds[1].split(':')[1]
      dbg[curBB] = []
      #print("init2 BB %s" % curBB, file=sys.stderr)
    if len(flds) > 3 and flds[-2] == "!dbg":
      dbgn = flds[-1][1:]
      #print("dbg %s" % dbgn, file=sys.stderr)
      try:
        if not dbgn in dbg[curBB]:
           dbg[curBB].append(dbgn)
           #print("add %s to %s" % (dbgn, curBB), file=sys.stderr)
        else:
           #print("%s already in %s" % (dbgn, curBB), file=sys.stderr)
           pass
      except KeyError:
        print("curBB not set for line %d: %s" % (l, d), file=sys.stderr)
    if len(flds) > 3 and flds[0][0] == "!" and flds[1] == "=":
      if cfilename == None and flds[1] == "=" and flds[2] == '!DIFile(filename:':
          cfilename = flds[3].split('"')[1]
      if flds[0][1:].isdigit() and flds[2] == '!DILocation(line:':
        dbgn = flds[0][1:]
        cline = flds[3][:-1]
        #print("dbg %s at line %s" % (dbgn, cline), file=sys.stderr)
        linemap[dbgn] = cline
  for bb in dbg:
    po = []
    for pl in dbg[bb]:
      #print("pl = %s for bb %s" % (pl, bb), file=sys.stderr)
      if pl in linemap and not linemap[pl] in po:
        po.append(linemap[pl]) 
    if len(po): print("%s:%s:%s:%s" % (llfile, bb, cfilename, ' '.join(po)))
  return 0

if __name__ == "__main__":
  for f in sys.argv[1:]:
    llg2cdb(f)


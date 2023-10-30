#!/usr/bin/env python3

import sys,os

def loop2db(filename):
  with open(filename, "r") as f:
    data = f.read().splitlines()
  l = 0
  fn = ""
  db = {}
  for d in data:
    l += 1
    flds = d.lstrip().split(' ')
    if len(flds) == 0: continue
    if len(flds) > 4 and flds[0] == "Printing" and flds[3] == "Loop":
      fn = d.split("'")[3]
      db[fn] = {}
    if len(flds) > 5 and flds[0] == "Loop" and flds[2] == "depth":
      depth = flds[3]
      for ss in flds[5].split(","):
        css = ss.split('<')[0][1:]
        if css.isdigit():
          idx = "L_" + fn + "_" + css
          db[fn][idx] = depth
  outf = filename[:-4] + "ll"
  for d in db.keys():
    for idx in db[d].keys():
      print(":".join((outf, d, idx, db[d][idx])))
  return 0

if __name__ == "__main__":
  for f in sys.argv[1:]:
    loop2db(f)


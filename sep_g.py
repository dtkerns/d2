#!/usr/bin/env python3
import sys,os
def sepg(filename):
  with open(filename, "r") as f:
    data = f.read().splitlines()
  n = 0
  for d in data:
    if "digraph" in d:
      if of: of.close()
      ofn = "g%04d.dot" % n
      n += 1
      of = open(ofn, "w")
    print(d, file=of)
  of.close()
  return 0
if __name__ == "__main__":
  sepg(sys.argv[1])

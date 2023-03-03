#!/usr/bin/env python3

import sys,os,re,copy

terminators = "ret br switch indirectbr invoke callbr resume catchswitch catchret cleanupret unreachable".split(" ")
expops = "mul fmul udiv sdiv fdiv urem srem frem".split(" ")
Zexpops = {}
for i in expops:
  Zexpops[i] = 0
p_subr0 = re.compile("[(].*")
#p_subr1 = re.compile("^@([^(]+)[(].+")
#p_subr2 = re.compile("^%([0-9]+)[(].+")

def clean(lbl, nm):
  return "L_" + nm + "_" + lbl.replace("%", "").replace(",", "")

def pfuncstats(filename, fn, bbList, numBB, numEx, callList):
  global expops
  for bb in bbList:
    if not bb.startswith("L_"+fn+"_"): continue
    os = "%s:%s:%d:%s" % (filename, fn, numBB[fn], bb)
    for i in expops:
      os += ":%s %d" % (i, numEx[bb][i])
    os += ":%s" % (",".join(callList[bb]))
    print(os)

def ll2dot(filename):
  print("digraph G {")
  with open(filename, "r") as f:
    data = f.read().splitlines()
  l = 0
  fnm = ""
  symtab = []
  lsymtab = []
  callList = {}
  inSwitch = False
  g = "" # 
  for d in data:
    l += 1
    flds = d.lstrip().split(' ')
    if len(flds) == 0: continue
    if inSwitch:
      if flds[0] == "]":
        inSwitch = False
      else:
        for i in range(len(flds)):
          if flds[i] == "label":
            print("%s -> %s;" % (curBB, clean(flds[i+1], fnm)))
            break
      continue
    # start of function
    if flds[0] == "define":
      for f in flds:
        if f[0] == '@':
          fnm = re.sub(p_subr0, "", f[1:])
          break
      if fnm not in symtab:
        symtab.append(fnm)
      print("subgraph cluster_%s {\n label=\"%s\";" % (fnm, fnm))
      curBB = "entry_" + fnm
      print("%s[style=filled fillcolor=\"yellow\"];" % (curBB))
      inSwitch = False
    # start of BB
    if len(flds[0]) and flds[0][-1] == ":":
      BB = flds[0][:-1]
      curBB = "L_" + fnm + "_" + BB
      inSwitch = False
    if flds[0] == ";" and flds[1] == "label":
      curBB = "L_" + fnm + "_" + flds[1][:-1]
      inSwitch = False
    # calls subroutine
    if len(flds) > 3 and "call" in flds:
      sr = None
      srf = flds.index("call")
      if flds[srf+1] == "fastcc": srf += 3
      else: srf += 2
      if flds[srf][0] == '%':
         sr = "FPDR_" + flds[srf][1:].split('(')[0]
      elif flds[srf][0] == '@':
         sr = flds[srf][1:].split('(')[0]
      else:
        print("%s:%d: FAILED TO FIND SR" % (filename, l), file=sys.stderr)
        return -1
      sr = sr.replace(".", "_")
      if sr in symtab:
        g += "%s -> entry_%s[color=\"red\"];\nexit_%s -> %s[color=\"blue\"];\n" % (curBB, sr, sr, curBB)
      else:
        lsym = curBB + "_" + sr
        if not lsym in lsymtab:
          lsymtab.append(lsym)
          g += "%s[label=\"%s\"];\n" % (lsym, sr)
        g += "%s -> %s[color=\"pink\",dir=both,minlen=0];\n" % (curBB, lsym)
    # BB terminators: ret, br, switch, indirectbr, invoke, callbr, resume, catchswitch, catchret, cleanupret, unreachable
    if flds[0] == "br":
      if flds[1] == "label":
        print("%s -> %s;" % (curBB, clean(flds[2], fnm)))
      elif flds[1] == "i1":
        print("%s -> %s;" % (curBB, clean(flds[4], fnm)))
        print("%s -> %s;" % (curBB, clean(flds[6], fnm)))
    if flds[0] == "ret":
        print("%s -> exit_%s;" % (curBB, fnm))
    if flds[0] == "switch":
      inSwitch = True
      for i in range(len(flds)):
        if flds[i] == "label":
          print("%s -> %s;" % (curBB, clean(flds[i+1], fnm)))
          break
      continue
    if flds[0] == "}":
       print("}")
  print(g + "}")
  return 0

if __name__ == "__main__":
  for f in sys.argv[1:]:
    ll2dot(f)


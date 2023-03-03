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
  lbl.replace("%", "").replace(",", "")
  return "L_" + nm + "_" + lbl

def pfuncstats(filename, fn, bbList, numBB, numEx, callList):
  global expops
  for bb in bbList:
    if not bb.startswith("L_"+fn+"_"): continue
    os = "%s:%s:%d:%s" % (filename, fn, numBB[fn], bb)
    for i in expops:
      os += ":%s %d" % (i, numEx[bb][i])
    os += ":%s" % (",".join(callList[bb]))
    print(os)

def ll2db(filename):
  global expops, Zexpops, p_subr0 #, p_subr1, p_subr2
  with open(filename, "r") as f:
    data = f.read().splitlines()
  l = 0
  fnm = ""
  symtab = []
  lsymtab = []
  bbList = []
  callList = {}
  numEx = {}
  numBB = {}
  inSwitch = False
  for d in data:
    l += 1
    flds = d.lstrip().split(' ')
    if len(flds) == 0: continue
    if inSwitch:
      if flds[0] == "]":
        inSwitch = False
      continue
    # start of function
    if flds[0] == "define":
      for f in flds:
        if f[0] == '@':
          fnm = re.sub(p_subr0, "", f[1:])
          break
      if fnm not in symtab:
        symtab.append(fnm)
      numBB[fnm] = 1
      curBB = "entry_" + fnm
      callList[curBB] = []
      numEx[curBB] = copy.deepcopy(Zexpops)
      inSwitch = False
    # start of BB
    if len(flds[0]) and flds[0][-1] == ":":
      BB = flds[0][:-1]
      curBB = "L_" + fnm + "_" + BB
      callList[curBB] = []
      numEx[curBB] = copy.deepcopy(Zexpops)
      numBB[fnm] += 1
      if curBB not in bbList: bbList.append(curBB)
      inSwitch = False
    if flds[0] == ";" and flds[1] == "label":
      curBB = "L_" + fnm + "_" + flds[1][:-1]
      callList[curBB] = []
      numEx[curBB] = copy.deepcopy(Zexpops)
      numBB[fnm] += 1
      if curBB not in bbList: bbList.append(curBB)
      inSwitch = False
    # expensive ops
    if len(flds) > 3 and flds[1] == "=" and flds[2] in expops:
      numEx[curBB][flds[2]] += 1
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
        callList[curBB].append(sr)
      else:
        lsym = curBB + "_" + sr
        if not lsym in lsymtab:
          lsymtab.append(lsym)
        callList[curBB].append(sr)
    # BB terminators: ret, br, switch, indirectbr, invoke, callbr, resume, catchswitch, catchret, cleanupret, unreachable
    if flds[0] == "br": continue
    if flds[0] == "ret": continue
    if flds[0] == "unreachable": continue
    if flds[0] == "switch":
      inSwitch = True
      continue
    if flds[0] in terminators:
      print("new terminator %s" % flds[0], file=sys.stderr)
    if flds[0] == "}":
       pfuncstats(filename, fnm, bbList, numBB, numEx, callList)
  return 0

if __name__ == "__main__":
  for f in sys.argv[1:]:
    ll2db(f)


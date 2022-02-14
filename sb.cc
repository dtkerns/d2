#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>
#include <map>

bool debug;

class Node {
  public:
  Node(std::string l) : label(l) { numIn = numOut = numCall = 0; size_t i = l.find_last_of('_'); lineno = atoi(l.substr(i+1).c_str());}
  Node(Node *np) { lineno = np->lineno; numIn = np->numIn; numOut = np->numOut; numCall=np->numCall; label = np->label; toList = np->toList; callList = np->callList; fprintf(stderr, "copy from ptr\n");}
  int lineno;
  int numIn;
  int numOut;
  int numCall;
  std::string label;
  std::vector<std::string> toList;
  std::vector<std::string> fmList;
  std::vector<std::string> callList;
};

class SB {
  public:
  SB(const std::string &n) : name(n) { }
  SB(const std::string &n, std::vector<Node> &l) : name(n), list(l) { }
  bool addNode(const Node &n);
  void addConstraint(const std::string s);
  bool addTolist(std::vector<Node> &fullList);
  bool isContained();
  const std::string name;
  std::vector<Node> list; 
  std::vector<std::string> constraints; // external function calls
};

std::vector<SB> sbv;

void ppsbv()
{
  for (int i = 0; i < sbv.size(); i++) {
    printf("digraph G {\n");
    printf("  compound=true;\n");
    printf("  subgraph cluster_%s_%d {\n", sbv[i].name.c_str(), i);
    printf("    label=\"%s_%d\";\n", sbv[i].name.c_str(), i);
    printf("    entry [style=invis];\n");
    printf("    exit [style=invis];\n");
    std::string firstLabel = sbv[i].list[0].label;
    printf("    entry -> %s\n", firstLabel.c_str());
    for (int e = 0; e < sbv[i].list.size(); e++) {
      for (int ne = 0; ne < sbv[i].list[e].toList.size(); ne++) {
        printf("    %s -> %s;\n", sbv[i].list[e].label.c_str(), sbv[i].list[e].toList[ne].c_str());
      }
    }
    printf("  }\n");
    for (int c = 0; c < sbv[i].constraints.size(); c++) {
      printf("  %s -> %s [ltail=cluster_%s_%d];\n", firstLabel.c_str(), sbv[i].constraints[c].c_str(), sbv[i].name.c_str(), i);
    }
    printf("}\n");
  }
}

void pptree(std::map<std::string, std::vector<Node> > &tree)
{
  printf("digraph G {\n");
  for (std::map<std::string, std::vector<Node> >::iterator it=tree.begin(); it!=tree.end(); ++it) {
    printf("subgraph cluster_%s {\n", /* // len %d\n", */ (it->first).c_str() /* , (int) (it->second).size() */);
    for (std::vector<Node>::iterator nit = (it->second).begin() ; nit != (it->second).end(); ++nit) {
      //printf("// %s %d (in %d out %d)\n", nit->label.c_str(), (int) nit->toList.size(), nit->numIn, nit->numOut);
      for (std::vector<std::string>::iterator lit = nit->toList.begin(); lit != nit->toList.end(); ++lit) {
        printf("%s -> %s;\n", nit->label.c_str(), lit->c_str());
      }
    }
    printf("}\n");
  }
  for (std::map<std::string, std::vector<Node> >::iterator it=tree.begin(); it!=tree.end(); ++it) {
    for (std::vector<Node>::iterator nit = (it->second).begin() ; nit != (it->second).end(); ++nit) {
      for (std::vector<std::string>::iterator lit = nit->callList.begin(); lit != nit->callList.end(); ++lit) {
        bool lib = false;
        if (nit->label.compare(0, nit->label.size(), *lit, 0, nit->label.size()) == 0) {
          lib = true;
        }
        printf("%s -> %s%s;\n", nit->label.c_str(), lit->c_str(), lib?"[dir=both,minlen=0]":"");
      }
    }
  }
  printf("}\n");
}

void ppnode(Node *n)
{
  printf("NODE-> %s (%d, %d)", n->label.c_str(), n->lineno, n->numIn);
  printf("\n\t(%d) ", n->numOut);
  for (size_t i = 0; i < n->toList.size(); ++i) printf("%s%s", i==0?"":",", n->toList[i].c_str());
  printf("\n\t(%d) ", n->numCall);
  for (size_t i = 0; i < n->callList.size(); ++i) printf("%s%s", i==0?"":",", n->callList[i].c_str());
  printf("\n");
}

Node *findNode(std::vector<Node> &list, std::string &label)
{
  for (std::vector<Node>::iterator it = list.begin() ; it != list.end(); ++it) {
    if (it->label.compare(label) == 0) {
//printf("] found %s (%s) @ %p\n", it->label.c_str(), label.c_str(), it);
      return &(*it);
    }
  }
  Node *node = new Node(label);
  list.push_back(*node);
  return findNode(list, label); // go find the new entry, and return it
}

bool listContainsLabel(const std::vector<std::string> list, const std::string label)
{
  for (int li = 0; li < list.size(); ++li) {
    if (label.compare(list[li]) == 0) return true;
  }
  return false;
}

bool SB::isContained()
{
  int numOutMiss = 0;
  int numInMiss = 0;
  for (int ni = 0; ni < this->list.size(); ++ni) {
    for (int li = 0; li < this->list[ni].toList.size(); ++li) {
      std::string needle = this->list[ni].toList[li];
      bool found = false;
      for (int ni2 = 0; ni2 < this->list.size(); ++ni2) {
        if (needle.compare(this->list[ni2].label) == 0) {
          found = true;
          break;
        }
      }
      if (!found) {
        numOutMiss++;
      }
    }
    for (int li = 0; li < this->list[ni].fmList.size(); ++li) {
      std::string needle = this->list[ni].fmList[li];
      if (needle.compare(0, 6, "entry_") == 0) continue;
      bool found = false;
      for (int ni2 = 0; ni2 < this->list.size(); ++ni2) {
        if (needle.compare(this->list[ni2].label) == 0) {
          found = true;
          break;
        }
      }
      if (!found) {
        numInMiss++;
      }
    }
  }
  if (numOutMiss < 2 && numInMiss < 2) {
    return true;
  }
  return false;
}

bool contains(const std::vector<std::string> &list, const std::string &name)
{
  bool found = false;
  for (int n = 0; n < list.size(); n++) {
    if (list[n].compare(name) == 0) {
      found = true;
      break;
    }
  }
  return found;
}

void addUniq(std::vector<std::string> &addList, const std::string &name)
{
  bool found = contains(addList, name);
  if (!found) {
    addList.push_back(name);
  }
}

bool SB::addTolist(std::vector<Node> &fullList)
{
  bool ret = false;
  int numAdd = 0;
  // create a list of nodes to add
  //printf("enter addTolist: %s %s (%d)\n", this->name.c_str(), this->list[0].label.c_str(), this->list.size());
  std::vector<std::string> addList;
  std::vector<std::string> inList;
  for (int n = 0; n < this->list.size(); n++) {
    inList.push_back(this->list[n].label);
  }
  for (int n = 0; n < this->list.size(); n++) {
    if (this->list[n].label.compare(0, 5, "exit_") == 0) continue;
    for (int m = 0; m < this->list[n].toList.size(); m++) {
      if (!contains(inList, this->list[n].toList[m])) {
        addUniq(addList, this->list[n].toList[m]);
      }
    }
  }
  //printf("add:"); for (int n = 0; n < addList.size(); n++) { printf(" %s", addList[n].c_str()); } printf("\n");
  // add the new nodes, if any are before top node flag
  Node *np = NULL;
  for (int n = 0; n < addList.size(); n++) {
    np = findNode(fullList, addList[n]);
fprintf(stderr, "%s: chk %s, %d <? %d\n", this->list[0].label.c_str(), np->label.c_str(), np->lineno, this->list[0].lineno);
    if (np->lineno < this->list[0].lineno) {
fprintf(stderr, "\t\t%s: dont add %s, %d < %d\n", this->list[0].label.c_str(), np->label.c_str(), np->lineno, this->list[0].lineno);
      ret = true;
      break;
    }
    if (!ret && addNode(*np)) {
fprintf(stderr, "\t\tadding %s\n", np->label.c_str());
      numAdd++;
    }
  }
  if (numAdd == 0) {
    ret = true;
  }
  return ret;
}

void SB::addConstraint(const std::string s) // add unique strings
{
  bool inList = false;
  for (int i = 0; i < this->constraints.size(); i++) {
    if (s.compare(constraints[i]) == 0) {
       inList = true;
       break;
    }
  }
  if (!inList) {
    constraints.push_back(s);
  }
}

bool SB::addNode(const Node &n) // add uniqe nodes
{
  bool inList = false;
  for (int i = 0; i < this->list.size(); i++) {
    if (n.label.compare(list[i].label) == 0) {
       inList = true;
       break;
    }
  }
  if (!inList) {
fprintf(stderr, "\t%s add %s\n", this->name.c_str(), n.label.c_str());
    this->list.push_back(n);
    for (int i = 0; i < n.callList.size(); i++) {
      std::string name;
      if (n.callList[i].compare(0, n.label.size(), n.label) == 0) {
        name = n.callList[i].substr(n.label.size()+1);
      } else {
        name = n.callList[i];
      }
//printf("addNode:%s: constraint -> %s -> %s\n", n.label.c_str(), n.callList[i].c_str(), name.c_str());
      addConstraint(name);
    }
  }
  return !inList; // return true if added
}

SB subEnd(SB &sb)
{
  SB n(sb);
  std::string firstLabel = n.list[0].label;
  std::string *potMiss;
  for (int ni = 0; ni < n.list.size(); ++ni) {
    for (int li = 0; li < n.list[ni].toList.size(); ++li) {
      std::string needle = n.list[ni].toList[li];
      bool found = false;
      for (int ni2 = 0; ni2 < n.list.size(); ++ni2) {
        if (needle.compare(n.list[ni2].label) == 0) {
          found = true;
          break;
        }
      }
      if (!found) {
fprintf(stderr, "change %s to exit\n", needle.c_str());
        n.list[ni].toList[li] = "exit";
        return n;
      }
    }
  }
  return n;
}

void startSB(const std::string fn, std::vector<Node> &fullList, const Node &n)
{
  SB sb(fn);
  sb.addNode(n);
  int lastNodeCount = sb.list.size();
  if (n.numIn == 1 && n.numOut == 1) {
    //printf("add SB for %s %d %d\n", fn.c_str(), sb.list[0].lineno, (int) sbv.size());
    sbv.push_back(subEnd(sb));
  }
  bool done = false;
  while (!done) {
    done = sb.addTolist(fullList);
    if (lastNodeCount == sb.list.size()) {
      done = true;
    }
    if (!done && sb.isContained()) {
      sbv.push_back(subEnd(sb));
      //printf("add SB for %s %d %d\n", fn.c_str(), sb.list[0].lineno, (int) sbv.size());
    }
    lastNodeCount = sb.list.size();
  }
}

void findSB(std::map<std::string, std::vector<Node> > &tree)
{
  for (std::map<std::string, std::vector<Node> >::iterator it=tree.begin(); it!=tree.end(); ++it) {
    for (std::vector<Node>::iterator nit = (it->second).begin() ; nit != (it->second).end(); ++nit) {
      if (nit->numOut != 1) continue;
      startSB(it->first, it->second, *nit);
    }
  }
}

std::map<std::string, std::vector<Node> > parse(FILE *fd)
{
  std::map<std::string, std::vector<Node> > tree;
  std::vector<Node> nodeList;
  Node *node;
  bool inSubgraph = false;
  char buf[256], from[128], to[128], rest[128], *bp, *p;
  std::string funcName;
  while (fgets(buf, sizeof(buf), fd) != NULL) {
  bp = buf;
  while (bp && *bp && (*bp == ' ' || *bp == '	')) bp++;
//puts(buf);
    if (inSubgraph) {
      if (strchr(bp, '>') != NULL) {
        if (sscanf(bp, "%s -> %[^[ ;]", from, to) == 2) {
          std::string flabel(from);
          std::string tlabel(to);
//printf("] find node for from %s\n", flabel.c_str());
          node = findNode(nodeList, flabel);
          node->numOut++;
//printf("] add %s to %s\n", tlabel.c_str(), flabel.c_str());
          node->toList.push_back(tlabel);
//printf("] node @ %p\n", node);
//printf("] check node @ %p\n", node=findNode(nodeList, flabel));
//ppnode(node);
//printf("] find node for to %s\n", tlabel.c_str());
          node = findNode(nodeList, tlabel);
          node->numIn++;
          node->fmList.push_back(flabel);
        } else {
          fprintf(stderr, "SG parse error in : %s", buf);
        }
        continue;
      } else if (*bp == '}') {
//printf("] add nodeList (%d) to %s\n", (int) nodeList.size(), funcName.c_str());
        tree[funcName] = nodeList;
        nodeList.clear();
        inSubgraph = false;
        continue;
      } else {
        if (debug) printf("skipping: %s", buf);
      }
    } else if (strncmp(bp, "subgraph cluster_", 17) == 0) { // subgraph cluster_GenerateKey { //}
      p = strchr(&bp[17], ' '); // 
      *p = 0;
      funcName.assign(&bp[17]);
      inSubgraph = true;
//printf("] enter subgraph %s\n", funcName.c_str());
      continue;
    } else if (strchr(bp, '>') != NULL) {
      int n;
      rest[0] = 0;
//printf("] %s", buf);
      if ((n = sscanf(bp, "%s -> %[^[ ;][%[^];]", from, to, rest)) > 1) {
        char fn[128];
        if (from[0] == 'L') {
          strcpy(fn, &from[2]);
          p = strrchr(fn, '_');
          *p = 0;
//printf("] >>>>>>>>>>>> %s -> %s %s %s\n", from, to, fn, rest);
        } else if (strncmp(from, "entry_", 6) == 0) {
          strcpy(fn, &from[6]);
        } else {
continue;
          n = sscanf(from, "exit_%s", fn);
        }
        funcName.assign(fn);
        std::string flabel(from);
        std::string tlabel(to);
        node = findNode(tree[funcName], flabel);
        node->numCall++;
        node->callList.push_back(tlabel);
      } else {
        fprintf(stderr, "parse error in(%d): %s", n, buf);
      }
      continue;
    } else {
      if (debug) printf("skipping: %s", buf);
    }
  }
  return tree;
}

int main(int argc, char **argv)
{
  if (argc < 1) {
    printf("usage: %s file.dot", argv[0]);
    return -1;
  }
  FILE *fd = fopen(argv[1], "r");
  if (fd == NULL) {
    printf("%s: failed to open %s for reading, errno %d\n", argv[0], argv[1], errno);
    return -2;
  }
  std::map<std::string, std::vector<Node> > tree = parse(fd);
  fclose(fd);
  pptree(tree);
  findSB(tree);
  ppsbv();
  return 0;
}

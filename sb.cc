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
  int lineno;
  int numIn;
  int numOut;
  int numCall;
  std::string label;
  std::vector<std::string> toList;
  std::vector<std::string> callList;
};

void ppnode(Node *n)
{
  printf("NODE-> %s (%d, %d)", n->label.c_str(), n->lineno, n->numIn);
  printf("\n\t(%d) ", n->numOut);
  for (size_t i = 0; i < n->toList.size(); ++i) printf("%s%s", i==0?"":",", n->toList[i].c_str());
  printf("\n\t(%d) ", n->numCall);
  for (size_t i = 0; i < n->callList.size(); ++i) printf("%s%s", i==0?"":",", n->callList[i].c_str());
  printf("\n");
}

class SB {
  public:
  SB(const std::string &n) : name(n) { }
  SB(const std::string &n, std::vector<Node> &l) : name(n), list(l) { }
  void addNode(const Node &n);
  void addConstraint(const std::string s);
  bool addTolist(const std::vector<Node> &fullList, int level);
  bool isContained(const std::vector<Node> &fullList);
  const std::string name;
  std::vector<Node> list; 
  std::vector<std::string> constraints; // external function calls
};

std::vector<SB> sbv;

void ppsbv() // TODO
{
}

bool listContainsLabel(const std::vector<std::string> list, const std::string label)
{
  for (int li = 0; li < list.size(); ++li) {
    if (label.compare(list[li]) == 0) return true;
  }
  return false;
}

bool SB::isContained(const std::vector<Node> &fullList) // TODO
{
  int numOutMiss = 0;
  for (int ni = 0; ni < this->list.size(); ++ni) {
    for (int li = 0; li < this->list[ni].toList.size(); ++li) {
      
    }
  }
  return false;
}

bool SB::addTolist(const std::vector<Node> &fullList, int level) // TODO
{
  return false;
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

void SB::addNode(const Node &n) // add uniqe nodes
{
  bool inList = false;
  for (int i = 0; i < this->list.size(); i++) {
    if (n.label.compare(list[i].label) == 0) {
       inList = true;
       break;
    }
  }
  if (!inList) {
    this->list.push_back(n);
    for (int i = 0; i < n.callList.size(); i++) {
      addConstraint(n.callList[i].substr(n.label.size()+1));
    }
  }
}

void startSB(const std::string fn, const std::vector<Node> &fullList, const Node &n)
{
  SB sb(fn);
  sb.addNode(n);
  if (n.numIn == 1) sbv.push_back(sb);
  bool done = false;
  int level = 0;
  while (!done) {
    done = sb.addTolist(fullList, level);
    if (sb.isContained(fullList)) sbv.push_back(sb);
    level++;
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

std::map<std::string, std::vector<Node> > parse(FILE *fd)
{
  std::map<std::string, std::vector<Node> > tree;
  std::vector<Node> nodeList;
  Node *node;
  bool inSubgraph = false;
  char buf[256], from[128], to[128], rest[128], *p;
  std::string funcName;
  while (fgets(buf, sizeof(buf), fd) != NULL) {
//puts(buf);
    if (inSubgraph) {
      if (strchr(buf, '>') != NULL) {
        if (sscanf(buf, "%s -> %[^[ ;]", from, to) == 2) {
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
        } else {
          fprintf(stderr, "SG parse error in : %s", buf);
        }
        continue;
      } else if (buf[0] == '}') {
//printf("] add nodeList (%d) to %s\n", (int) nodeList.size(), funcName.c_str());
        tree[funcName] = nodeList;
        nodeList.clear();
        inSubgraph = false;
        continue;
      } else {
        if (debug) printf("skipping: %s", buf);
      }
    } else if (strncmp(buf, "subgraph cluster_", 17) == 0) { // subgraph cluster_GenerateKey { //}
      p = strchr(&buf[17], ' '); // 
      *p = 0;
      funcName.assign(&buf[17]);
      inSubgraph = true;
//printf("] enter subgraph %s\n", funcName.c_str());
      continue;
    } else if (strchr(buf, '>') != NULL) {
      int n;
      rest[0] = 0;
//printf("] %s", buf);
      if ((n = sscanf(buf, "%s -> %[^[ ;][%[^];]", from, to, rest)) > 1) {
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
//printf("call pptree\n");
  pptree(tree);
  findSB(tree);
  ppsbv();
  return 0;
}

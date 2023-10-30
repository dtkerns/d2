#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

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

bool contains(const std::vector<std::string> &list, const std::string &name)
{
  bool found = false;
  for (int n = 0; n < list.size(); ++n) {
    if (name.compare(list[n]) == 0) {
      found = true;
      break;
    }
  }
  return found;
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

// only used while debugging
void ppnode(Node *n)
{
  printf("NODE-> %s (%d, %d)", n->label.c_str(), n->lineno, n->numIn);
  printf("\n\t(%d) ", n->numOut);
  for (size_t i = 0; i < n->toList.size(); ++i) printf("%s%s", i==0?"":",", n->toList[i].c_str());
  printf("\n\t(%d) ", n->numCall);
  for (size_t i = 0; i < n->callList.size(); ++i) printf("%s%s", i==0?"":",", n->callList[i].c_str());
  printf("\n");
}

// this will always find the node, even if it has to push a new one
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

void findBack(std::map<std::string, std::vector<Node> > &tree)
{
  for (std::map<std::string, std::vector<Node> >::iterator it=tree.begin(); it!=tree.end(); ++it) {
    printf("%s\n", (it->first).c_str());
    for (std::vector<Node>::iterator nit = (it->second).begin() ; nit != (it->second).end(); ++nit) {
      for (std::vector<std::string>::iterator lit = nit->toList.begin(); lit != nit->toList.end(); ++lit) {
        Node *np = findNode(it->second, *lit);
        if (np->lineno < nit->lineno) {
          printf(" %s %d -> %s %d\n", nit->label.c_str(), nit->lineno, np->label.c_str(), np->lineno);
        }
      }
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

#define UFMT "usage: %s [-s] [-b] file.dot\n"

int main(int argc, char **argv)
{
  int back = 0;
  int showInput = 0;
  int opt;
  while ((opt = getopt(argc, argv, "bs")) != -1) {
    switch (opt) {
    case 'b':
      back = 1;
      break;
    case 's':
      showInput = 1;
      break;
    default: /* '?' */
      fprintf(stderr, UFMT, argv[0]);
      exit(EXIT_FAILURE);
    }
  }
  if (optind >= argc) {
    fprintf(stderr, UFMT, argv[0]);
    return -1;
  }

  FILE *fd = fopen(argv[optind], "r");
  if (fd == NULL) {
    printf("%s: failed to open %s for reading, errno %d\n", argv[0], argv[1], errno);
    return -2;
  }
  std::map<std::string, std::vector<Node> > tree = parse(fd);
  fclose(fd);
  if (showInput) {
    pptree(tree);
  }
  findBack(tree);
  return 0;
}

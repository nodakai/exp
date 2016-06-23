#include <iostream>
#include <cstdio>
#include <memory>
#include <functional>

#include <libxml/parser.h>
#include <libxml/tree.h>

int main(int argc, char *argv[]) {
  LIBXML_TEST_VERSION

  std::unique_ptr<xmlDoc, void(*)(xmlDocPtr)> doc{ ::xmlParseFile(argv[1]), ::xmlFreeDoc };
  if ( ! doc) {
    std::cerr << "Could not read " << argv[1] << std::endl;
    return 1;
  }

  ::xmlDocDump(stdout, doc.get());

  return 0;
}

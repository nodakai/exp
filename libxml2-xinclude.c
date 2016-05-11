#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xinclude.h>

static void parse(xmlNodePtr n, int depth) {
    char tmp[1024];
    memset(tmp, ' ', depth);
    tmp[depth] = '\0';

    for (xmlAttrPtr a = n->properties; a; a = a->next) {
        if (a->type == XML_ATTRIBUTE_NODE) {
            xmlNodePtr valPtr = a->children;
            printf("%s[%s]=[%s]\n", tmp, a->name, valPtr->content);
        }
    }

    for (xmlNodePtr c = n->children; c; c = c->next) {
        if (c->type == XML_ELEMENT_NODE) {
            printf("%s[%s]\n", tmp, c->name);
            parse(c, depth + 1);
        }
    }
}

int main(int argc, char *argv[]) {

    LIBXML_TEST_VERSION

    {
        xmlDocPtr doc = xmlReadFile(argv[1], NULL, 0);
        if (doc) {
            int rc = xmlXIncludeProcess(doc);
            printf("xmlXIncludeProcess => %d\n", rc);
            if (rc < 0) {
                fprintf(stderr, "XInclude processing failed\n");
                exit(1);
            } else {
                xmlNodePtr root = xmlDocGetRootElement(doc);
                if (root) {
                    parse(root, 0);
                } else {
                    fprintf(stderr, "xmlDocGetRootElement failed\n");
                    exit(1);
                }
            }

            xmlFreeDoc(doc);
        } else {
            fprintf(stderr, "xmlReadFile failed\n");
            exit(1);
        }
    }

    xmlCleanupParser();
    printf("Bye\n");

    return 0;
}

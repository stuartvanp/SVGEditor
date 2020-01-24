/*Stuart van Pinxteren student #0973785*/

#include <stdio.h>
#include "SVGParser.h"
#include "LinkedListAPI.h"
#include <libxml/parser.h>
#include <libxml/tree.h>


xmlNode * openXml(char* fileName, xmlDoc * doc);
static void print_element_names(xmlNode * a_node);

void addAttributes( void (addToList)(void * myStructure, Attribute * toAdd), void * structure, xmlNode * node);
void addAttributesSVG(void * myStructure, Attribute *toAdd);

void addNameSpace(SVGimage * svg, xmlNode * root);
void addPaths(SVGimage * svg, xmlNode * node);

SVGimage* createSVGimage(char* fileName){
    xmlDoc * doc = NULL;

    xmlNode* root = NULL;

    LIBXML_TEST_VERSION

    doc = xmlReadFile(fileName, NULL, 0);
        if (doc == NULL) {
            return NULL;
        }
    root = xmlDocGetRootElement(doc);
    
    if (root == NULL){
        return NULL;
    }
    print_element_names(root);   
    SVGimage * svg = malloc(sizeof(SVGimage)); 
    addNameSpace(svg, root);

    svg->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);
 //   svg->circles = initializeList(circleToString, deleteCircle, compareCircles);
 //   svg->rectangles = initializeList(rectangleToString, deleteRectangle, compareRectangles);
 //   svg->groups = initializeList(rectangleToString, deleteRectangle, compareRectangles);
 //   svg->paths = initializeList(pathToString, deletePath, comparePaths);
 
    addAttributes(addAttributesSVG, svg, root);
    
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return svg;   
}



/*
This function takes an SVGimage struct and returns a string describing said struct

*/
char* SVGimageToString(SVGimage* img){
    if (img == NULL) {
        return NULL;
    }
    char * toAdd = NULL;
    char * string = malloc (sizeof(char) * 1000);
    
    sprintf(string, "SVG Image\nNameSpace: %s\nTitle: %s\nDescription: %s\nAttributes:", img->namespace, img->title, img->description);

    toAdd = toString(img->otherAttributes);   //gets a string describing the attributes and adds it to string
    string = realloc(string, strlen(string) + strlen(toAdd) + 100);
    strcat(string, toAdd);    
    free(toAdd);                     //frees string of attributes bc it was added to main string which was reallocated

    return string;
}

void deleteSVGimage(SVGimage* img){
    freeList(img->otherAttributes);
    free(img);

    return;
}


void deleteAttribute( void* data){
    Attribute * attrib = data;
    free(attrib->name);
    free(attrib->value);
    free(attrib);
    return;
}
char* attributeToString( void* data){
    Attribute * attrib = data;
    int length = 0;
    length =strlen("\tName: ") + strlen(attrib->name) + strlen("\n\tValue: ") + strlen(attrib->value) + strlen("\n") + 51;
    char * str = malloc(sizeof(char) * (length));
    sprintf(str, "\tName: %s\n\tValue: %s\n", attrib->name, attrib->value);
    return str;
}
int compareAttributes(const void *first, const void *second){
    return 0;
}

void deleteGroup(void* data){
    return;
}
char* groupToString( void* data){
    return "a";
}
int compareGroups(const void *first, const void *second){
    return 0;
}

void deleteRectangle(void* data){
    return;
}
char* rectangleToString(void* data){
    return "A";
}
int compareRectangles(const void *first, const void *second){
    return 0;
}

void deleteCircle(void* data){
    return;
}
char* circleToString(void* data){
    return "A";
}
int compareCircles(const void *first, const void *second){
    return 0;
}

void deletePath(void* data){
    return;
}
char* pathToString(void* data){
    return "A";
}
int comparePaths(const void *first, const void *second){
    return 0;
}


void addNameSpace(SVGimage * svg, xmlNode * root) {
    int noTitle = 0;
    int noDesc  = 0;
    strncpy(svg->namespace, (char *)root->ns->href, 255);

    for (xmlNode * mover = root->children; mover != NULL; mover = mover->next){
        if (strcmp((char *)mover->name, "title") == 0){
            if (mover->children->content != NULL){
                strncpy(svg->title, (char *)mover->children->content, 255);  
                noTitle = 1;
            }
        }
       if (strcmp((char *)mover->name, "desc") == 0){
            if (mover->children->content != NULL) {
                strncpy(svg->description, (char *)mover->children->content, 255);
                noDesc = 1;
            }
        }
    }
    if (noTitle == 0){
        strncpy(svg->title, "", 2);
    }
    if (noDesc == 0) {
        strncpy(svg->description, "", 2);
    }
    
}

//finds the attributes of a specific xml node, adds them too attributes list in SVG header
void addAttributes( void (addToList)(void * myStructure, Attribute * toAdd), void * structure, xmlNode * node){ 
    xmlAttr * xmlattribute = NULL;
    Attribute * newAtt = NULL;   
    for (xmlattribute = node->properties; xmlattribute != NULL; xmlattribute = xmlattribute->next ){ //iterates through the nodes attributes
        newAtt = NULL;
        newAtt = malloc(sizeof(Attribute));  //creates struct
        newAtt->name = malloc(sizeof(char) * (strlen((char *)xmlattribute->name) + 1));
        newAtt->name = strcpy(newAtt->name, (char *)xmlattribute->name); //copies name over

        newAtt->value = malloc(sizeof(char) * (strlen((char *)xmlattribute->children->content) + 1));
        newAtt->value = strcpy(newAtt->value, (char *)xmlattribute->children->content); //copies value over
        
        addToList(structure, newAtt);
        //insertBack(svg->otherAttributes, newAtt); //inserts struct into svg struct list
    }
}

void addAttributesSVG(void * myStructure, Attribute * toAdd){
    SVGimage * svg = myStructure;
    insertBack(svg->otherAttributes, toAdd );
    return;
}

/* void addPaths(SVGimage * svg, xmlNode * node) {
    for (xmlNode * mover = node->children;)

    
} */

static void print_element_names(xmlNode * a_node) {
    xmlNode *cur_node = NULL;

    for (cur_node = a_node; cur_node != NULL; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            printf("node type: Element, name: %s\n", cur_node->name);
        }

        // Uncomment the code below if you want to see the content of every node.

        if (cur_node->content != NULL ){
            printf("  content: %s\n", cur_node->content);
        }

        // Iterate through every attribute of the current node
        xmlAttr *attr;
        for (attr = cur_node->properties; attr != NULL; attr = attr->next)
        {
            xmlNode *value = attr->children;
            char *attrName = (char *)attr->name;
            char *cont = (char *)(value->content);
            printf("\tattribute name: %s, attribute value = %s\n", attrName, cont);
        }

        print_element_names(cur_node->children);
    }
}

int main (int argc, char **argv) {
    SVGimage * svg = createSVGimage(argv[1]);
    if (svg == NULL) {
        printf("invalid SVG");

    }
    char * string = SVGimageToString(svg);
    printf("%s\n", string);
    free(string);
    deleteSVGimage(svg);

    return 0;
}
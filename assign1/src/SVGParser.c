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
            printf("COULD NOT OPEN0000000000000000000000000000000");
            xmlFreeDoc(doc);
            xmlCleanupParser();
            return NULL;
        }
    root = xmlDocGetRootElement(doc);
    
    if (root == NULL){
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }
    //print_element_names(root);   
    SVGimage * svg = malloc(sizeof(SVGimage)); 
    addNameSpace(svg, root);

    svg->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);
 //   svg->circles = initializeList(circleToString, deleteCircle, compareCircles);
 //   svg->rectangles = initializeList(rectangleToString, deleteRectangle, compareRectangles);
 //   svg->groups = initializeList(rectangleToString, deleteRectangle, compareRectangles);
    svg->paths = initializeList(pathToString, deletePath, comparePaths);
 
    addAttributes(addAttributesSVG, svg, root);
    addPaths(svg, root);
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
    free(toAdd);                    //frees string of attributes bc it was added to main string which was reallocated

    
    toAdd = toString(img->paths);       //gets a string describing paths and adds it  
    string = realloc(string, sizeof(char) * (strlen(string) + strlen(toAdd) + 100));
    strcat(string, toAdd);
    free(toAdd);                    //frees OG path string
    
    return string;
}

void deleteSVGimage(SVGimage* img){
    if (img == NULL) {
        return;
    }
    freeList(img->otherAttributes);
    freeList(img->paths);
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
    Path * pth = data;
    
    freeList(pth->otherAttributes);
    free(pth->data);
    free(pth);
    return;
}


char* pathToString(void* data){
    Path * pth = data;
    char * toAdd = NULL;
    char * string = NULL;

    int length = strlen("Path:\n\tAttributes:\n\tData: ") + strlen(pth->data) + 50;
    string = malloc(sizeof(char) * length);
    sprintf(string, "Path:\n\tData: %s\n\n\tAttributes:\n", pth->data);
    toAdd = toString(pth->otherAttributes);
    string = realloc(string, sizeof(char) * (strlen(string) + strlen(toAdd) + 50));
    strcat(string, toAdd);
    free(toAdd);

    return string;
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
//function pointer is for what kind of structure the attributes are being added to, these functions are below
void addAttributes( void (addToList)(void * myStructure, Attribute * toAdd), void * structure, xmlNode * node){ 
    xmlAttr * xmlattribute = NULL;
    Attribute * newAtt = NULL;   
    for (xmlattribute = node->properties; xmlattribute != NULL; xmlattribute = xmlattribute->next ){ //iterates through the nodes attributes
        newAtt = NULL;
        newAtt = malloc(sizeof(Attribute));  //creates struct
        newAtt->name = malloc(sizeof(char) * (strlen((char *)xmlattribute->name) + 1));
        strcpy(newAtt->name, (char *)xmlattribute->name); //copies name over

        newAtt->value = malloc(sizeof(char) * (strlen((char *)xmlattribute->children->content) + 1));
        strcpy(newAtt->value, (char *)xmlattribute->children->content); //copies value over
        
        addToList(structure, newAtt);
        //insertBack(svg->otherAttributes, newAtt); //inserts struct into svg struct list
    }
}

void addAttributesSVG(void * myStructure, Attribute * toAdd){
    SVGimage * svg = myStructure;
    insertBack(svg->otherAttributes, toAdd );
    return;
}

void addPaths(SVGimage * svg, xmlNode * node) {
    Path * addPath;
    Attribute * otherAtt;
    
    for (xmlNode * mover = node->children; mover != NULL; mover = mover->next){ //iterates through the elements of xmltree 
        addPath = NULL;
        
        if (strcmp((char*)mover->name, "path") == 0) { //if a path is found
            addPath = malloc(sizeof(Path));                  //creates struct
            addPath->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);         //initializes list of otherAtttributes

            for (xmlAttr * attrib = mover->properties; attrib != NULL; attrib = attrib->next ){ //iterates through the attributes
            
                if (strcmp((char*)attrib->name, "d") == 0){                 //d is the only important attribute
                    addPath->data = malloc(sizeof(char) * (strlen((char *)attrib->children->content) + 1));  //malllocs memory for data
                    strcpy(addPath->data, (char *) attrib->children->content);   //copies in data
                }
                else{  //handling for otherAttributes
                    otherAtt = malloc(sizeof(Attribute));    //malloc attribute struct
                    otherAtt->name = malloc(sizeof(char) * (strlen((char *) attrib->name) + 1)); //malloc and copy attrib name
                    strcpy(otherAtt->name, (char *)attrib->name );
                    otherAtt->value = malloc (sizeof(char) * (strlen((char *) attrib->children->content) + 1));  //malloc and copy attrib value
                    strcpy(otherAtt->value, (char *) attrib->children->content);
                    insertBack(addPath->otherAttributes, otherAtt);  //insert attribut struct into list otherattributes of path struct
                } 
            }
            insertBack(svg->paths, addPath);
        }
    }
} 
/* 
void addCircles(SVGimage * svg, xmlNode * node) {
    for (xmlNode * mover = node; mover != NULL; mover = node->)
}
 */
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
        //return 0;
    }

    char * string = SVGimageToString(svg);
  //  printf("%s\n", string);
  //  free(string);

    deleteSVGimage(svg);


    return 0;
}
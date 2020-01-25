/*Stuart van Pinxteren student #0973785*/

#include <stdio.h>
#include "SVGParser.h"
#include "LinkedListAPI.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <ctype.h>


xmlNode * openXml(char* fileName, xmlDoc * doc);
static void print_element_names(xmlNode * a_node);

void addAttributes( void (addToList)(void * myStructure, Attribute * toAdd), void * structure, xmlNode * node);
void addAttributesSVG(void * myStructure, Attribute *toAdd);
//void addCircles(SVGimage * svg, xmlNode * node);
void addRectangles(SVGimage * svg, xmlNode * node);

float getValue(xmlChar * valStr);
void getUnits(xmlChar * valStr, char * toCopy);

void addNameSpace(SVGimage * svg, xmlNode * root);
void addPaths(SVGimage * svg, xmlNode * node);

SVGimage* createSVGimage(char* fileName){
    xmlDoc * doc = NULL;

    xmlNode* root = NULL;

    LIBXML_TEST_VERSION

    doc = xmlReadFile(fileName, NULL, 0);
        if (doc == NULL) {
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
    svg->rectangles = initializeList(rectangleToString, deleteRectangle, compareRectangles);
 //   svg->groups = initializeList(rectangleToString, deleteRectangle, compareRectangles);
    svg->paths = initializeList(pathToString, deletePath, comparePaths);
 
    addAttributes(addAttributesSVG, svg, root);
    addPaths(svg, root);
    //addCircles(svg, root);
    addRectangles(svg, root);
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
    string = realloc(string, strlen(string) + strlen(toAdd) + 50);
    strcat(string, toAdd);    
    free(toAdd);                    //frees string of attributes bc it was added to main string which was reallocated

    
    toAdd = toString(img->paths);       //gets a string describing paths and adds it  
    string = realloc(string, sizeof(char) * (strlen(string) + strlen(toAdd) + 50));
    strcat(string, toAdd);
    free(toAdd);                    //frees OG path string

    toAdd = toString(img->rectangles);
    string = realloc(string, sizeof(char) * (strlen(string) + strlen(toAdd) + 50));
    strcat(string, toAdd);
    free(toAdd); 

    
    return string;
}


//frees svg image and all necessary other data
void deleteSVGimage(SVGimage* img){
    if (img == NULL) {
        return;
    }
    freeList(img->otherAttributes);
    freeList(img->paths);
    freeList(img->rectangles);
    free(img);

    return;
}

//helper function to delete attribute structs form linked list
void deleteAttribute( void* data){
    Attribute * attrib = data;
    free(attrib->name);
    free(attrib->value);
    free(attrib);
    return;
}
//returns a string describing attributes
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
// function adds rectangles to svg file, 
void addRectangles(SVGimage * svg, xmlNode * node) {
    Rectangle * rect = NULL;
    
    for (xmlNode * mover = node->children; mover != NULL; mover = mover->next ) { //iterates through all the pieces of svg
        rect = NULL;
        if (strcmp((char*)mover->name, "rect") == 0) {  //finds rectangle and mallocs
            rect = malloc(sizeof(Rectangle));
            rect->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);
            
            for (xmlAttr * attrib = mover->properties; attrib != NULL; attrib = attrib->next) {  //iterates through rectangle attributes
                Attribute * otherAtt = NULL;

                if (strcmp((char *)attrib->name, "y") == 0 ){  //finds y coordinate and saves it
                    rect->y = getValue(attrib->children->content);
                }
            
                else if (strcmp((char *)attrib->name, "x") == 0 ){ //finds x coordinate and saves it
                    rect->x = getValue(attrib->children->content);
                    strcpy(rect->units, "");
                    getUnits(attrib->children->content, rect->units);  //gets units, if ther are any, otherwise copies in a blank string
                }
            
                else if (strcmp((char *)attrib->name, "width") == 0 ){ //finds width and saves it
                    rect->width = getValue(attrib->children->content);
                    if (rect->width < 0) {    //negative width is illegal
                        rect->width = rect->width * -1;
                    }
                }
            
                else if (strcmp((char *)attrib->name, "height") == 0 ){ //finds width and saves it
                    rect->height = getValue(attrib->children->content); //negative height is illegal
                    if (rect->height < 0) {
                        rect->height = rect->height * -1;
                    }
                }
                else{  //adds otherattributes to list
                    otherAtt = malloc(sizeof(Attribute));
                    otherAtt->name = malloc(sizeof(char) * (strlen((char *) attrib->name) + 1));  //mallocs and copies name
                    strcpy(otherAtt->name, (char *)attrib->name);
                    otherAtt->value = malloc (sizeof(char) * (strlen((char*) attrib->children->content) + 1));  //mallocs and copies value
                    strcpy(otherAtt->value, (char *)attrib->children->content);
                    insertBack(rect->otherAttributes, otherAtt);   //insert struct into attributes list
                }
            }
            insertBack(svg->rectangles, rect);
        }
    }

}
void deleteRectangle(void* data){
    Rectangle * rect = data;
    freeList(rect->otherAttributes);
    free(rect);
    return;
}

float getValue(xmlChar * valStr){
    return atof((char*)valStr);
}
//updates to copy with a version of valStr that is only letter chars
void getUnits(xmlChar * valStr, char * toCopy) {
    int j = 0;
    char string[50];
    char units[50];
    strcpy(units, "");
    strcpy(string, (char *)valStr);
    for (int i = 0; i < strlen(string); i ++){
        if (isalpha(string[i])) {
            units[j] = string[i];
            j++;
        }
    }
    units[j] = '\0';
    strcpy(toCopy, units);
}

char* rectangleToString(void* data){
    Rectangle * rect = data;
    char * string = NULL;
    char * toAdd = NULL;
    int length = 300;
    string = malloc(sizeof(char) * length);
    sprintf(string, "Rectangle:\n\tX: %lf\n\n\tY: %lf\n\n\tWidth: %lf\n\n\tHeight: %lf\n\n\tUnits: %s\n\n\tOther Attributes:\n", rect->x, rect->y, rect->width, rect->height, rect->units);
    toAdd = toString(rect->otherAttributes);
    string = realloc (string, sizeof(char) * (strlen(string) + strlen(toAdd) + 1));
    strcat(string, toAdd);
    free(toAdd);
    return string;
}
int compareRectangles(const void *first, const void *second){
    return 0;
}

/* void addCircles(SVGimage * svg, xmlNode * node) {
    Circle * circ = NULL;
    for (xmlNode * mover = node->children; mover != NULL; mover = mover->next) {
        if (strcmp((char *) mover->name, "circle") == 0) {
            printf("FOUND CIRCLE %s", mover->name);

        }

    }

} */

void deleteCircle(void* data){
    return;
}
char* circleToString(void* data){
    return "A";
}
int compareCircles(const void *first, const void *second){
    return 0;
}

//helper function for deleting Path structs from linked list
void deletePath(void* data){
    Path * pth = data;
    
    freeList(pth->otherAttributes);
    free(pth->data);
    free(pth);
    return;
}
//returns a string describing the path and its attributes
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

// adds the title, desc, and namespace to svg struct, not to be used on its own.
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
    if (string != NULL) {
        printf("%s\n", string);
        free(string);
    }

   deleteSVGimage(svg);


    return 0;
}
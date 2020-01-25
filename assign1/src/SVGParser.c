/*Stuart van Pinxteren student #0973785*/

#include <stdio.h>
#include "SVGParser.h"
#include "LinkedListAPI.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <ctype.h>



void addAttributes(SVGimage * svg, xmlNode * node);

void addCircles(Group * grp, SVGimage * svg, xmlNode * node);
void addRectangles(SVGimage * svg, xmlNode * node);
void AddGroupsSVG(SVGimage * svg, xmlNode * node);

void addCirclesToGroup(Group * grp, xmlNode * node);
void addRectanglesToGroup(Group * grp, xmlNode * node);
void AddGroupsToGroup(Group * mygrp, xmlNode * node);

float getValue(xmlChar * valStr);
void getUnits(xmlChar * valStr, char * toCopy);

void addNameSpace(SVGimage * svg, xmlNode * root);
void addPaths(Group * grp, SVGimage * svg, xmlNode * node);

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
    SVGimage * svg = malloc(sizeof(SVGimage)); 
    addNameSpace(svg, root);

    svg->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);
    svg->circles = initializeList(circleToString, deleteCircle, compareCircles);
    svg->rectangles = initializeList(rectangleToString, deleteRectangle, compareRectangles);
    svg->groups = initializeList(groupToString, deleteGroup, compareGroups);
    svg->paths = initializeList(pathToString, deletePath, comparePaths);
 
    addAttributes(svg, root);
    addPaths(NULL, svg, root);
    addCircles(NULL, svg, root);
    addRectangles(svg, root);
    AddGroupsSVG(svg, root);
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
    
    sprintf(string, "\nSVG Image\nNameSpace: %s\nTitle: %s\nDescription: %s\nAttributes:", img->namespace, img->title, img->description);

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

    toAdd = toString(img->circles);
    string = realloc(string, sizeof(char) * (strlen(string) + strlen(toAdd) + 50));
    strcat(string, toAdd);
    free(toAdd);

    toAdd = toString(img->groups);
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
    freeList(img->circles);
    freeList(img->groups);
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

//stub function
int compareAttributes(const void *first, const void *second){
    return 0;
}

void deleteGroup(void* data){
    Group * grp = data;
    freeList(grp->otherAttributes);
    freeList(grp->paths);
    freeList(grp->circles);
    freeList(grp->rectangles);
    freeList(grp->groups);
    free(grp);
    return;
}
char* groupToString( void* data){
    Group * grp = data;
    char * string = malloc(sizeof(char) * 300);
    char * toAdd = NULL;
    sprintf(string, "\nGroup: <open>\n\n\tAttributes:\n");

    toAdd = toString(grp->otherAttributes);
    string = realloc(string, sizeof(char) * (strlen(string) + strlen(toAdd) + 50));
    strcat(string, toAdd);
    free(toAdd);

    toAdd = toString(grp->paths);
    string = realloc(string, sizeof(char) * (strlen(string) + strlen(toAdd) + 50));
    strcat(string, toAdd);
    free(toAdd);

    toAdd = toString(grp->circles);
    string = realloc(string, sizeof(char) * (strlen(string) + strlen(toAdd) + 50));
    strcat(string, toAdd);
    free(toAdd);

    toAdd = toString(grp->rectangles);
    string = realloc(string, sizeof(char) * (strlen(string) + strlen(toAdd) + 50));
    strcat(string, toAdd);
    free(toAdd);

    toAdd = toString(grp->groups);
    string = realloc(string, sizeof(char) * (strlen(string) + strlen(toAdd) + 50));
    strcat(string, toAdd);
    free(toAdd);

    strcat(string, "Group: <close>\n");
    return string;
}

//stub function for LLAPi
int compareGroups(const void *first, const void *second){
    return 0;
}

void AddGroupsSVG(SVGimage * svg, xmlNode * node){
    Group * grp = NULL;

    for (xmlNode *mover = node ->children; mover != NULL; mover = mover->next) {
        grp = NULL;
        if (strcmp((char*)mover->name, "g") == 0){
            grp = malloc(sizeof(Group));
            grp->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);
            grp->paths = initializeList(pathToString, deletePath, comparePaths);
            grp->circles = initializeList(circleToString, deleteCircle, compareCircles);
            grp->rectangles = initializeList(rectangleToString, deleteRectangle, compareRectangles);
            grp->groups = initializeList(groupToString, deleteGroup, compareGroups);
            
            for (xmlAttr * attrib = mover->properties; attrib != NULL; attrib = attrib->next) {
                Attribute * otherAtt = NULL;
                otherAtt = malloc(sizeof(Attribute));
                otherAtt->name = malloc(sizeof(char) * (strlen((char *)attrib->name) + 1));
                strcpy(otherAtt->name, (char *)attrib->name);
                otherAtt->value = malloc (sizeof(char) * (strlen((char *) attrib->children->content) + 1));
                strcpy(otherAtt->value, (char*)attrib->children->content);
                insertBack(grp->otherAttributes, otherAtt);
            }
            addPaths(grp, NULL, mover);
            addCircles(grp, NULL, mover);
            addRectanglesToGroup(grp, mover);
            AddGroupsToGroup(grp, mover);

            insertBack(svg->groups, grp);
        }
    }
}

void AddGroupsToGroup(Group * mygrp, xmlNode * node){
    Group * grp = NULL;

    for (xmlNode *mover = node ->children; mover != NULL; mover = mover->next) {
        grp = NULL;
        if (strcmp((char*)mover->name, "g") == 0){
            grp = malloc(sizeof(Group));
            grp->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);
            grp->paths = initializeList(pathToString, deletePath, comparePaths);
            grp->circles = initializeList(circleToString, deleteCircle, compareCircles);
            grp->rectangles = initializeList(rectangleToString, deleteRectangle, compareRectangles);
            grp->groups = initializeList(groupToString, deleteGroup, compareGroups);

            
            for (xmlAttr * attrib = mover->properties; attrib != NULL; attrib = attrib->next) {
                Attribute * otherAtt = NULL;
                otherAtt = malloc(sizeof(Attribute));
                otherAtt->name = malloc(sizeof(char) * (strlen((char *)attrib->name) + 1));
                strcpy(otherAtt->name, (char *)attrib->name);
                otherAtt->value = malloc (sizeof(char) * (strlen((char *) attrib->children->content) + 1));
                strcpy(otherAtt->value, (char*)attrib->children->content);
                insertBack(grp->otherAttributes, otherAtt);
            }
            addPaths(grp, NULL, mover);
            addCircles(grp, NULL, mover);
            addRectanglesToGroup(grp, mover);
            AddGroupsToGroup(grp, mover);

            insertBack(mygrp->groups, grp);
        }
    }
}

void addCirclesToGroup(Group * grp, xmlNode * node) {
    Circle * circ = NULL;
    Attribute * otherAtt = NULL; 
             
    for (xmlNode * mover = node->children; mover != NULL; mover = mover->next) { //moves through the SVG elements
        circ = NULL;

        if (strcmp((char *) mover->name, "circle") == 0) {  //finds a circle, mallocs
            
            circ = malloc(sizeof(Circle));
            circ->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);
            strcpy(circ->units, "");
            
            for (xmlAttr * attrib = mover->properties; attrib != NULL; attrib = attrib->next ){  //moves throught the circles attributes
                otherAtt = NULL;
            
                if (strcmp((char *)attrib->name, "cx") == 0) {   //finds the cx value
                    circ->cx = getValue(attrib->children->content);
                    if (strlen(circ->units) == 0){
                        getUnits(attrib->children->content, circ->units);  //gets units
                    }
                }
                else if (strcmp((char*)attrib->name, "cy") == 0){  //finds the cy value
                    circ->cy = getValue(attrib->children->content);
                    if (strlen(circ->units) == 0){
                        getUnits(attrib->children->content, circ->units);  //gets units
                    }
                }
                else if (strcmp((char*)attrib->name, "r") == 0){   //finds the r value
                    circ->r = getValue(attrib->children->content);
                    if (strlen(circ->units) == 0){
                        getUnits(attrib->children->content, circ->units);  //gets units
                    }
                }
                else{
                    otherAtt = malloc(sizeof(Attribute));       //handles other attributes
                    otherAtt->name = malloc(sizeof(char) * (strlen((char *) attrib->name) + 1));  //malloc and copies names
                    strcpy(otherAtt->name, (char*)attrib->name);
                    otherAtt->value = malloc(sizeof(char) * (strlen((char *)attrib->children->content) + 1)); //malloc and copies value
                    strcpy(otherAtt->value, (char*) attrib->children->content);
                    insertBack(circ->otherAttributes, otherAtt);   //puts em in a mf list
                }
            }
            insertBack(grp->circles, circ); //inserts circle into list
        }

    }
} 


void addRectanglesToGroup(Group * grp, xmlNode * node) {
    Rectangle * rect = NULL;
    
    for (xmlNode * mover = node->children; mover != NULL; mover = mover->next ) { //iterates through all the pieces of svg
        rect = NULL;
        if (strcmp((char*)mover->name, "rect") == 0) {  //finds rectangle and mallocs
            rect = malloc(sizeof(Rectangle));
            rect->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);
            strcpy(rect->units, "");
            
            for (xmlAttr * attrib = mover->properties; attrib != NULL; attrib = attrib->next) {  //iterates through rectangle attributes
                Attribute * otherAtt = NULL;

                if (strcmp((char *)attrib->name, "y") == 0 ){  //finds y coordinate and saves it
                    rect->y = getValue(attrib->children->content);
                    if (strlen(rect->units) == 0){
                        getUnits(attrib->children->content, rect->units);  //gets units, if ther are any, otherwise copies in a blank string
                    }                    
                }
            
                else if (strcmp((char *)attrib->name, "x") == 0 ){ //finds x coordinate and saves it
                    rect->x = getValue(attrib->children->content);
                    strcpy(rect->units, "");
                    if (strlen(rect->units) == 0){
                        getUnits(attrib->children->content, rect->units);  //gets units, if ther are any, otherwise copies in a blank string
                    }
                }
            
                else if (strcmp((char *)attrib->name, "width") == 0 ){ //finds width and saves it
                    rect->width = getValue(attrib->children->content);
                    if (rect->width < 0) {    //negative width is illegal
                        rect->width = rect->width * -1;
                    }
                    if (strlen(rect->units) == 0){
                        getUnits(attrib->children->content, rect->units);  //gets units, if ther are any, otherwise copies in a blank string
                    }
                }
            
                else if (strcmp((char *)attrib->name, "height") == 0 ){ //finds width and saves it
                    rect->height = getValue(attrib->children->content); //negative height is illegal
                    if (rect->height < 0) {
                        rect->height = rect->height * -1;
                    }
                    if (strlen(rect->units) == 0){
                        getUnits(attrib->children->content, rect->units);  //gets units, if ther are any, otherwise copies in a blank string
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
            insertBack(grp->rectangles, rect);
        }
    }

}
// function adds rectangles to svg file, 
void addRectangles(SVGimage * svg, xmlNode * node) {
    Rectangle * rect = NULL;
    
    for (xmlNode * mover = node->children; mover != NULL; mover = mover->next ) { //iterates through all the pieces of svg
        rect = NULL;
        if (strcmp((char*)mover->name, "rect") == 0) {  //finds rectangle and mallocs
            rect = malloc(sizeof(Rectangle));
            rect->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);
            strcpy(rect->units, "");
            
            for (xmlAttr * attrib = mover->properties; attrib != NULL; attrib = attrib->next) {  //iterates through rectangle attributes
                Attribute * otherAtt = NULL;

                if (strcmp((char *)attrib->name, "y") == 0 ){  //finds y coordinate and saves it
                    rect->y = getValue(attrib->children->content);
                    if (strlen(rect->units) == 0){
                        getUnits(attrib->children->content, rect->units);  //gets units, if ther are any, otherwise copies in a blank string
                    }                    
                }
            
                else if (strcmp((char *)attrib->name, "x") == 0 ){ //finds x coordinate and saves it
                    rect->x = getValue(attrib->children->content);
                    strcpy(rect->units, "");
                    if (strlen(rect->units) == 0){
                        getUnits(attrib->children->content, rect->units);  //gets units, if ther are any, otherwise copies in a blank string
                    }
                }
            
                else if (strcmp((char *)attrib->name, "width") == 0 ){ //finds width and saves it
                    rect->width = getValue(attrib->children->content);
                    if (rect->width < 0) {    //negative width is illegal
                        rect->width = rect->width * -1;
                    }
                    if (strlen(rect->units) == 0){
                        getUnits(attrib->children->content, rect->units);  //gets units, if ther are any, otherwise copies in a blank string
                    }
                }
            
                else if (strcmp((char *)attrib->name, "height") == 0 ){ //finds width and saves it
                    rect->height = getValue(attrib->children->content); //negative height is illegal
                    if (rect->height < 0) {
                        rect->height = rect->height * -1;
                    }
                    if (strlen(rect->units) == 0){
                        getUnits(attrib->children->content, rect->units);  //gets units, if ther are any, otherwise copies in a blank string
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
//function to free a rectangle struct
void deleteRectangle(void* data){
    Rectangle * rect = data;
    freeList(rect->otherAttributes);
    free(rect);
    return;
}
//gets the numerical value out of a string
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
        if (isalpha(string[i]) || string[i] == '%') {
            units[j] = string[i];
            j++;
        }
    }
    units[j] = '\0';
    strcpy(toCopy, units);
}
//returns a string describing recatangle data
char* rectangleToString(void* data){
    Rectangle * rect = data;
    char * string = NULL;
    char * toAdd = NULL;
    int length = 300;
    string = malloc(sizeof(char) * length);
    sprintf(string, "Rectangle:\n\tX: %.4lf\n\n\tY: %.4lf\n\n\tWidth: %.4lf\n\n\tHeight: %.4lf\n\n\tUnits: %s\n\n\tOther Attributes:\n", rect->x, rect->y, rect->width, rect->height, rect->units);
    toAdd = toString(rect->otherAttributes);
    string = realloc (string, sizeof(char) * (strlen(string) + strlen(toAdd) + 1));
    strcat(string, toAdd);
    free(toAdd);
    return string;
}
//stub for linked list api
int compareRectangles(const void *first, const void *second){
    return 0;
}
//adds circles to svg struct
void addCircles(Group * grp, SVGimage * svg, xmlNode * node) {
    Circle * circ = NULL;
    Attribute * otherAtt = NULL; 
             
    for (xmlNode * mover = node->children; mover != NULL; mover = mover->next) { //moves through the SVG elements
        circ = NULL;

        if (strcmp((char *) mover->name, "circle") == 0) {  //finds a circle, mallocs
            
            circ = malloc(sizeof(Circle));
            circ->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);
            strcpy(circ->units, "");
            
            for (xmlAttr * attrib = mover->properties; attrib != NULL; attrib = attrib->next ){  //moves throught the circles attributes
                otherAtt = NULL;
            
                if (strcmp((char *)attrib->name, "cx") == 0) {   //finds the cx value
                    circ->cx = getValue(attrib->children->content);
                    if (strlen(circ->units) == 0){
                        getUnits(attrib->children->content, circ->units);  //gets units
                    }
                }
                else if (strcmp((char*)attrib->name, "cy") == 0){  //finds the cy value
                    circ->cy = getValue(attrib->children->content);
                    if (strlen(circ->units) == 0){
                        getUnits(attrib->children->content, circ->units);  //gets units
                    }
                }
                else if (strcmp((char*)attrib->name, "r") == 0){   //finds the r value
                    circ->r = getValue(attrib->children->content);
                    if (strlen(circ->units) == 0){
                        getUnits(attrib->children->content, circ->units);  //gets units
                    }
                }
                else{
                    otherAtt = malloc(sizeof(Attribute));       //handles other attributes
                    otherAtt->name = malloc(sizeof(char) * (strlen((char *) attrib->name) + 1));  //malloc and copies names
                    strcpy(otherAtt->name, (char*)attrib->name);
                    otherAtt->value = malloc(sizeof(char) * (strlen((char *)attrib->children->content) + 1)); //malloc and copies value
                    strcpy(otherAtt->value, (char*) attrib->children->content);
                    insertBack(circ->otherAttributes, otherAtt);   //puts em in a mf list
                }
            }
            if (svg == NULL){
                insertBack(grp->circles, circ); //inserts circle into list
            }
            else if (grp == NULL) {
                insertBack(svg->circles, circ);
            }
        }


    }
} 
//frees a circle struct
void deleteCircle(void* data){
    Circle * circ = data;
    freeList(circ->otherAttributes);
    free(circ);

    return;
}
//returns a string describing a circle struct
char* circleToString(void* data){
    Circle * circ = data;
    char * string = NULL;
    char * toAdd = NULL;
    string = malloc (sizeof(char *) * 300);
    sprintf(string, "Circle:\n\tCX: %.4lf\n\n\tCY: %.4lf\n\n\tR: %.4lf\n\n\tUnits: %s\n\n\tOther Attributes:\n", circ->cx, circ->cy, circ->r, circ->units);
    toAdd = toString(circ->otherAttributes);
    string = realloc(string, sizeof(char *)  * (strlen(toAdd) + strlen(string) + 1));
    strcat(string, toAdd);
    free(toAdd);
    return string;
}
//stub for linked list
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
//stub for linked list api
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
void addAttributes(SVGimage * svg, xmlNode * node){ 
    xmlAttr * xmlattribute = NULL;
    Attribute * newAtt = NULL;   
    for (xmlattribute = node->properties; xmlattribute != NULL; xmlattribute = xmlattribute->next ){ //iterates through the nodes attributes
        newAtt = NULL;
        newAtt = malloc(sizeof(Attribute));  //creates struct
        newAtt->name = malloc(sizeof(char) * (strlen((char *)xmlattribute->name) + 1));
        strcpy(newAtt->name, (char *)xmlattribute->name); //copies name over

        newAtt->value = malloc(sizeof(char) * (strlen((char *)xmlattribute->children->content) + 1));
        strcpy(newAtt->value, (char *)xmlattribute->children->content); //copies value over

        insertBack(svg->otherAttributes, newAtt);
    }
}

void addPaths(Group * grp, SVGimage * svg, xmlNode * node) {
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
            if (svg == NULL) {
                insertBack(grp->paths, addPath);
            }
            else if (grp == NULL){
                insertBack(svg->paths, addPath);
            }
        }
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
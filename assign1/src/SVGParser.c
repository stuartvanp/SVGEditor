/*Stuart van Pinxteren student #0973785*/
#include <math.h>
#include <stdio.h>
#include <strings.h>
#include "SVGParser.h"
#include "LinkedListAPI.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <ctype.h>
#include <libxml/tree.h>
#include <libxml/xmlschemastypes.h>

#define LIBXML_SCHEMAS_ENABLED



void addNameSpace(SVGimage * svg, xmlNode * root);
void addAttributes(SVGimage * svg, xmlNode * node);
void addCircles(Group * grp, SVGimage * svg, xmlNode * node);
void addRectangles(Group * grp, SVGimage * svg, xmlNode * node); 
void addGroups(Group * mygrp, SVGimage * svg, xmlNode * node);
void addPaths(Group * grp, SVGimage * svg, xmlNode * node);


float getValue(xmlChar * valStr);
void getUnits(xmlChar * valStr, char * toCopy);



void getRectsGroup(List * grps, List * rects);
void getCirclesGroup(List * grps, List * circles);
void getPathsGroup(List * grps, List * pths);
void getGroupsGroup(Group * grpScan, List * grpAdd);
void freeSoftList(List * toFree);
int groupLength(Group * grp);



bool validDoc(xmlDoc * doc, char * schemaFile);
bool validSVG(SVGimage * img);
bool validRect(Rectangle * rect);
bool validCircle(Circle * circ);
bool validAttrib(Attribute * attrib);
bool validPath(Path * pth);
bool validGroup(Group * grp);

xmlDoc * createXml(SVGimage * img);
void rectsXml(xmlNode * root, SVGimage * img, Group * grp);
void circsXml(xmlNode * root, SVGimage * img, Group * grp);
void pathsXml(xmlNode * root, SVGimage * img, Group * grp);
void groupsXml(xmlNode * root, SVGimage * img, Group * grp);

SVGimage * fakeCreateSVG(xmlDoc * doc, char * schemaFile);

/* 
*This function creates an svg struct that describes filename, an svg image
*CREDIT: The first ~10 lines of code in the function, which handle the xml file
* were not written by me. They were taken from the file libXmlExample.c (author: Dodji Seketeli) 
*which was given to us in the 'starter' package.
* */

SVGimage* createSVGimage(char* fileName){
    xmlDoc * doc = NULL;
    xmlNode* root = NULL;
    LIBXML_TEST_VERSION

    doc = xmlReadFile(fileName, NULL, 0); //opening file
        if (doc == NULL) {
            xmlFreeDoc(doc);
            xmlCleanupParser();
            return NULL;          //exit if  file does not open properly
        }
    root = xmlDocGetRootElement(doc);
    
    if (root == NULL){     //exit if getrootelement fails
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }
    SVGimage * svg = malloc(sizeof(SVGimage)); //malloc struct
    addNameSpace(svg, root);                   // add namespace, title, desc

    svg->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);   //initialize all struct lists
    svg->circles = initializeList(circleToString, deleteCircle, compareCircles);
    svg->rectangles = initializeList(rectangleToString, deleteRectangle, compareRectangles);
    svg->groups = initializeList(groupToString, deleteGroup, compareGroups);
    svg->paths = initializeList(pathToString, deletePath, comparePaths);
 
    addAttributes(svg, root);  //add all the elements to the svg struct
    addPaths(NULL, svg, root);
    addCircles(NULL, svg, root);
    addRectangles(NULL, svg, root);
    addGroups(NULL, svg, root);

    xmlFreeDoc(doc);        //free residual xml mallocs
    xmlCleanupParser();
    return svg;   //return the completed struct
}

/*
This function takes an SVGimage struct and returns a string describing said struct
*/
char* SVGimageToString(SVGimage* img){
    if (img == NULL) {
        return NULL;
    }
    char * toAdd = NULL;
    char * string = malloc (sizeof(char) * 1000);           //allocating and formatting
    
    sprintf(string, "\nSVG Image\nNameSpace: %s\nTitle: %s\nDescription: %s\nAttributes:", img->namespace, img->title, img->description);

    toAdd = toString(img->otherAttributes);   //gets a string describing the attributes and adds it to string
    string = realloc(string, strlen(string) + strlen(toAdd) + 50);
    strcat(string, toAdd);    
    free(toAdd);                    //frees string of attributes bc it was added to main string which was reallocated

    
    toAdd = toString(img->paths);       //gets a string describing paths and adds it  
    string = realloc(string, sizeof(char) * (strlen(string) + strlen(toAdd) + 50));
    strcat(string, toAdd);
    free(toAdd);                    //frees OG path string

    toAdd = toString(img->rectangles);  //add string describng rectangles
    string = realloc(string, sizeof(char) * (strlen(string) + strlen(toAdd) + 50));
    strcat(string, toAdd);
    free(toAdd); 

    toAdd = toString(img->circles);      //add circle string
    string = realloc(string, sizeof(char) * (strlen(string) + strlen(toAdd) + 50));
    strcat(string, toAdd);
    free(toAdd);

    toAdd = toString(img->groups);       // add group string
    string = realloc(string, sizeof(char) * (strlen(string) + strlen(toAdd) + 50));
    strcat(string, toAdd);
    free(toAdd);

    return string;  //return groups
}


//frees svg image and all necessary other data
void deleteSVGimage(SVGimage* img){
    if (img == NULL) {
        return;
    }
    freeList(img->otherAttributes); //free all the lists
    freeList(img->paths);
    freeList(img->rectangles);
    freeList(img->circles);
    freeList(img->groups);
    free(img);      //free actual struct

    img = NULL;

    return;
}

/*ATTRIBUTE FUNCTIONS 
*the below functions are used for attribute manipulation
*they should never be directly called
*/

/* adds the attributes to the svg struct, node is the root of the xml tree */
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

        insertBack(svg->otherAttributes, newAtt);   //inserts into linked list
    }
}
//helper function to delete attribute structs form linked list
void deleteAttribute( void* data){
    Attribute * attrib = data;
    if (attrib->name != NULL){
        free(attrib->name);
    }  //frees struct pointers
    if (attrib->value!= NULL){
        free(attrib->value);
    }
    free(attrib);       //frees struct
    return;
}
//returns a string describing the attribute
char* attributeToString( void* data){
    Attribute * attrib = data;
    int length =strlen("\tName: ") + strlen(attrib->name) + strlen("\n\tValue: ") + strlen(attrib->value) + strlen("\n") + 51; 
    char * str = malloc(sizeof(char) * (length));                   //malloc and format
    sprintf(str, "\tName: %s\n\tValue: %s\n", attrib->name, attrib->value);
    return str;   //return string, API handles freeing of string
}
//stub function
int compareAttributes(const void *first, const void *second){
    return 0;
}

// adds the title, desc, and namespace to svg struct, not to be used on its own.
void addNameSpace(SVGimage * svg, xmlNode * root) {
    int noTitle = 0;
    int noDesc  = 0;
    strncpy(svg->namespace, (char *)root->ns->href, 255);

    for (xmlNode * mover = root->children; mover != NULL; mover = mover->next){
        if (strcasecmp((char *)mover->name, "title") == 0){
            if (mover->children->content != NULL){
                strncpy(svg->title, (char *)mover->children->content, 255);  
                noTitle = 1;
            }
        }
       if (strcasecmp((char *)mover->name, "desc") == 0){
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


/* the below functions are for manipulating the group sturctures,  
* They should be NEVER be called directly
 */

/* exmines nodes->children for groups, adds them to either mygrp or svg, whichever is not NULL
* recursviely calls itself to add all gorups in the tree to the group or svg struct they should be contained in
*/
void addGroups(Group * mygrp, SVGimage * svg, xmlNode * node){
    Group * grp = NULL;

    for (xmlNode *mover = node ->children; mover != NULL; mover = mover->next) {  //searches through tree level for groups
        grp = NULL;
        if (strcasecmp((char*)mover->name, "g") == 0){          //finds a group
            grp = malloc(sizeof(Group));                    //mallocs and initializes struct vars
            grp->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);
            grp->paths = initializeList(pathToString, deletePath, comparePaths);
            grp->circles = initializeList(circleToString, deleteCircle, compareCircles);
            grp->rectangles = initializeList(rectangleToString, deleteRectangle, compareRectangles);
            grp->groups = initializeList(groupToString, deleteGroup, compareGroups);
            
            for (xmlAttr * attrib = mover->properties; attrib != NULL; attrib = attrib->next) {  //iterates through groups attributes
                Attribute * otherAtt = NULL;
                otherAtt = malloc(sizeof(Attribute));       //mallocs an attribute struct
                otherAtt->name = malloc(sizeof(char) * (strlen((char *)attrib->name) + 1));  //mallocs and copies name
                strcpy(otherAtt->name, (char *)attrib->name);   
                otherAtt->value = malloc (sizeof(char) * (strlen((char *) attrib->children->content) + 1));
                strcpy(otherAtt->value, (char*)attrib->children->content);   //mallocs and copies value
                insertBack(grp->otherAttributes, otherAtt);        //inserts attribute into group attrib list
            }
            addPaths(grp, NULL, mover);      //adds any paths to group
            addCircles(grp, NULL, mover);     //adds any circls to the group 
            addRectangles(grp, NULL, mover);   //adds any rects to the group
            addGroups(grp, NULL, mover);       //adds any groups to the group, recursive call! this handles groups within groups within groups...
            
            if (svg == NULL) {  //inserts the group into the necessary list, this is becuase the group can be added to svg list or group list depending on sitch
                insertBack(mygrp->groups, grp);
            }
            else if ( mygrp == NULL) {
                insertBack(svg->groups, grp);
            }
        }
    }
}
//function to free a group struct
void deleteGroup(void* data){
    Group * grp = data;
    freeList(grp->otherAttributes);  //frees all the lists..
    freeList(grp->paths);
    freeList(grp->circles);
    freeList(grp->rectangles);
    freeList(grp->groups);
    free(grp);       //frees the struct
    return;
}
//function to return a string describing a group
char* groupToString( void* data){
    Group * grp = data;
    char * string = malloc(sizeof(char) * 300);  //allocate memory
    char * toAdd = NULL;
    sprintf(string, "\nGroup: <open>\n\n\tAttributes:\n"); //format string

    toAdd = toString(grp->otherAttributes);      //add string describing attributes
    string = realloc(string, sizeof(char) * (strlen(string) + strlen(toAdd) + 50));
    strcat(string, toAdd);
    free(toAdd);

    toAdd = toString(grp->paths);           //add string describing paths
    string = realloc(string, sizeof(char) * (strlen(string) + strlen(toAdd) + 50));
    strcat(string, toAdd);
    free(toAdd);

    toAdd = toString(grp->circles);         //add string describing circles
    string = realloc(string, sizeof(char) * (strlen(string) + strlen(toAdd) + 50));
    strcat(string, toAdd);
    free(toAdd);

    toAdd = toString(grp->rectangles);      //add string describing rectangles
    string = realloc(string, sizeof(char) * (strlen(string) + strlen(toAdd) + 50));
    strcat(string, toAdd);
    free(toAdd);

    toAdd = toString(grp->groups);         //add string describing groups
    string = realloc(string, sizeof(char) * (strlen(string) + strlen(toAdd) + 50));
    strcat(string, toAdd);
    free(toAdd);

    strcat(string, "Group: <close>\n");    //this is bc there are groups within groups, helps w/ readability
    return string;
}
//stub function for LLAPi
int compareGroups(const void *first, const void *second){
    return 0;
}

/* 
*below are functions for manipulating rectangle structs,
*They should NEVER be called directly
*/


// function adds rectangles to svg struct or a group struct, depending which one is not NULL, 
void addRectangles(Group * grp, SVGimage * svg, xmlNode * node) {
    Rectangle * rect = NULL;
    
    for (xmlNode * mover = node->children; mover != NULL; mover = mover->next ) { //iterates through all the pieces of svg
        rect = NULL;
        if (strcasecmp((char*)mover->name, "rect") == 0) {  //finds rectangle and mallocs
            rect = malloc(sizeof(Rectangle));
            rect->y = 0;
            rect->x = 0;
            rect->width = 0;
            rect->height = 0;
            rect->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);
            strcpy(rect->units, "");
            
            for (xmlAttr * attrib = mover->properties; attrib != NULL; attrib = attrib->next) {  //iterates through rectangle attributes
                Attribute * otherAtt = NULL;

                if (strcasecmp((char *)attrib->name, "y") == 0 ){  //finds y coordinate and saves it
                    rect->y = getValue(attrib->children->content);
                    if (strlen(rect->units) == 0){
                        getUnits(attrib->children->content, rect->units);  //gets units, if ther are any, otherwise copies in a blank string
                    }                    
                }
            
                else if (strcasecmp((char *)attrib->name, "x") == 0 ){ //finds x coordinate and saves it
                    rect->x = getValue(attrib->children->content);
                    strcpy(rect->units, "");
                    if (strlen(rect->units) == 0){
                        getUnits(attrib->children->content, rect->units);  //gets units, if ther are any, otherwise copies in a blank string
                    }
                }
            
                else if (strcasecmp((char *)attrib->name, "width") == 0 ){ //finds width and saves it
                    rect->width = getValue(attrib->children->content);
                    if (strlen(rect->units) == 0){
                        getUnits(attrib->children->content, rect->units);  //gets units, if ther are any, otherwise copies in a blank string
                    }
                }
            
                else if (strcasecmp((char *)attrib->name, "height") == 0 ){ //finds width and saves it
                    rect->height = getValue(attrib->children->content); //negative height is illegal

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
            if (svg == NULL) {
                insertBack(grp->rectangles, rect);
            }
            else if (grp == NULL) {
                insertBack(svg->rectangles, rect);
            }
        }
    }

}
//function to free a rectangle struct
void deleteRectangle(void* data){
    Rectangle * rect = data;
    freeList(rect->otherAttributes);  //frees list
    free(rect);     //frees struct, all other struct elements are statically allocated
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
    string = malloc(sizeof(char) * length);     //allocating and formatting
    sprintf(string, "Rectangle:\n\tX: %.4lf\n\n\tY: %.4lf\n\n\tWidth: %.4lf\n\n\tHeight: %.4lf\n\n\tUnits: %s\n\n\tOther Attributes:\n", rect->x, rect->y, rect->width, rect->height, rect->units);
    toAdd = toString(rect->otherAttributes);   
    string = realloc (string, sizeof(char) * (strlen(string) + strlen(toAdd) + 1));
    strcat(string, toAdd);                    //adding string of attributes
    free(toAdd);
    return string; //return string
}
//stub for linked list api
int compareRectangles(const void *first, const void *second){
    return 0;
}

/* 
*below are the functions for handling circle structs
*They should NEVER be called directly... unless youre me and know how they work
*/

//adds circles to svg struct, or group, depending which arguement is not NULL
void addCircles(Group * grp, SVGimage * svg, xmlNode * node) {
    Circle * circ = NULL;
    Attribute * otherAtt = NULL; 
             
    for (xmlNode * mover = node->children; mover != NULL; mover = mover->next) { //moves through the SVG elements
        circ = NULL;

        if (strcasecmp((char *) mover->name, "circle") == 0) {  //finds a circle, mallocs
            
            circ = malloc(sizeof(Circle));
            circ->cx = 0;
            circ->cy = 0;
            circ->r = 0;
            circ->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);
            strcpy(circ->units, "");
            
            for (xmlAttr * attrib = mover->properties; attrib != NULL; attrib = attrib->next ){  //moves throught the circles attributes
                otherAtt = NULL;
            
                if (strcasecmp((char *)attrib->name, "cx") == 0) {   //finds the cx value
                    circ->cx = getValue(attrib->children->content);
                    if (strlen(circ->units) == 0){
                        getUnits(attrib->children->content, circ->units);  //gets units
                    }
                }
                else if (strcasecmp((char*)attrib->name, "cy") == 0){  //finds the cy value
                    circ->cy = getValue(attrib->children->content);
                    if (strlen(circ->units) == 0){
                        getUnits(attrib->children->content, circ->units);  //gets units
                    }
                }
                else if (strcasecmp((char*)attrib->name, "r") == 0){   //finds the r value
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
    string = malloc (sizeof(char *) * 300);   //allocating and formatting
    sprintf(string, "Circle:\n\tCX: %.4lf\n\n\tCY: %.4lf\n\n\tR: %.4lf\n\n\tUnits: %s\n\n\tOther Attributes:\n", circ->cx, circ->cy, circ->r, circ->units);
    toAdd = toString(circ->otherAttributes);    //adding attribute string
    string = realloc(string, sizeof(char *)  * (strlen(toAdd) + strlen(string) + 1));
    strcat(string, toAdd);
    free(toAdd);
    return string;
}
//stub for linked list
int compareCircles(const void *first, const void *second){
    return 0;
}

/*
*below are the functions for handlign paths
* they should never be called directly
*
 */

 //adds paths to svg or grp, whichever argument is not null
void addPaths(Group * grp, SVGimage * svg, xmlNode * node) {
    Path * addPath;
    Attribute * otherAtt;
    
    for (xmlNode * mover = node->children; mover != NULL; mover = mover->next){ //iterates through the elements of xmltree 
        addPath = NULL;
        
        if (strcasecmp((char*)mover->name, "path") == 0) { //if a path is found
            addPath = malloc(sizeof(Path));                  //creates struct
            addPath->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);         //initializes list of otherAtttributes

            for (xmlAttr * attrib = mover->properties; attrib != NULL; attrib = attrib->next ){ //iterates through the attributes
            
                if (strcasecmp((char*)attrib->name, "d") == 0){                 //d is the only important attribute
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
    int length = strlen("Path:\n\tAttributes:\n\tData: ") + strlen(pth->data) + 100;
    string = malloc(sizeof(char) * length);
    sprintf(string, "Path:\n\tData: %s\n\n\tAttributes:\n", pth->data);
    toAdd = toString(pth->otherAttributes);
    string = realloc(string, sizeof(char) * (strlen(string) + strlen(toAdd) + 100));
    strcat(string, toAdd);
    free(toAdd);

    return string;
}
//stub for linked list api
int comparePaths(const void *first, const void *second){
    return 0;
}



// Function that returns a list of all rectangles in the image.  
List* getRects(SVGimage* img){
    if (img == NULL)  {   //validates svg;
        return NULL;
    }
    List * rects =  initializeList(rectangleToString, deleteRectangle, compareRectangles);  //allocates a list
    ListIterator iter = createIterator(img->rectangles);  //creates iterator

    while (iter.current != NULL){
        insertBack(rects,nextElement(&iter));  //adds all rectangles from svg->rectangles
    }
    getRectsGroup(img->groups, rects);  //recursive function for groups


    return rects;
}
 //recursive function that returns ALL rectangles in list grps
void getRectsGroup(List * grps, List * rects){
    if (grps == NULL) {
        return;
    }
    ListIterator grpiter = createIterator(grps);  //iterator for the list of groups

    while(grpiter.current != NULL) {   //iterates through every group
        Group * tempGrp = nextElement(&grpiter);    //iterator for the group list of rectangles
        if (tempGrp->rectangles != NULL){
            ListIterator recter =  createIterator(tempGrp->rectangles);
            while (recter.current != NULL) {  //iterates through the rectangles
                insertBack(rects,nextElement(&recter)); //adds them
            
            }
        }  
        getRectsGroup(tempGrp->groups, rects);  //recursive call onto the groups List within the group being looked at
    }
    return;

}
// Function that returns a list of all circles in the image.  
List* getCircles(SVGimage* img){
    if (img == NULL)  {   //validates svg;
        return NULL;
    }
    List * circs =  initializeList(circleToString, deleteCircle, compareCircles);  //allocates a list
    ListIterator iter = createIterator(img->circles);  //creates iterator

    while (iter.current != NULL){
        insertBack(circs,nextElement(&iter));  //adds all circles from svg->circles
    }
    getCirclesGroup(img->groups, circs);  //recursive function that looks through groups
    return circs;
}
//recursive function for adding circles within groups to circles list
void getCirclesGroup(List * grps, List * circles) {
    if (grps == NULL) {
        return;
    }
    ListIterator grpiter = createIterator(grps);  //iterator for the list of groups

    while(grpiter.current != NULL) {   //iterates through every group
        Group * tempGrp = nextElement(&grpiter);    //this returns the current group then pushes the iterator forward
        if (tempGrp->circles != NULL) {
            ListIterator circiter =  createIterator(tempGrp->circles);
            while (circiter.current != NULL) {  //iterates through the circles
                insertBack(circles,nextElement(&circiter)); //adds them
            }
        }

        getCirclesGroup(tempGrp->groups, circles);  //recursive call onto the groups List within the group being looked at
    }
    return;
}

// Function that returns a list of all paths in the image.  
List* getPaths(SVGimage* img){
     if (img == NULL)  {   //validates svg;
        return NULL;
    }
    List * pths =  initializeList(pathToString, deletePath, comparePaths);  //allocates a list
    ListIterator iter = createIterator(img->paths);  //creates iterator

    while (iter.current != NULL){
        insertBack(pths,nextElement(&iter));  //adds all paths from svg->paths
    }
    getPathsGroup(img->groups, pths);  //recursive function that looks through groups
    return pths;
}
//recursive function for returning adding paths within groups to pths list
void getPathsGroup(List * grps, List * pths){
    if (grps == NULL) {
        return;
    }
    ListIterator grpiter = createIterator(grps);  //iterator for the list of groups

    while(grpiter.current != NULL) {   //iterates through every group
        Group * tempGrp = nextElement(&grpiter);    //this returns the current group then pushes the iterator forward
        if (tempGrp->paths != NULL) {
            ListIterator pathiter =  createIterator(tempGrp->paths);
            while (pathiter.current != NULL) {  //iterates through the paths
                insertBack(pths,nextElement(&pathiter)); //adds them            
            }  
        }
        getPathsGroup(tempGrp->groups, pths);  //recursive call onto the groups List within the group being looked at
    }
    return;
} 

// Function that returns a list of all groups in the image.  
List* getGroups(SVGimage* img){
    if (img == NULL) {   //validates img
        return NULL;
    }
    List * grpAdd = initializeList(groupToString, deleteGroup, compareGroups); //creates list
    ListIterator iter = createIterator(img->groups);    //iterator for svg groups list
    
    while (iter.current != NULL) {     //iterates through groups list
        Group * tempGrp = nextElement(&iter);   //gets current group and pushes iterator forwards
        insertBack(grpAdd, tempGrp);          //inserts 
        getGroupsGroup(tempGrp, grpAdd);
    }
    
    return grpAdd;
}   
//recursive functions that returns ALL groups contained within grpScan
void getGroupsGroup(Group * grpScan, List * grpAdd) {
    if (grpScan->groups != NULL){
        ListIterator iter = createIterator(grpScan->groups);  //iterator for grpscans group list
        while (iter.current != NULL) {
            Group * tempGrp = nextElement(&iter);  //gets current group and moves iterator forward
            insertBack(grpAdd, tempGrp);             //inserts into list
            getGroupsGroup(tempGrp, grpAdd);      //recursive call on the group just found
        }
    }
    return;
}

//function is used to free the Lists that are returned the GetXXXX functions
//CREDIT: the two if statements  from the beginning of this function were taken from the linkedListAPI.c function "clearList"
//this is a similar function but it does not free the data structures at each node 
//i use it in my numXXXX functions
void freeSoftList(List * toFree){
    if (toFree == NULL){  //validates list
        return;
    }
    if (toFree->head == NULL) { //empty list case
        free(toFree);
        return;
    }
    Node * dummy = toFree->head;  //dummy is 1 node behind current
    Node * current = dummy->next;

    while (current != NULL){ //walks through the list
        free(dummy); 
        dummy = current;
        current = current->next;
    }
    free(dummy);  //free the last node
    free(toFree); //free the list
    return;
}



// Function that returns the number of all rectangles with the specified area
int numRectsWithArea(SVGimage* img, float area){
    if (img == NULL || area < 0) {  //validates inputs
        return 0;
    }
    
    int numRects = 0;
    int compare = (int) ceil(area); //rounds input
    List * rects = getRects(img);   //gets a list of all rectangles in the svg
    

    ListIterator iter = createIterator(rects);  //create iterator

    while (iter.current != NULL){     //walk through list
        Rectangle * tempRect = nextElement(&iter);     //returns rectangle and pushes iterator forward
        int tempArea = ceil(tempRect->width * tempRect -> height);    //calculate area
        if (compare == tempArea) {
            numRects++;              //increment if equal
        }
    }
    freeSoftList(rects);  //frees the soft list of rects
    return numRects;
}


// Function that returns the number of all circles with the specified area
int numCirclesWithArea(SVGimage* img, float area){
    if (img == NULL|| area < 0) {  //validates inputs
        return 0;
    }
    int numCircles = 0;
    int compare = (int)ceil(area);
    List * circles = getCircles(img);   //gets a soft list of all the circles in svg

    ListIterator iter = createIterator(circles);

    while (iter.current != NULL) {              //iterates through the list
        Circle * tempCirc = nextElement(&iter);    //returns current circle and pushes iterator
        int tempArea = (int)ceil( 3.14159 * tempCirc->r * tempCirc->r);    //calculate area
  
        if (tempArea == compare) {     //compare and increment
            numCircles++;
        }
    }
    freeSoftList(circles);     //free the list and return numCircles
    return numCircles;
}


// Function that returns the number of all paths with the specified data - i.e. Path.data field
int numPathsWithdata(SVGimage* img, char* data){
    if (img == NULL ||data == NULL ||strlen(data) == 0){
        return 0;
    }
    int numPaths = 0;
    List * paths = getPaths(img);
    ListIterator iter = createIterator(paths);
    while (iter.current != NULL) {
        Path * temPath = nextElement(&iter);
        if (strcmp(temPath->data, data) == 0) {
            numPaths++;
        }
    }
    freeSoftList(paths);
    return numPaths;

}


// Function that returns the number of all groups with the specified length - see A1 Module 2 for details
int numGroupsWithLen(SVGimage* img, int len){
    if (img == NULL || len < 0){   //validate inputs
        return 0;
    }
    int numGroups = 0;
    int compare = len;
    List * groups = getGroups(img);  //get a list of all groups in svg

    ListIterator iter = createIterator(groups);  //iterator for group list

    while(iter.current != NULL){
        Group * tempGrp = nextElement(&iter);   //returns current and pushes iterator
        if (compare == groupLength(tempGrp)){   //compares and increments
            numGroups++;
        }
    }
    freeSoftList(groups);  //frees list
    return numGroups;
}
//returns 'length' of group
int groupLength(Group * grp) {
    return getLength(grp->circles) + getLength(grp->rectangles) + getLength(grp->paths) + getLength(grp->groups);
}


int numAttr(SVGimage* img){
    if (img == NULL) { // validates inputs
        return 0;
    }
    int numAtt = 0;

    //gets list of all elements in the svg struct
    List * rects = getRects(img);
    List * circles = getCircles(img);
    List * paths = getPaths(img);
    List * groups = getGroups(img);
    List * svgAtt = img->otherAttributes;


    numAtt = numAtt + getLength(svgAtt); //adds svg attributes

    //adds rectanle attributes
    ListIterator iter = createIterator(rects);  //list iterators
    while(iter.current != NULL) {
        Rectangle * rect = nextElement(&iter);   //sets rect equal to current node and pushes iter forward
        numAtt = numAtt + getLength(rect->otherAttributes);  //adds length
    }
    //adds rectangle attributes
    iter = createIterator(circles);
    while(iter.current != NULL) {
        Circle * circ = nextElement(&iter);
        numAtt = numAtt + getLength(circ->otherAttributes);
    }
    //adds path attributes
    iter = createIterator(paths);
    while(iter.current != NULL) {
        Path * path = nextElement(&iter);
        numAtt = numAtt + getLength(path->otherAttributes);
    }
    //adds group attributes
    iter = createIterator(groups);
    while(iter.current != NULL){
        Group * group = nextElement(&iter);
        numAtt = numAtt + getLength(group->otherAttributes);
    }
    //frees the lists that were created
    freeSoftList(rects);
    freeSoftList(circles);
    freeSoftList(paths);
    freeSoftList(groups);
    return numAtt;
}

SVGimage* createValidSVGimage(char* fileName, char* schemaFile) {

    if (fileName == NULL || schemaFile == NULL) {
        return NULL;
    }
    xmlDoc * doc = NULL;
    xmlNode* root = NULL;
    LIBXML_TEST_VERSION

    doc = xmlReadFile(fileName, NULL, 0); //opening file
    if (doc == NULL) {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;          //exit if  file does not open properly
    }

    if (validDoc(doc, schemaFile) == false) {
        xmlFreeDoc(doc);        //free residual xml mallocs
        xmlCleanupParser();
        xmlMemoryDump();
        return NULL;
    }

    root = xmlDocGetRootElement(doc);
    
    if (root == NULL){     //exit if getrootelement fails
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }
    SVGimage * svg = malloc(sizeof(SVGimage)); //malloc struct
    addNameSpace(svg, root);                   // add namespace, title, desc

    svg->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);   //initialize all struct lists
    svg->circles = initializeList(circleToString, deleteCircle, compareCircles);
    svg->rectangles = initializeList(rectangleToString, deleteRectangle, compareRectangles);
    svg->groups = initializeList(groupToString, deleteGroup, compareGroups);
    svg->paths = initializeList(pathToString, deletePath, comparePaths);
 
    addAttributes(svg, root);  //add all the elements to the svg struct
    addPaths(NULL, svg, root);
    addCircles(NULL, svg, root);
    addRectangles(NULL, svg, root);
    addGroups(NULL, svg, root);

    xmlFreeDoc(doc);        //free residual xml mallocs
    xmlCleanupParser();
    xmlMemoryDump(); ///////////////////////////////////////////////////////////////////////////////////////////////////////////dunno fam
    return svg;   //return the completed struct
}


bool validDoc(xmlDoc * doc, char * schemaFile) {
    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr ctxt;
    xmlLineNumbersDefault(1);

    ctxt = xmlSchemaNewParserCtxt(schemaFile);

    xmlSchemaSetParserErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
    schema = xmlSchemaParse(ctxt);
    xmlSchemaFreeParserCtxt(ctxt);

    xmlSchemaValidCtxtPtr ctext;
    int ret = 0;
    ctext = xmlSchemaNewValidCtxt(schema);
    xmlSchemaSetValidErrors(ctext, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
    ret = xmlSchemaValidateDoc(ctext, doc);
    xmlSchemaFreeValidCtxt(ctext);
    xmlSchemaFree(schema);
    if (ret == 0) {
        return true;
    }
    return false;
}


//function takes an svg struct and uses it to create an xml. does not free the svg
xmlDoc * createXml(SVGimage * img) {
    xmlDoc * doc = NULL;
    xmlNode * root_node = NULL;
    xmlNs * ns = NULL;


    doc = xmlNewDoc(BAD_CAST "1.0");    //creates doc and root node
    root_node = xmlNewNode(NULL, BAD_CAST "svg");
    xmlDocSetRootElement(doc, root_node);
    
    ns = xmlNewNs(root_node, BAD_CAST img->namespace, NULL);  //adds name space
    xmlSetNs(root_node, ns);

    if (strlen(img->title) != 0) {  //adds a title, if it exists
        xmlNewChild(root_node, NULL, BAD_CAST "title", BAD_CAST img->title);
    }
    if (strlen(img -> description) != 0) {  //adds a description, if it exists
        xmlNewChild(root_node, NULL, BAD_CAST "desc", BAD_CAST img->description);
    }
    //adds all of the otherAttributesfrom the svg struct
    ListIterator iter = createIterator(img->otherAttributes);
    while (iter.current != NULL) {
        Attribute * otherAtt = nextElement(&iter);
        xmlNewProp(root_node, BAD_CAST otherAtt->name, BAD_CAST otherAtt->value);
    }

    rectsXml(root_node, img, NULL); //adds rectangles
    circsXml(root_node, img, NULL); //adds circles
    pathsXml(root_node, img, NULL);  //adds paths
    groupsXml(root_node, img, NULL);

    return doc;
}

//adds rects in img->rects to become children of root
void rectsXml(xmlNode * root, SVGimage * img, Group * grp) {
    ListIterator iter;
    if(img != NULL) { 
        iter = createIterator(img->rectangles);  //create iterator
    }
    else {
        iter = createIterator(grp->rectangles); 
    }
    char buffer[256];
    while (iter.current != NULL){     //walk through list
        Rectangle * tempRect = nextElement(&iter);     //returns rectangle and pushes iterator forward
        xmlNode * rectNode = xmlNewChild(root, NULL, BAD_CAST "rect", NULL);   //cretaes child node to root for found rectangle
        
        sprintf(buffer, "%f%s", tempRect->x, tempRect->units);  //adds x val and units
        xmlNewProp(rectNode, BAD_CAST "x", BAD_CAST buffer);

        sprintf(buffer, "%f%s", tempRect->y, tempRect->units);   //adds y val
        xmlNewProp(rectNode, BAD_CAST "y", BAD_CAST buffer);

        sprintf(buffer, "%f%s", tempRect->width, tempRect->units); //adds width
        xmlNewProp(rectNode, BAD_CAST "width", BAD_CAST buffer);

        sprintf(buffer, "%f%s", tempRect->height, tempRect->units);  //adds height
        xmlNewProp(rectNode, BAD_CAST "height",BAD_CAST buffer);

        ListIterator attIter = createIterator(tempRect->otherAttributes);  //iterator for otherAtts

        while (attIter.current != NULL) {
            Attribute * otherAtt  = nextElement(&attIter);        
            xmlNewProp(rectNode, BAD_CAST otherAtt->name, BAD_CAST otherAtt->value); //adds otherAtts of to rectangle nodes
        }
    }
    return;
}

//adds circs in img->circs to become children of root
void circsXml(xmlNode * root, SVGimage * img, Group * grp) {
    ListIterator iter; 
    if (img != NULL) {
        iter= createIterator(img->circles); // iterates through circles list
    }
    else{
        iter = createIterator(grp->circles);
    }
    
    char buffer[256];

    while(iter.current != NULL) {  //iterates thoguh circls list
        Circle * circ = nextElement(&iter);
        xmlNode * node = xmlNewChild(root, NULL, BAD_CAST "circle", NULL); //adds circl child node

        sprintf(buffer, "%f%s", circ->cx, circ->units);  //adds cx
        xmlNewProp(node, BAD_CAST "cx", BAD_CAST buffer);

        sprintf(buffer, "%f%s", circ->cy, circ->units); //adds cy
        xmlNewProp(node, BAD_CAST "cy", BAD_CAST buffer);
    
        sprintf(buffer, "%f%s", circ->r, circ->units);  //adds radius
        xmlNewProp(node, BAD_CAST "r", BAD_CAST buffer);

        ListIterator attIter = createIterator(circ->otherAttributes);

        while (attIter.current != NULL) {  //iterates through other attributes of circles
            Attribute * otherAtt = nextElement(&attIter);
            xmlNewProp(node, BAD_CAST otherAtt->name, BAD_CAST otherAtt->value); //adds other attributes to circl node
        }
    }
}

//adds paths from img->paths to root
void pathsXml(xmlNode * root, SVGimage * img, Group * grp){
    ListIterator iter;
    if (img != NULL) {
        iter = createIterator(img->paths);
    }
    else {
        iter = createIterator(grp->paths);
    }
    while (iter.current != NULL) {   //iterates through the paths list
        Path * pth = nextElement(&iter);
        xmlNode * node = xmlNewChild(root, NULL, BAD_CAST "path", NULL);  //adds a path node to

        xmlNewProp(node, BAD_CAST "d", BAD_CAST pth->data);  //adds data
        ListIterator attIter = createIterator(pth->otherAttributes);

        while (attIter.current != 0) {   //adds otherAttributes
            Attribute * otherAtt = nextElement(&attIter);
            xmlNewProp(node, BAD_CAST otherAtt->name, BAD_CAST otherAtt->value);
        }
    }
}


//adds groups form img or grp to be children of node root
void groupsXml(xmlNode * root, SVGimage * img, Group * grp) {
    ListIterator iter;
    if (img != NULL){  //checks if from img or grp
        iter = createIterator(img->groups);
    }
    else {
        iter = createIterator(grp->groups);
    }
    while (iter.current != NULL) {  //iterates through group
        Group * tmpgrp = nextElement(&iter);
        xmlNode * node = xmlNewChild(root, NULL, BAD_CAST "g", NULL); //makes child node

        ListIterator attIter = createIterator(tmpgrp->otherAttributes); //iterator for other attributes
        while (attIter.current != NULL) {
            Attribute * otherAtt = nextElement(&attIter);   //adds attributes
            xmlNewProp(node, BAD_CAST otherAtt->name, BAD_CAST otherAtt->value);
        }
        rectsXml(node, NULL, tmpgrp);  //adds rects, circlces, paths
        circsXml(node, NULL, tmpgrp);
        pathsXml(node, NULL, tmpgrp);
        groupsXml(node, NULL, tmpgrp);  //recursive call to add groups within groups 
    }
}

//boolena function that validates an svg struct against header file parameters
bool validSVG(SVGimage * img){
    ListIterator iter;
    if (strlen(img->namespace) <= 0){  //checks validity of svg struct
        return false;
    }
    if (img->rectangles == NULL || img->circles  == NULL || img->paths == NULL) {
        return false;
    }
    if  (img->groups == NULL || img->otherAttributes == NULL){
        return false;
    }

    iter = createIterator(img->otherAttributes);
    while (iter.current != NULL){    //iterates through attributes
        Attribute * attrib = nextElement(&iter);
        if (validAttrib(attrib) == false){   //checks attribute parameters
            return false;
        }
    }

    List * rects = getRects(img);  //checks validity of all rectangle structs
    iter = createIterator(rects);
    while (iter.current != NULL) {
        Rectangle * rect = nextElement(&iter);
        if (validRect(rect) == false) {
            freeSoftList(rects);
            return false;
        }
    }
    freeSoftList(rects);

    List * circs = getCircles(img);  //chekcs validity of all circle structs
    iter = createIterator(circs);
    while (iter.current != NULL) {
        Circle * circ = nextElement(&iter);
        if (validCircle(circ) == false){
            freeSoftList(circs);
            return false;
        }
    }
    freeSoftList(circs);



    List * pths = getPaths(img);  //checks validity of all path structs
    iter = createIterator(pths);
    while (iter.current != NULL){
        Path * pth = nextElement(&iter);
        if (validPath(pth) == false){
            freeSoftList(pths);
            return false;
        }
    }
    freeSoftList(pths);



    List * grps = getGroups(img);   //checks validity of all group structs
    iter = createIterator(grps);
    while (iter.current != NULL) {
        Group * grp = nextElement(&iter);
        if (validGroup(grp) == false){
            freeSoftList(grps);
            return false;
        }
    }
    freeSoftList(grps);

    return true;   //returns true if nothing fails
}

//balidates a rectangle struct and its other attributes
bool validRect(Rectangle * rect){
    if (rect == NULL) { 
        return false;
    }
    if (rect->width < 0 || rect->height < 0 || rect->otherAttributes == NULL) {
        return false;  //checks necessary paremeters
    }
    ListIterator iter = createIterator(rect->otherAttributes);
    while (iter.current != NULL){    //iterates through attributes
        Attribute * attrib = nextElement(&iter);
        if (validAttrib(attrib) == false){   //checks attribute parameters
            return false;
        }
    }
    return true;

}
//boolean validatin of a circle struct. 
bool validCircle(Circle * circ) {
    
    if (circ == NULL) {
        return false;
    }
    if (circ->r < 0 || circ->otherAttributes == NULL) {
        return false;  //checks parameters
    }
    
    ListIterator iter = createIterator(circ->otherAttributes);
    while (iter.current != NULL){    //iterates through attributes
        Attribute * attrib = nextElement(&iter);
        if (validAttrib(attrib) == false){   //checks attribute parameters
            return false;
        }
    }
    return true;
}

//boolean validation of a path struct
bool validPath(Path *pth){
    if (pth == NULL){
        return false;
    }
    if (pth->data == NULL || pth->otherAttributes == NULL) {
        return false;  //validates parameters
    }

    ListIterator iter = createIterator(pth->otherAttributes);
    while (iter.current != NULL){    //iterates through attributes
        Attribute * attrib = nextElement(&iter);
        if (validAttrib(attrib) == false){   //checks attribute parameters
            return false;
        }
    }
    return true;
}

//boolean validation for a group based on header parameters
bool validGroup(Group * grp){
    if (grp == NULL){
        return false;
    }
    if (grp->rectangles == NULL || grp->circles == NULL || grp->paths == NULL) {
        return false;  //testing parameters
    }
    if (grp->groups == NULL || grp->otherAttributes == NULL){
        return false;  //further testing
    }
    ListIterator iter = createIterator(grp->otherAttributes);
    while (iter.current != NULL){    //iterates through attributes
        Attribute * attrib = nextElement(&iter);
        if (validAttrib(attrib) == false){   //checks attribute parameters
            return false;
        }
    }
    return true;
}


//boolean validates an attribute struct
bool validAttrib(Attribute * attrib) {
    if (attrib == NULL) { //returns false if pointer is null or struct pointers are null
        return false;
    }
    if (attrib->name == NULL || attrib->value == NULL){
        return false;
    }
    return true;
}



//frees an xmlDoc  
void docFree(xmlDoc * doc) {
    xmlFreeDoc(doc);
    xmlCleanupParser();
    xmlMemoryDump();
}

//helper function to TEST my code, never used IRL
SVGimage * fakeCreateSVG(xmlDoc * doc, char * schemaFile) {
    xmlNode* root = NULL;
    LIBXML_TEST_VERSION


    if (doc == NULL) {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;          //exit if  file does not open properly
    }

    


    if (validDoc(doc, schemaFile) != 0) {
        xmlFreeDoc(doc);        //free residual xml mallocs
        xmlCleanupParser();
        xmlMemoryDump();
        return NULL;
    }

    root = xmlDocGetRootElement(doc);
    
    if (root == NULL){     //exit if getrootelement fails
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }
    SVGimage * svg = malloc(sizeof(SVGimage)); //malloc struct
    addNameSpace(svg, root);                   // add namespace, title, desc

    svg->otherAttributes = initializeList(attributeToString, deleteAttribute, compareAttributes);   //initialize all struct lists
    svg->circles = initializeList(circleToString, deleteCircle, compareCircles);
    svg->rectangles = initializeList(rectangleToString, deleteRectangle, compareRectangles);
    svg->groups = initializeList(groupToString, deleteGroup, compareGroups);
    svg->paths = initializeList(pathToString, deletePath, comparePaths);
 
    addAttributes(svg, root);  //add all the elements to the svg struct
    addPaths(NULL, svg, root);
    addCircles(NULL, svg, root);
    addRectangles(NULL, svg, root);
    addGroups(NULL, svg, root);

    xmlFreeDoc(doc);        //free residual xml mallocs
    xmlCleanupParser();
    xmlMemoryDump(); 
    return svg;   //return the completed struct
}


bool validateSVGimage(SVGimage* img, char* schemaFile){

    if (img == NULL || schemaFile == NULL){
        return false;
    }
    if (validSVG(img) == false) {
        return false;
    }

    xmlDoc * doc = createXml(img);
    if (validDoc(doc, schemaFile) == false) {
        docFree(doc);
        return false;
    }
    docFree(doc);
    
    return true;
}

bool writeSVGimage(SVGimage* img, char* fileName){
    if (img == NULL || fileName == NULL) {
        return false;
    }


    xmlDoc * doc = createXml(img);
    if (doc == NULL) {
        return false;
    }
    
    int ret = xmlSaveFormatFileEnc(fileName, doc, "UTF-8", 1);
    if (ret == -1) {
        docFree(doc);
        return false;
    }
    docFree(doc);
    return true;
}


int main (int argc, char * argv[]) {


    SVGimage * svg = createValidSVGimage(argv[1], "svg.xsd");
    char * str = SVGimageToString(svg);
    printf("%s", str);
    free(str);  
 

    writeSVGimage(svg, "write.svg");

    SVGimage * svg2 = createValidSVGimage("write.svg", "svg.xsd");

    str = SVGimageToString(svg2);
    printf("%s", str);
    deleteSVGimage(svg);
    deleteSVGimage(svg2);
    free(str);

    return 0;
}
 


 
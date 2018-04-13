
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/**
*Node of the hash table. 
**/
typedef struct HashNode
{
	char* key; ///< integer that represents a piece of data in the table (eg 35->"hello")
	void *data; ///< pointer to generic data that is to be stored in the hash table
	struct HashNode *next; ///< pointer to the next Node if a collision is detected
} HashNode;


typedef struct Xref
{
	char id[10]; ///< integer that represents a piece of data in the table (eg 35->"hello")
	void *data; ///< pointer to generic data that is to be stored in the hash table
    
} Xref;

/**
*Hash table structure
**/
typedef struct HTable HTable;
struct HTable
{
	size_t size; ///< number that represents the size of the hash table
	HashNode **table; ///< array that contains all of the table nodes
	void (*destroyData)(void *data); ///< function pointer to a function to delete a single piece of data from the hash table
	int (*hashFunction)(size_t tableSize, char* key); ///< function pointer to a function to hash the data 
    void (*printNode)(void *toBePrinted); ///< function pointer to a function that prints out a data element of the table
};



/**Function to point the hash table to the appropriate functions. Allocates memory to the struct and table based on the size given.
*@return pointer to the hash table
*@param size size of the hash table
*@param hashFunction function pointer to a function to hash the data
*@param destroyData function pointer to a function to delete a single piece of data from the hash table
*@param printNode function pointer to a function that prints out a data element of the table
**/
HTable *createTable(size_t size, int (*hashFunction)(size_t tableSize, char* key),void (*destroyData)(void *data),void (*printNode)(void *toBePrinted));

/**Function for creating a node for the hash table.
*@pre Node must be cast to void pointer before being added.
*@post Node is valid and able to be added to the hash table
*@param key character that represents the data (eg 35->"hello")
*@param data is a generic pointer to any data type.
*@return returns a node for the hash table
**/
HashNode *createNode(char* key, void *data);

/** Deletes the entire hash table and frees memory of every element.
*@pre Hash Table must exist.
*@param hashTable pointer to hash table containing elements of data
**/
void destroyTable(HTable *hashTable);  

/**Inserts a Node in the hash table.
*@pre hashTable type must exist and have data allocated to it
*@param hashTable pointer to the hash table
*@param key integer that represents the data (eg 35->"hello")
*@param data pointer to generic data that is to be inserted into the list
**/
void insertData(HTable *hashTable, char* key, void *data);

/** THIS FUNCTION IS NOT MANDATORY, users call this function to insert a Node in the hash table.
* It's meant as a wrapper for insertData so the users don't need to generate the key when adding.
*@pre hashTable type must exist and have data allocated to it
*@param hashTable pointer to the hash table
*@param data pointer to generic data that is to be inserted into the list
**/
void insertDataInMap(HTable *hashTable, void *data);

/**Function to remove a node from the hash table 
 *@pre Hash table must exist and have memory allocated to it
 *@post Node at key will be removed from the hash table if it exists.
 *@param hashTable pointer to the hash table struct
 *@param key integer that represents a piece of data in the table (eg 35->"hello")
 **/
void removeData(HTable *hashTable, char* key);

/**Function to return the data from the key given.
 *@pre The hash table exists and has memory allocated to it
 *@param hashTable pointer to the hash table containing data nodes
 *@param key integer that represents a piece of data in the table (eg 35->"hello")
 *@return returns a pointer to the data in the hash table. Returns NULL if no match is found.
 **/
void *lookupData(HTable *hashTable, char* key);

FILE *openFile(char *filename);

/**Function to get the data from the node
*@return the data of the node (the word)
*@param the node
**/
void *getData(HashNode * node);

/**Function to get the key from the node
*@return the key of the node
*@param a node
**/
char *getKey(HashNode * node);

/**Function to generate a hash for the key using hash division.
*@param size of the hash table
*@param key to be hashed into the index
**/
int hashNode(size_t tableSize, char* key);

/**Function to free the memory of the node but no real data to avoid double free
*@param Item that needs to be deleted from the list.
**/
void dummyDelete(void *data);

/**Function to print the data of the Node
*@param the data to print out
**/
void printNodeData(void *toBePrinted);

/**Function to print just the name of an Individual struct
*@param a string to print out
**/
char *printIndividualName(void *toBePrinted);

/**Function get a string from a file
*@param a string to populate
*@return the new string
**/
char *myfgets(char *dst, int max, FILE *fp);


/**Function to write an event to a file
*@param an event to be written
*@param A file to write toFile
*@param A list containing comparisons
**/
void writeEvent(void* toBePrinted, FILE *outFile, List list);

/**Function to write an family to a file
*@param a family to be written
*@param A file to write toFile
*@param A list containing comparisons
**/
void writeFamily(void* toBePrinted, FILE *outFile, List list);

/**Function to write a field to a file
*@param A field to write toFile
*@param A list containing comparisons
**/
void writeField(void* toBePrinted, FILE *outFile, List list);

/**Function to write an individual to a file
*@param an individual to be written
*@param A file to write toFile
*@param A list containing comparisons
**/
void writeIndividual(void* toBePrinted, FILE *outFile, List list);

/**Function to write a list to a file
*@param an list to compare
*@param A file to write toFile
*@param A function pointer for writing to the file
**/
void toFile(List list, List compareList, FILE *outFile, void (*printFunction)(void *toBePrinted, FILE *outFile, List list));

/**Function to get an XREF from a list
*@param an XREF list
*@param A function pointer for comparison
*@return the XREF
**/
void* findXREF(List list, int (*customCompare)(const void* first,const void* second), const void* searchRecord);

/**Function to compare to values
*@param a value to compare
*@param A second value to compare
*@return a boolean value if the comparison is true or not
**/
bool compareFieldsBool(const void* first,const void* second);

/**Helper function for getDescendants
*@param a list of descendants
*@param A GEDCOM object
*@param A n individual to add
*@return the new list
**/
List getDescendantsRecursive(List descendantList, const GEDCOMobject *object, Individual *individual);

/**Function to compare to individual names
*@param an individual to compare
*@param A second individual to compare
*@return an integer
**/
int compareIndividualsName(const void *first, const void* second);

/**Helper function for getDescendantListN
*@param a tempList containing all descendants
*@param A GEDCOM object
*@param An individual to add
*@param The max number of generations
*@param an index of the list
*@return the new string
**/
List **getDescendantsRecursiveN(List **tempList, List *descendantList, const GEDCOMobject *object, Individual *individual, unsigned int maxGen, int index);

/**Helper function for getAncestorListN
*@param a tempList containing all ancestors
*@param A GEDCOM object
*@param An individual to add
*@param The max number of generations
*@param an index of the list
*@return the new string
**/
List **getAncestorsRecursiveN(List **tempList, List *ancestorList, const GEDCOMobject *object, Individual *individual, unsigned int maxGen, int index);

/**Function to validate a GEDCOM object
*@param an object to validate
*@return true or false if the object is valid
**/
bool validateRecord(const GEDCOMobject *object);
char *gedcomToJSON(char* filename);
bool compareIndividualsBool(const void* first, const void *second);
char *getAncestorsJSON(char *filename, const char* json, int num);
void jsonToNewFile(const char *json, char *filename);
void addIndJSON(char *filename, const char *json);
char *parseIndividualsToJSON(char *filename);
char *getDescendantsJSON(char *filename, const char* json, int num);
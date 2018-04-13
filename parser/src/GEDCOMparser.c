#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


#include "GEDCOMparser.h"
#include "GEDCOMutilities.h"

GEDCOMerror createGEDCOM(char* fileName, GEDCOMobject** obj) {

    int i;
    char **data = malloc(sizeof(char*));
    bool invalidGEDCOM = true;
    bool invalidFile = false;
    bool invalidRecord = false;
    bool invalidHeader = false;
    bool otherError = false;
    int line = 0;
    int lastLine = 0;
    bool carriageReturn = false;
    HTable *individualTable = createTable(10000, &hashNode, &dummyDelete, &printNodeData);
    HTable *familyTable = createTable(10000, &hashNode, &dummyDelete, &printNodeData);
    FILE* infile = openFile(fileName);
    if (infile == NULL) {
        invalidFile = true;
    }

    if (invalidFile) {
        GEDCOMerror gedcomError;
        gedcomError.type = INV_FILE;
        gedcomError.line = -1;
        return gedcomError;
    }

    i = 1;
    data[0] = malloc(sizeof(char) * 257);
    if (fileName[strlen(fileName) - 5] == 'R') {
        if (fileName[strlen(fileName) - 6] == '1') {
            carriageReturn = true;
        }
    }
    
    if (carriageReturn) {
        myfgets(data[0], 257, infile);
    }

    else {
        fgets(data[0], 257, infile);
    }

    if (data[0][strlen(data[0]) - 2] == '\r') {
        data[0][strlen(data[0]) - 2] = '\0';
    }

    if ((data[0][strlen(data[0])- 1] == '\n') || (data[0][strlen(data[i-1]) - 1] == '\r')) {
        data[0][strlen(data[0]) - 1] = '\0';
    }
    
    while (!feof(infile)) {
        ++i;
        data = realloc(data, sizeof(char*) * i);
        data[i - 1] = malloc(sizeof(char) * 257);
        if (carriageReturn) {
            myfgets(data[i-1], 257, infile);
        }

        else {
            fgets(data[i-1], 257, infile);
        }

        if (strlen(data[i - 1]) > 256) {
            invalidRecord = true;
            break;
        }

        if ((data[i-1][0] == '\r') || (data[i-1][0] == '\n')) {
            char tempString[50];
            int x;
            int y = 0;
            for (x = 0; x < strlen(data[i-1]); ++x) {
                if ((data[i-1][x] != '\r') && (data[i-1][x] != '\n')) {
                    tempString[y] = data[i-1][x];
                    ++y;
                }
            }
            tempString[y] = '\0';
            strcpy(data[i-1], tempString);
        }

        if (strlen(data[i-1]) > 1) {
            if ((data[i-1][strlen(data[i-1]) - 2] == '\r') || (data[i-1][strlen(data[i-1]) - 2] == '\n')) {
                data[i-1][strlen(data[i-1]) - 2] = '\0';
            }
        }

        if (strlen(data[i-1]) >= 1) {
            if ((data[i-1][strlen(data[i-1])- 1] == '\n') || (data[i-1][strlen(data[i-1]) - 1] == '\r')) {
                data[i-1][strlen(data[i-1]) - 1] = '\0';
            }
        }

    }

    fclose(infile);

    if (invalidRecord) {
        GEDCOMerror gedcomError;
        gedcomError.type = INV_RECORD;
        gedcomError.line = line;
        *obj = NULL;
        return gedcomError;
    }

    GEDCOMobject *object = malloc(sizeof(GEDCOMobject));
    object->families = initializeList(&printFamily, &deleteFamily, &compareFamilies);
    object->individuals = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);
    object->header = NULL;
    object->submitter = NULL;

    
    for (line = 0; line < i; ++line) {
        int x = 0;
        char *token;
        char** parsedData = malloc(sizeof(char*));
        token = strtok(data[line], " ");
        while (token != NULL) {
            parsedData = realloc(parsedData, sizeof(char*) * x + 2);
            parsedData[x] = malloc(sizeof(char) * 50);
            strcpy(parsedData[x], token);
            token = strtok(NULL, " ");
            ++x;
        }

        if (x < 1) {
            invalidGEDCOM = true;
            break;
        }


        if (strcmp(parsedData[0], "0") == 0) {

            if (strncmp(parsedData[1], "HEAD", 4) == 0) {
                if (object->header == NULL) {
                    Header *head = malloc(sizeof(Header));
                    strcpy(head->source, "");
                    head->encoding = -1;
                    head->submitter = NULL;
                    head->gedcVersion = -1.0;
                    head->otherFields = initializeList(&printField, &deleteField, &compareFields);
                    object->header = head;
                }

                else {
                    invalidGEDCOM = true;
                    break;
                }
            }

            else if (strncmp(parsedData[1], "TRLR", 4) == 0) {
                invalidGEDCOM = false;
                break;
            }

            else {
                if (2 <= (x - 1)) {
                    if (strncmp(parsedData[2], "INDI", 4) == 0) {
                        if ((parsedData[1][0] == '@') && (parsedData[1][strlen(parsedData[1]) - 1] == '@')) {
                            Individual *individual = malloc(sizeof(Individual));
                            individual->givenName = malloc(sizeof(char) + 2);
                            individual->surname = malloc(sizeof(char) + 2);
                            strcpy(individual->givenName, "");
                            strcpy(individual->surname, "");
                            individual->events = initializeList(&printEvent, &deleteEvent, &compareEvents);
                            individual->families = initializeList(&printFamily, &dummyDelete, &compareFamilies);
                            individual->otherFields = initializeList(&printField, &deleteField, &compareFields);
                            insertData(individualTable, parsedData[1], individual);
                        }

                        else {
                            invalidRecord = true;
                            break;
                        }
                    }

                    else if (strncmp(parsedData[2], "FAM", 3) == 0) {
                        if ((parsedData[1][0] == '@') && (parsedData[1][strlen(parsedData[1]) - 1] == '@')) {
                            Family *family = malloc(sizeof(Family));
                            family->husband = NULL;
                            family->wife = NULL;
                            family->events = initializeList(&printEvent, &deleteEvent, &compareEvents);
                            family->children = initializeList(&printIndividualName, &dummyDelete, &compareIndividuals);
                            family->otherFields = initializeList(&printField, &deleteField, &compareFields);
                            insertData(familyTable, parsedData[1], family);
                        }

                        else {
                            invalidRecord = true;
                            break;
                        }
                    }

                    else if (strncmp(parsedData[2], "SUBM", 4) == 0) {
                        if ((parsedData[1][0] == '@') && (parsedData[1][strlen(parsedData[1]) - 1] == '@')) {
                            if (object->submitter == NULL) {
                                Submitter *submitter = malloc(sizeof(Submitter) + sizeof(char) + 255);
                                strcpy(submitter->submitterName, "");
                                strcpy(submitter->address, "");
                                submitter->otherFields = initializeList(&printField, &deleteField, &compareFields);
                                object->submitter = submitter;
                            }
                        }

                        else {
                            invalidRecord = true;
                            break;
                        }
                    }

                    else {
                        invalidRecord = true;
                        break;
                    }
                }
            }
        }

        else if (strncmp(parsedData[1], "HEAD", 4) == 0) {
                invalidHeader = true;
                break;
            }
            
        else {
            if (!isdigit(parsedData[0][0])) {
                invalidRecord = true;
                break;
            }
        }

        int currLine = atoi(parsedData[0]);
        if (((currLine - lastLine) > 1) || (currLine > 2)) {
            invalidRecord = true;
            line += 1;
            break;
            int j;
            for (j = 0; j < x; ++j) {
                free(parsedData[j]);
            }
            free(parsedData);
        }
        lastLine = currLine;

        int j;
        for (j = 0; j < x; ++j) {
            free(parsedData[j]);
        }
        free(parsedData);
    }

    if ((object->header == NULL) || (object->submitter == NULL)) {
        invalidGEDCOM = true;
    }

    if (invalidRecord) {
        GEDCOMerror gedcomError;
        gedcomError.type = INV_RECORD;
        gedcomError.line = line;
        *obj = NULL;
        return gedcomError;
    }

    if (invalidHeader) {
        GEDCOMerror gedcomError;
        gedcomError.type = INV_HEADER;
        gedcomError.line = line;
        *obj = NULL;
        return gedcomError;
    }

    if (invalidGEDCOM) {
        GEDCOMerror gedcomError;
        gedcomError.type = INV_GEDCOM;
        gedcomError.line = -1;
        *obj = NULL;
        return gedcomError;
    }

    else if (invalidFile) {
        GEDCOMerror gedcomError;
        gedcomError.type = INV_FILE;
        gedcomError.line = line;
        *obj = NULL;
        return gedcomError;
    }

    //Second pass through
    bool isHeader = false;
    bool isEvent = false;
    bool isFamily = false;
    bool isIndividual = false;
    bool isSubmitter = false;
    bool gedc = false;
    infile = openFile(fileName);
    data = malloc(sizeof(char*));

    i = 1;
    data[0] = malloc(sizeof(char) * 257);
    if (carriageReturn) {
        myfgets(data[0], 257, infile);
    }

    else {
        fgets(data[0], 257, infile);
    }

    if (data[0][strlen(data[0]) - 2] == '\r') {
        data[0][strlen(data[0]) - 2] = '\0';
    }

    if ((data[0][strlen(data[0])- 1] == '\n') || (data[0][strlen(data[i-1]) - 1] == '\r')) {
        data[0][strlen(data[0]) - 1] = '\0';
    }

    while (!feof(infile)) {
        ++i;
        data = realloc(data, sizeof(char*) * i);
        data[i - 1] = malloc(sizeof(char) * 258);
        if (carriageReturn) {
            myfgets(data[i-1], 257, infile);
        }

        else {
            fgets(data[i-1], 257, infile);
        }
        if (strlen(data[i - 1]) > 257) {
            invalidRecord = true;
            break;
        }

        if ((data[i-1][0] == '\r') || (data[i-1][0] == '\n')) {
            char tempString[50];
            int x;
            int y = 0;
            for (x = 0; x < strlen(data[i-1]); ++x) {
                if ((data[i-1][x] != '\r') && (data[i-1][x] != '\n')) {
                    tempString[y] = data[i-1][x];
                    ++y;
                }
            }
            tempString[y] = '\0';
            strcpy(data[i-1], tempString);
        }

        if (strlen(data[i-1]) > 1) {
            if ((data[i-1][strlen(data[i-1]) - 2] == '\r') || (data[i-1][strlen(data[i-1]) - 2] == '\n')) {
                data[i-1][strlen(data[i-1]) - 2] = '\0';
            }
        }

        if (strlen(data[i-1]) >= 1) {
            if ((data[i-1][strlen(data[i-1])- 1] == '\n') || (data[i-1][strlen(data[i-1]) - 1] == '\r')) {
                data[i-1][strlen(data[i-1]) - 1] = '\0';
            }
        }
    }

    fclose(infile);

    Family* family = NULL;
    Event* event = NULL;
    Individual* individual = NULL;
    char* lastString = NULL;

    for (line = 0; line < i; ++line) {
        int x = 0;
        char *token;
        char** parsedData = malloc(sizeof(char*));
        token = strtok(data[line], " ");
        while (token != NULL) {
            parsedData = realloc(parsedData, sizeof(char*) * x + 2);
            parsedData[x] = malloc(sizeof(char) * 50);
            strcpy(parsedData[x], token);
            token = strtok(NULL, " ");
            ++x;
        }

        if (isFamily) {
            family = getFromBack(object->families);

            if (isEvent) {
                event = getFromBack(family->events);
            }
        }

        else if (isIndividual) {
            individual = getFromBack(object->individuals);

            if (isEvent) {
                event = getFromBack(individual->events);
            }
        }

        if (strcmp(parsedData[0], "0") == 0) {

            if (isHeader) {
                isHeader = false;
            }

            else if (isIndividual) {
                isIndividual = false;
            }

            else if (isFamily) {
                isFamily = false;
            }

            else if (isSubmitter) {
                isSubmitter = false;

            }

            if (strncmp(parsedData[1], "HEAD", 4) == 0) {
                isHeader = true;
            }

            else if (strncmp(parsedData[1], "TRLR", 4) == 0) {
                break;
            }

            else {
                if (2 <= (x - 1)) {
                    if (strncmp(parsedData[2], "INDI", 4) == 0) {
                        isIndividual = true;
                        Individual *newIndividual = lookupData(individualTable, parsedData[1]);
                        insertBack(&object->individuals, newIndividual);
                    }

                    else if (strncmp(parsedData[2], "FAM", 3) == 0) {
                        isFamily = true;
                        Family* newFamily = lookupData(familyTable, parsedData[1]);
                        insertBack(&object->families, newFamily);
                    }

                    else if (strncmp(parsedData[2], "SUBM", 4) == 0) {
                        isSubmitter = true;
                    }
                }
            }
        }

        else if (strcmp(parsedData[0], "1") == 0) {

            if (1 <= (x - 1)) {

                if (isHeader) {

                    if (gedc) {
                        gedc = false;
                    }

                    if (strcmp(parsedData[1], "SOUR") == 0) {

                        if (2 <= (x - 1)) {
                            strcpy(object->header->source, parsedData[2]);
                        }

                        else {
                            invalidHeader = true;
                            break;
                        }
                    }

                    else if (strncmp(parsedData[1], "CHAR", 4) == 0) {
                        if (2 <= (x - 1)) {
                            if (strncmp(parsedData[2], "UTF-8", 4) == 0) {
                                object->header->encoding = UTF8;
                            }

                            else if (strncmp(parsedData[2], "ANSEL", 4) == 0) {
                                object->header->encoding = ANSEL;
                            }

                            else if (strncmp(parsedData[2], "ASCII", 4) == 0) {
                                object->header->encoding = ASCII;
                            }

                            else if (strncmp(parsedData[2], "UNICODE", 4) == 0) {
                                object->header->encoding = UNICODE;
                            }

                            else {
                                invalidHeader = true;
                                break;
                            }
                        }

                        else {
                            invalidHeader = true;
                            break;
                        }
                    }

                    else if (strncmp(parsedData[1], "SUBM", 4) == 0) {

                        if (2 <= (x - 1)) {
                            if ((parsedData[2][0] == '@') && (parsedData[2][strlen(parsedData[2]) - 1] == '@')) {
                                object->header->submitter = object->submitter;
                            }

                            else if (parsedData[2][strlen(parsedData[2]) - 1] == '\r') {
                                if ((parsedData[2][0] == '@') && (parsedData[2][strlen(parsedData[2]) - 2] == '@')) {
                                    object->header->submitter = object->submitter;
                                }
                            }
                        }

                        else {
                            invalidHeader = true;
                            break;
                        }
                    }

                    else if (strncmp(parsedData[1], "GEDC", 4) == 0) {
                        gedc = true;
                    }

                    else {
                        if (2 <= (x - 1)) {
                            Field *otherField = malloc(sizeof(Field));
                            otherField->tag = malloc(sizeof(char) * strlen(parsedData[1]) + 2);
                            strcpy(otherField->tag, parsedData[1]);
                            int j;
                            int length = 0;
                            char* string = malloc(sizeof(char) * strlen(parsedData[2]) + 2);
                            strcpy(string, parsedData[2]);
                            for (j = 1; (2 + j) < x; ++j) {
                                length += strlen(parsedData[2 + j]);
                                string = realloc(string, sizeof(char) * (strlen(parsedData[2]) +  1 + length + 3));
                                strcat(string, " ");
                                strcat(string, parsedData[2 + j]);
                            }
                            otherField->value = malloc(sizeof(char) * strlen(string) + 2);
                            strcpy(otherField->value, string);
                            lastString = otherField->value;
                            free(string);
                            insertBack(&object->header->otherFields, otherField);
                        }
                        
                        else {
                            invalidHeader = true;
                            break;
                        }
                    }

                }

                else if (isIndividual) {

                    if (isEvent) {
                        isEvent = false;
                    }
                    
                    if (strncmp(parsedData[1], "NAME", 4) == 0) {

                        if (2 <= (x - 1)) {

                            if (parsedData[2][0] == '/') {
                                int j;
                                int length = 0;
                                char* string = malloc(sizeof(char) * strlen(parsedData[2]) + 3);
                                strcpy(string, parsedData[2]);
                                if (parsedData[2][strlen(parsedData[2]) - 1] != '/') {
                                    for (j = 1; (2 + j) < x; ++j) {
                                        if (parsedData[2 + j][strlen(parsedData[2 + j]) - 1] == '/') {
                                            break;
                                        }

                                        if (parsedData[2 + j][strlen(parsedData[2 + j]) - 1] == '\r') {
                                            if (parsedData[2 + j][strlen(parsedData[2 + j]) - 2] == '/') {
                                                break;
                                            }
                                        }
                                        length += strlen(parsedData[2 + j]);
                                        string = realloc(string, sizeof(char) * (strlen(parsedData[2]) + length + 3));
                                        strcat(string, " ");
                                        strcat(string, parsedData[2 + j]);
                                    }
                                }
                                char *newString = malloc(sizeof(char) * strlen(string) + 2);
                                int i;
                                j = 0;
                                for (i = 0; i < strlen(string); ++i) {
                                    if (string[i] != '/') {
                                        newString[j] = string[i];
                                        ++j;
                                    }
                                }
                                newString[j] = '\0';
                                individual->surname = realloc(individual->surname, sizeof(char) * strlen(newString) + 3);
                                strcpy(individual->surname, newString);
                                lastString = individual->surname;
                                free(string);
                                free(newString);
                            }
                            

                            else {
                                int j;
                                int length = 0;
                                char* string = malloc(sizeof(char) * strlen(parsedData[2]) + 2);
                                strcpy(string, parsedData[2]);
                                for (j = 1; (2 + j) < x; ++j) {
                                    if (parsedData[2 + j][0] == '/') {
                                        break;
                                    }

                                    length += strlen(parsedData[2 + j]);
                                    string = realloc(string, sizeof(char) * (strlen(parsedData[2]) + length + 3));
                                    strcat(string, " ");
                                    strcat(string, parsedData[2 + j]);
                                }
                                individual->givenName = realloc(individual->givenName, sizeof(char) * strlen(string) + 2);
                                strcpy(individual->givenName, string);
                                free(string);

                                char* string2 = malloc(sizeof(char) * strlen(parsedData[2 + j]) + 2);
                                strcpy(string2, parsedData[2 + j]);
                                length = 0;
                                int k;
                                for (k = j; (2 + k) < x; ++k) {
                                    if (parsedData[2 + k][strlen(parsedData[2 + k]) - 1] == '/') {
                                        break;
                                    }

                                    if (parsedData[2 + k][strlen(parsedData[2 + k]) - 1] == '\r') {
                                        if (parsedData[2 + k][strlen(parsedData[2 + k]) - 2] == '/') {
                                            break;
                                        }
                                    }
                                    length += strlen(parsedData[2 + k]);
                                    string2 = realloc(string2, sizeof(char) * (strlen(parsedData[2 + k]) + length + 3));
                                    strcat(string2, " ");
                                    strcat(string2, parsedData[2 + k]);
                                }

                                char *newString = malloc(sizeof(char) * strlen(string2) + 2);
                                int i;
                                int z = 0;
                                for (i = 0; i < strlen(string2); ++i) {
                                    if (string2[i] != '/') {
                                        newString[z] = string2[i];
                                        ++z;
                                    }
                                }
                                newString[z] = '\0';
                                individual->surname = realloc(individual->surname, sizeof(char) * strlen(newString) + 2);
                                strcpy(individual->surname, newString);
                                lastString = individual->surname;
                                free(string2);
                                free(newString);
                            }
                        }

                        else {
                            invalidRecord = true;
                            break;
                        }
                    }

                    else if ((strncmp(parsedData[1], "FAMC", 4) == 0) || (strncmp(parsedData[1], "FAMS", 4) == 0)) {
                        if (2 <= (x - 1)) {
                            if ((parsedData[2][0] == '@') && (parsedData[2][strlen(parsedData[2]) - 1] == '@')) {
                                Family *newFamily = lookupData(familyTable, parsedData[2]);
                                insertBack(&individual->families, newFamily);
                            }

                            else if (parsedData[2][strlen(parsedData[2]) - 1] == '\r') {
                                if ((parsedData[2][0] == '@') && (parsedData[2][strlen(parsedData[2]) - 2] == '@')) {
                                    Family *newFamily = lookupData(familyTable, parsedData[2]);
                                    insertBack(&individual->families, newFamily);
                                }
                            }

                            else {
                                invalidRecord = true;
                                break;
                            }
                        }
                    }

                    else if ((strncmp(parsedData[1], "BIRT", 4) == 0) || (strncmp(parsedData[1], "CHR", 3) == 0) ||
                            (strncmp(parsedData[1], "DEAT", 4) == 0) || (strncmp(parsedData[1], "BURI", 4) == 0) ||
                            (strncmp(parsedData[1], "ADOP", 4) == 0) || (strncmp(parsedData[1], "BAPM", 4) == 0) ||
                            (strncmp(parsedData[1], "BARM", 4) == 0) || (strncmp(parsedData[1], "BASM", 4) == 0) ||
                            (strncmp(parsedData[1], "CHRA", 4) == 0) || (strncmp(parsedData[1], "CONF", 4) == 0) ||
                            (strncmp(parsedData[1], "FCOM", 4) == 0) || (strncmp(parsedData[1], "ORDN", 4) == 0) ||
                            (strncmp(parsedData[1], "NATU", 4) == 0) || (strncmp(parsedData[1], "EMIG", 4) == 0) ||
                            (strncmp(parsedData[1], "IMMI", 4) == 0) || (strncmp(parsedData[1], "CENS", 4) == 0) ||
                            (strncmp(parsedData[1], "PROB", 4) == 0) || (strncmp(parsedData[1], "WILL", 4) == 0) ||
                            (strncmp(parsedData[1], "GRAD", 4) == 0) || (strncmp(parsedData[1], "RETI", 4) == 0) ||
                            (strncmp(parsedData[1], "EVEN", 4) == 0)) {
                                isEvent = true;
                                Event* newEvent = malloc(sizeof(Event));
                                strcpy(newEvent->type, parsedData[1]);
                                newEvent->date = malloc(sizeof(char) + 2);
                                newEvent->place = malloc(sizeof(char) + 2);
                                strcpy(newEvent->date, "");
                                strcpy(newEvent->place, "");
                                newEvent->otherFields = initializeList(&printField, &deleteField, &compareFields);
                                insertBack(&individual->events, newEvent);
                    }

                    else {
                        if (2 <= (x - 1)) {
                            Field *otherField = malloc(sizeof(Field));
                            otherField->tag = malloc(sizeof(char) * strlen(parsedData[1]) + 2);
                            strcpy(otherField->tag, parsedData[1]);
                            int j;
                            int length = 0;
                            char* string = malloc(sizeof(char) * strlen(parsedData[2]) + 2);
                            strcpy(string, parsedData[2]);
                            for (j = 1; (2 + j) < x; ++j) {
                                length += strlen(parsedData[2 + j]);
                                string = realloc(string, sizeof(char) * (strlen(parsedData[2]) +  1 + length + 3));
                                strcat(string, " ");
                                strcat(string, parsedData[2 + j]);
                            }
                            otherField->value = malloc(sizeof(char) * strlen(string) + 2);
                            strcpy(otherField->value, string);
                            lastString = otherField->value;
                            free(string);
                            insertBack(&individual->otherFields, otherField);
                        }
                        
                        else {
                            invalidRecord = true;
                            break;
                        }
                    }
                }

                else if (isFamily) {

                    if (isEvent) {
                        isEvent = false;
                    }
                    
                    if (strncmp(parsedData[1], "HUSB", 4) == 0) {
                         if (2 <= (x - 1)) {
                            if ((parsedData[2][0] == '@') && (parsedData[2][strlen(parsedData[2]) - 1] == '@')) {
                                Individual *husband = lookupData(individualTable, parsedData[2]);
                                family->husband = husband;
                                
                            }

                            else if (parsedData[2][strlen(parsedData[2]) - 1] == '\r') {
                                if ((parsedData[2][0] == '@') && (parsedData[2][strlen(parsedData[2]) - 2] == '@')) {
                                    Individual *husband = lookupData(individualTable, parsedData[2]);
                                    family->husband = husband;
                                }
                            }
                         }
                    }

                    else if (strncmp(parsedData[1], "WIFE", 4) == 0) {
                         if (2 <= (x - 1)) {
                            if ((parsedData[2][0] == '@') && (parsedData[2][strlen(parsedData[2]) - 1] == '@')) {
                                Individual * wife = lookupData(individualTable, parsedData[2]);
                                family->wife = wife;
                            }

                            else if (parsedData[2][strlen(parsedData[2]) - 1] == '\r') {
                                if ((parsedData[2][0] == '@') && (parsedData[2][strlen(parsedData[2]) - 2] == '@')) {
                                    Individual *wife = lookupData(individualTable, parsedData[2]);
                                    family->wife = wife;
                                }
                            }
                         }
                    }

                    else if (strncmp(parsedData[1], "CHIL", 4) == 0) {
                         if (2 <= (x - 1)) {
                            if ((parsedData[2][0] == '@') && (parsedData[2][strlen(parsedData[2]) - 1] == '@')) {
                                Individual *child = lookupData(individualTable, parsedData[2]);
                                insertBack(&family->children, child);
                            }

                            else if (parsedData[2][strlen(parsedData[2]) - 1] == '\r') {
                                if ((parsedData[2][0] == '@') && (parsedData[2][strlen(parsedData[2]) - 2] == '@')) {
                                    Individual *child = lookupData(individualTable, parsedData[2]);
                                    insertBack(&family->children, child);
                                }
                            }
                         }
                    }
                    
                    else if ((strncmp(parsedData[1], "ANUL", 4) == 0) || (strncmp(parsedData[1], "CENS", 4) == 0) ||
                            (strncmp(parsedData[1], "DIV", 3) == 0) || (strncmp(parsedData[1], "DIVF", 4) == 0) ||
                            (strncmp(parsedData[1], "ENGA", 4) == 0) || (strncmp(parsedData[1], "MARB", 4) == 0) ||
                            (strncmp(parsedData[1], "MARC", 4) == 0) || (strncmp(parsedData[1], "MARR", 4) == 0) ||
                            (strncmp(parsedData[1], "RESI", 4) == 0) || (strncmp(parsedData[1], "EVEN", 4) == 0)) {
                                isEvent = true;
                                Event* newEvent = malloc(sizeof(Event));
                                strcpy(newEvent->type, parsedData[1]);
                                newEvent->date = malloc(sizeof(char) + 2);
                                newEvent->place = malloc(sizeof(char) + 2);
                                strcpy(newEvent->date, "");
                                strcpy(newEvent->place, "");
                                newEvent->otherFields = initializeList(&printField, &deleteField, &compareFields);
                                insertBack(&family->events, newEvent);
                    }

                    else {
                        if (2 <= (x - 1)) {
                            Field *otherField = malloc(sizeof(Field));
                            otherField->tag = malloc(sizeof(char) * strlen(parsedData[1]) + 2);
                            strcpy(otherField->tag, parsedData[1]);
                            int j;
                            int length = 0;
                            char* string = malloc(sizeof(char) * strlen(parsedData[2]) + 2);
                            strcpy(string, parsedData[2]);
                            for (j = 1; (2 + j) < x; ++j) {
                                length += strlen(parsedData[2 + j]);
                                string = realloc(string, sizeof(char) * (strlen(parsedData[2]) +  1 + length + 3));
                                strcat(string, " ");
                                strcat(string, parsedData[2 + j]);
                            }
                            otherField->value = malloc(sizeof(char) * strlen(string) + 2);
                            strcpy(otherField->value, string);
                            free(string);
                            insertBack(&family->otherFields, otherField);
                        }
                    }
                }

                else if (isSubmitter) {
                    if (strncmp(parsedData[1], "NAME", 4) == 0) {
                        if (2 <= (x - 1)) {
                            strcpy(object->submitter->submitterName, parsedData[2]);
                        }
                    }

                    else if (strncmp(parsedData[1], "ADDR", 4) == 0) {
                        if (2 <= (x - 1)) {
                            int j;
                            int length = strlen(parsedData[2]);
                            object->submitter = realloc(object->submitter, sizeof(Submitter) +  length * sizeof(char)+ 3);
                            strcpy(object->submitter->address, parsedData[2]);
                            for (j = 1; (2 + j) < x; ++j) {
                                length += strlen(parsedData[2 + j]);
                                object->submitter = realloc(object->submitter, sizeof(Submitter) +  length * sizeof(char) + 3);
                                strcat(object->submitter->address, " ");
                                strcat(object->submitter->address, parsedData[2 + j]);
                            }
                            lastString = object->submitter->address;
                        }
                    }

                    else {
                        if (2 <= (x - 1)) {
                            Field *otherField = malloc(sizeof(Field));
                            otherField->tag = malloc(sizeof(char) * strlen(parsedData[1]) + 2);
                            strcpy(otherField->tag, parsedData[1]);
                            int j;
                            int length = 0;
                            char* string = malloc(sizeof(char) * strlen(parsedData[2]) + 2);
                            strcpy(string, parsedData[2]);
                            for (j = 1; (2 + j) < x; ++j) {
                                length += strlen(parsedData[2 + j]);
                                string = realloc(string, sizeof(char) * (strlen(parsedData[2]) +  1 + length + 3));
                                strcat(string, " ");
                                strcat(string, parsedData[2 + j]);
                            }
                            otherField->value = malloc(sizeof(char) * strlen(string) + 2);
                            strcpy(otherField->value, string);
                            lastString = otherField->value;
                            free(string);
                            insertBack(&object->submitter->otherFields, otherField);
                        }
                    }
                }
            }

            else {
                invalidHeader = true;
                break;
            }
        }

        else if (strcmp(parsedData[0], "2") == 0) {

            if (1 <= (x - 1)) {

                if (isHeader) {

                    if (gedc) {

                        if (strncmp(parsedData[1], "VERS", 4) == 0) {
                            if (2 <= (x - 1)) {
                                object->header->gedcVersion = atof(parsedData[2]);
                            }

                            else {
                                invalidHeader = true;
                                break;
                            }
                        }


                        else {
                            Field *otherField = malloc(sizeof(Field));
                            otherField->tag = malloc(sizeof(char) * strlen(parsedData[1]) + 2);
                            strcpy(otherField->tag, parsedData[1]);
                            int j;
                            int length = 0;
                            char* string = malloc(sizeof(char) * strlen(parsedData[2]) + 2);
                            strcpy(string, parsedData[2]);
                            for (j = 1; (2 + j) < x; ++j) {
                                length += strlen(parsedData[2 + j]);
                                string = realloc(string, sizeof(char) * (strlen(parsedData[2]) +  1 + length + 3));
                                strcat(string, " ");
                                strcat(string, parsedData[2 + j]);
                            }
                            otherField->value = malloc(sizeof(char) * strlen(string) + 2);
                            strcpy(otherField->value, string);
                            free(string);
                            insertBack(&object->header->otherFields, otherField);
                        }

                    }

                    else if ((strncmp(parsedData[1], "CONT", 4) == 0) || (strncmp(parsedData[1], "CONC", 4)) == 0) {
                        if (2 <= (x - 1)) {
                            int j;
                            int length = 0;
                            char* string = malloc(sizeof(char) * strlen(parsedData[2]) + 2);
                            strcpy(string, parsedData[2]);
                            for (j = 1; (2 + j) < x; ++j) {
                                length += strlen(parsedData[2 + j]);
                                string = realloc(string, sizeof(char) * (strlen(parsedData[2]) +  1 + length + 3));
                                strcat(string, " ");
                                strcat(string, parsedData[2 + j]);
                            }
                            lastString = realloc(lastString, sizeof(char) * (strlen(lastString) + 1 + strlen(string) + 2));
                            strcat(lastString, string);
                            free(string);
                        }
                    }


                    else {
                        if (2 <= (x - 1)) {
                            Field *otherField = malloc(sizeof(Field));
                            otherField->tag = malloc(sizeof(char) * strlen(parsedData[1]) + 2);
                            strcpy(otherField->tag, parsedData[1]);
                            int j;
                            int length = 0;
                            char* string = malloc(sizeof(char) * strlen(parsedData[2]) + 2);
                            strcpy(string, parsedData[2]);
                            for (j = 1; (2 + j) < x; ++j) {
                                length += strlen(parsedData[2 + j]);
                                string = realloc(string, sizeof(char) * (strlen(parsedData[2]) +  1 + length + 3));
                                strcat(string, " ");
                                strcat(string, parsedData[2 + j]);
                            }
                            otherField->value = malloc(sizeof(char) * strlen(string) + 2);
                            strcpy(otherField->value, string);
                            free(string);
                            insertBack(&object->header->otherFields, otherField);
                        }
                    }
                }

                else if (isEvent) {

                    if (strncmp(parsedData[1], "DATE", 4) == 0) {

                        if (2 <= (x - 1)) {
                            int j;
                            int length = 0;
                            char* string = malloc(sizeof(char) * strlen(parsedData[2]) + 2);
                            strcpy(string, parsedData[2]);
                            for (j = 1; (2 + j) < x; ++j) {
                                length += strlen(parsedData[2 + j]);
                                string = realloc(string, sizeof(char) * (strlen(parsedData[2]) + length + 4));
                                strcat(string, " ");
                                strcat(string, parsedData[2 + j]);
                            }
                            event->date = realloc(event->date, sizeof(char) * strlen(string) + 2);
                            strcpy(event->date, string);
                            free(string);
                        }
                    }

                    else if (strncmp(parsedData[1], "PLAC", 4) == 0) {

                        if (2 <= (x - 1)) {
                            int j;
                            int length = 0;
                            char* string = malloc(sizeof(char) * strlen(parsedData[2]) + 2);
                            strcpy(string, parsedData[2]);
                            for (j = 1; (2 + j) < x; ++j) {
                                length += strlen(parsedData[2 + j]);
                                string = realloc(string, sizeof(char) * (strlen(parsedData[2]) + length + 4));
                                strcat(string, " ");
                                strcat(string, parsedData[2 + j]);
                            }
                            event->place = realloc(event->place, sizeof(char) * strlen(string) + 2);
                            strcpy(event->place, string);
                            free(string);
                        }
                    }

                    else {
                        if (2 <= (x - 1)) {
                            Field *otherField = malloc(sizeof(Field));
                            otherField->tag = malloc(sizeof(char) * strlen(parsedData[1]) + 2);
                            strcpy(otherField->tag, parsedData[1]);
                            int j;
                            int length = 0;
                            char* string = malloc(sizeof(char) * strlen(parsedData[2]) + 2);
                            strcpy(string, parsedData[2]);
                            for (j = 1; (2 + j) < x; ++j) {
                                length += strlen(parsedData[2 + j]);
                                string = realloc(string, sizeof(char) * (strlen(parsedData[2]) +  1 + length + 3));
                                strcat(string, " ");
                                strcat(string, parsedData[2 + j]);
                            }
                            otherField->value = malloc(sizeof(char) * strlen(string) + 2);
                            strcpy(otherField->value, string);
                            free(string);
                            insertBack(&event->otherFields, otherField);
                        }
                    }
                }

                else if (isIndividual) {
                    if (2 <= (x - 1)) {
                        Field *otherField = malloc(sizeof(Field));
                        otherField->tag = malloc(sizeof(char) * strlen(parsedData[1]) + 2);
                        strcpy(otherField->tag, parsedData[1]);
                        int j;
                        int length = 0;
                        char* string = malloc(sizeof(char) * strlen(parsedData[2]) + 2);
                        strcpy(string, parsedData[2]);
                        for (j = 1; (2 + j) < x; ++j) {
                            length += strlen(parsedData[2 + j]);
                            string = realloc(string, sizeof(char) * (strlen(parsedData[2]) +  1 + length + 3));
                            strcat(string, " ");
                            strcat(string, parsedData[2 + j]);
                        }
                        otherField->value = malloc(sizeof(char) * strlen(string) + 2);
                        strcpy(otherField->value, string);
                        free(string);
                        insertBack(&individual->otherFields, otherField);
                    }
                }
            }

            else {
                invalidRecord = true;
                break;
            }
        }

        int j;
        for (j = 0; j < x; ++j) {
            free(parsedData[j]);
        }
        free(parsedData);
    }

    destroyTable(familyTable);
    destroyTable(individualTable);

    if ((object->header->encoding == -1) || (object->header->submitter == NULL) || (strcmp(object->header->source, "") == 0) || (object->header->gedcVersion == -1.0)) {
        invalidHeader = true;
    }

    if (invalidRecord) {
        GEDCOMerror gedcomError;
        gedcomError.type = INV_RECORD;
        gedcomError.line = line;
        *obj = NULL;
        return gedcomError;
    }

    else if (invalidHeader) {
        GEDCOMerror gedcomError;
        gedcomError.type = INV_HEADER;
        gedcomError.line = line;
        *obj = NULL;
        return gedcomError;
    }

    else if (otherError) {
        GEDCOMerror gedcomError;
        gedcomError.type = OTHER_ERROR;
        gedcomError.line = line;
        return gedcomError;
    }

    else {
        GEDCOMerror gedcomError;
        gedcomError.type = OK;
        gedcomError.line = -1;
        *obj = object;
        return gedcomError;
    }

}

char* printGEDCOM(const GEDCOMobject* obj) {

    int length = 8;
    char *printString = malloc(sizeof(char) * length);
    
    char gedcVersion[10];

    if (obj == NULL) {
        char *printString = malloc(sizeof(char) * 20);
        strcpy(printString, "Error: NULL object\n");
        return printString;
    }

    strcpy(printString, "Source: ");
    length += (strlen(obj->header->source));
    printString = realloc(printString, sizeof(char) * length + 3);
    strncat(printString, obj->header->source, strlen(obj->header->source));
    strcat(printString, "\n");
    sprintf(gedcVersion, "%.1f", obj->header->gedcVersion);
    length += 22;
    length += strlen(gedcVersion);
    printString = realloc(printString, sizeof(char) * length + 3);
    strcat(printString, "GEDCOM Version: ");
    strncat(printString, gedcVersion, strlen(gedcVersion));
    strcat(printString, "\n");
    length += 11;
    printString = realloc(printString, sizeof(char) * length + 3);
    strcat(printString, "Encoding: ");

    switch(obj->header->encoding) {
        case ANSEL:
            length += 6;
            printString = realloc(printString, sizeof(char) * length + 3);
            strcat(printString, "ANSEL\n");
            break;

        case UTF8:
            length += 6;
            printString = realloc(printString, sizeof(char) * length + 3);
            strcat(printString, "UTF-8\n");
            break;

        case UNICODE:
            length += 8;
            printString = realloc(printString, sizeof(char) * length + 3);
            strcat(printString, "UNICODE\n");
            break;

        case ASCII:
            length += 6;
            printString = realloc(printString, sizeof(char) * length + 3);
            strcat(printString, "ASCII\n");
            break;

        default:
            break;
    }

    length += 12;
    length += strlen(obj->submitter->submitterName);
    printString = realloc(printString, sizeof(char) * length + 3);
    strcat(printString, "Submitter: ");
    strncat(printString, obj->submitter->submitterName, strlen(obj->submitter->submitterName));
    strcat(printString, "\n");

    if (obj->header->otherFields.length != 0) {
        
        length += strlen(toString(obj->header->otherFields));
        length += 14;
        printString = realloc(printString, sizeof(char) * length + 3);
        strcat(printString, "Other Fields:\n");
        strcat(printString, toString(obj->header->otherFields));
        strcat(printString, "\n");
    }

    if (obj->families.length != 0) {
        length += strlen(toString(obj->families));
        printString = realloc(printString, sizeof(char) * length + 3);
        strcat(printString, toString(obj->families));
        strcat(printString, "\n");
    }

    if (obj->individuals.length != 0) {
        length += strlen(toString(obj->individuals));
        printString = realloc(printString, sizeof(char) * length + 3);
        strcat(printString, toString(obj->individuals));
        strcat(printString, "\n");
    }


    length += 16;
    length += strlen(obj->submitter->submitterName);
    printString = realloc(printString, sizeof(char) * length + 3);
    strcat(printString, "Submitter Name: ");
    strncat(printString, obj->submitter->submitterName, strlen(obj->submitter->submitterName));
    strcat(printString, "\n");

    if (obj->submitter->otherFields.length != 0) {
        length += strlen(toString(obj->submitter->otherFields));
        length += 14;
        printString = realloc(printString, sizeof(char) * length + 3);
        strcat(printString, "Other Fields:\n");
        strcat(printString, toString(obj->submitter->otherFields));
        strcat(printString, "\n");
    }
    length += 10;
    length += strlen(obj->submitter->address);
    printString = realloc(printString, sizeof(char) * length + 3);
    strcat(printString, "Address: ");
    strncat(printString, obj->submitter->address, strlen(obj->submitter->address));
    strcat(printString, "\n");

    return printString;

}

void deleteGEDCOM(GEDCOMobject* obj) {

    if (obj == NULL) {
        return;
    }

    if (obj->header != NULL) {
        free(obj->header);
    }

    if (obj->individuals.length != 0) {
        //clearList(&obj->individuals);
    }

    if (obj->families.length != 0) {
        //clearList(&obj->families);
    }

    if (obj->submitter != NULL) {
        free(obj->submitter);
    }

    free(obj);
}

char* printError(GEDCOMerror err) {

    char* error;
    char line[10];
    sprintf(line, "%d", err.line);

    switch(err.type) {
        case OK:
            error = malloc(sizeof(char) * 2);
            strcpy(error, "OK");
            break;

        case INV_FILE:
            error = malloc(sizeof(char) *  20 + strlen(line) + 2);
            strcpy(error, "invalid file (line ");
            strncat(error, line, strlen(line));
            strcat(error, ")");
            break;

        case INV_GEDCOM:
            error = malloc(sizeof(char) *  21 + strlen(line) + 2);
            strcpy(error, "invalid GEDCOM (line ");
            strncat(error, line, strlen(line));
            strcat(error, ")");
            break;

        case INV_HEADER:
            error = malloc(sizeof(char) *  21 + strlen(line) + 2);
            strcpy(error, "invalid header (line ");
            strncat(error, line, strlen(line));
            strcat(error, ")");
            break;

        case INV_RECORD:
            error = malloc(sizeof(char) *  21 + strlen(line) + 2);
            strcpy(error, "invalid record (line ");
            strncat(error, line, strlen(line));
            strcat(error, ")");
            break;

        case OTHER_ERROR:
            error = malloc(sizeof(char) *  12 + strlen(line) + 2);
            strcpy(error, "other (line ");
            strncat(error, line, strlen(line));
            strcat(error, ")");
            break;

        case WRITE_ERROR:
            error = malloc(sizeof(char) *  21 + strlen(line) + 2);
            strcpy(error, "write error (line ");
            strncat(error, line, strlen(line));
            strcat(error, ")");
            break;

        default:
            error = malloc(sizeof(char) * 14);
            strcpy(error, "error printing");
            break;

    }

    return error;
}

Individual* findPerson(const GEDCOMobject* familyRecord, bool (*compare)(const void* first, const void* second), const void* person) {
    if (familyRecord == NULL) {
        return NULL;
    }

    if (person == NULL) {
        return NULL;
    }
    
    return findElement(familyRecord->individuals, compare, person);
}

List getDescendants(const GEDCOMobject* familyRecord, const Individual* person) {
    List list = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);

    if (person == NULL) {
        return list;
    }

    if (familyRecord == NULL) {
        return list;
    }

    ListIterator iter = createIterator(person->families);
	Family* family;

    if (person->families.length != 0) {
        while( (family = nextElement(&iter)) != NULL) {

            if ((compareIndividuals(family->husband, person) == 0) || (compareIndividuals(family->wife, person) == 0)) {
                ListIterator iter = createIterator(family->children);
                Individual *child;
                if (family->children.length != 0) {
                    while( (child = nextElement(&iter)) != NULL) {
                        list = getDescendantsRecursive(list, familyRecord, child);
                    }
                }
            }

        }
        return list;
    }

    return list;
}

List getDescendantsRecursive(List descendantList, const GEDCOMobject *object, Individual *individual) {

    Individual *individualCopy = malloc(sizeof(Individual));
    individualCopy->givenName = malloc(sizeof(char) * strlen(individual->givenName) + 1);
    strcpy(individualCopy->givenName, individual->givenName);
    individualCopy->surname = malloc(sizeof(char) * strlen(individual->surname) + 1);
    strcpy(individualCopy->surname, individual->surname);
    individualCopy->families = initializeList(&printFamily, &dummyDelete, &compareFamilies);
    ListIterator familyiterCopy = createIterator(individual->families);
	Family* familyCopy;
    while( (familyCopy = nextElement(&familyiterCopy)) != NULL) {
        insertBack(&individualCopy->families, familyCopy);
    }
    individualCopy->events = initializeList(&printEvent, &dummyDelete, &compareEvents);
    ListIterator eventIterCopy = createIterator(individual->events);
    Event* eventCopy;
    while( (eventCopy = nextElement(&eventIterCopy)) != NULL) {
        insertBack(&individualCopy->events, eventCopy);
    }
    individualCopy->otherFields = initializeList(&printField, &dummyDelete, &compareFields);
    ListIterator fielditerCopy = createIterator(individual->otherFields);
    Field* fieldCopy;
    while( (fieldCopy = nextElement(&fielditerCopy)) != NULL) {
        insertBack(&individualCopy->otherFields, fieldCopy);
    }
    insertBack(&descendantList, individualCopy);

    ListIterator iter = createIterator(individual->families);
	Family* family;

    if (individual->families.length != 0) {
        while( (family = nextElement(&iter)) != NULL) {

            if ((compareIndividuals(family->husband, individual) == 0) || (compareIndividuals(family->wife, individual) == 0)) {
                ListIterator iter = createIterator(family->children);
                Individual *child;
                if (family->children.length != 0) {
                    bool add = true;
                    while( (child = nextElement(&iter)) != NULL) {
                        //Check if individual already exists in list
                        ListIterator iterator = createIterator(descendantList);
                        Individual* ind;

                        if (descendantList.length != 0) {
                            while( (ind = nextElement(&iterator)) != NULL) {
                                if (compareIndividuals(ind, child) == 0) {
                                    add = false;
                                    break;
                                }
                            }
                        }
                        if (add) {
                            descendantList = getDescendantsRecursive(descendantList, object, child);
                        }
                    }
                }
            }

        }
        return descendantList;
    }
    return descendantList;
}

List getDescendantListN(const GEDCOMobject* familyRecord, const Individual* person, unsigned int maxGen) {
    List gList = initializeList(&printGeneration, &deleteGeneration, &compareGenerations);
    if ((int)maxGen < 0) {
        return gList;
    }

    if ((int)maxGen == 0) {
        maxGen = 100;
    }

    List **tempList = malloc(sizeof(List*) * maxGen);
    int i;
    for (i = 0; i < maxGen; ++i) {
        tempList[i] = malloc(sizeof(List));
        *tempList[i] = initializeList(&printIndividualName, &deleteIndividual, &compareIndividualsName);
    }

    if (person == NULL) {
        return gList;
    }

    if (familyRecord == NULL) {
        return gList;
    }

    int index = 0;
    ListIterator iter = createIterator(person->families);
	Family* family;
    if (person->families.length != 0) {
        while( (family = nextElement(&iter)) != NULL) {

            if ((compareIndividuals(family->husband, person) == 0) || (compareIndividuals(family->wife, person) == 0)) {
                ListIterator iter = createIterator(family->children);
                Individual *child;
                if (family->children.length != 0) {
                    
                    while( (child = nextElement(&iter)) != NULL) {
                        tempList = getDescendantsRecursiveN(tempList, tempList[0], familyRecord, child, maxGen - 1, index + 1);
                    }
                }
            }
        }
        int j;
        for (j = 0; j < maxGen; ++j) {
            if (tempList[j]->length != 0) {
                bool add;
                add = true;
                ListIterator iterator = createIterator(gList);
                List* iList;

                while( (iList = nextElement(&iterator)) != NULL) {
                    if (compareGenerations(iList, tempList[j]) == 0) {
                        add = false;
                        break;
                    }
                }
                if (add) {
                    insertBack(&gList, tempList[j]);
                }
            }
        }
        return gList;
    }
    return gList;
}

List **getDescendantsRecursiveN(List **tempList, List *descendantList, const GEDCOMobject *object, Individual *individual, unsigned int maxGen, int index) {

    Individual *individualCopy = malloc(sizeof(Individual));
    individualCopy->givenName = malloc(sizeof(char) * strlen(individual->givenName) + 2);
    strcpy(individualCopy->givenName, individual->givenName);
    individualCopy->surname = malloc(sizeof(char) * strlen(individual->surname) + 2);
    strcpy(individualCopy->surname, individual->surname);
    individualCopy->families = initializeList(&printFamily, &dummyDelete, &compareFamilies);
    ListIterator familyiterCopy = createIterator(individual->families);
	Family* familyCopy;
    while( (familyCopy = nextElement(&familyiterCopy)) != NULL) {
        insertBack(&individualCopy->families, familyCopy);
    }
    individualCopy->events = initializeList(&printEvent, &dummyDelete, &compareEvents);
    ListIterator eventIterCopy = createIterator(individual->events);
    Event* eventCopy;
    while( (eventCopy = nextElement(&eventIterCopy)) != NULL) {
        insertBack(&individualCopy->events, eventCopy);
    }
    individualCopy->otherFields = initializeList(&printField, &dummyDelete, &compareFields);
    ListIterator fielditerCopy = createIterator(individual->otherFields);
    Field* fieldCopy;
    while( (fieldCopy = nextElement(&fielditerCopy)) != NULL) {
        insertBack(&individualCopy->otherFields, fieldCopy);
    }
    printIndividualName(individualCopy);
    insertSorted(descendantList, individualCopy);

    if (maxGen == 0) {
        return tempList;
    }

    ListIterator iter = createIterator(individual->families);
	Family* family;

    if (individual->families.length != 0) {
        while( (family = nextElement(&iter)) != NULL) {

            if ((compareIndividuals(family->husband, individual) == 0) || (compareIndividuals(family->wife, individual) == 0)) {
                ListIterator iter = createIterator(family->children);
                Individual *child;
                if (family->children.length != 0) {
                    bool add = true;
                    while( (child = nextElement(&iter)) != NULL) {
                        //Check if individual already exists in list
                        ListIterator iterator = createIterator(*tempList[index]);
                        Individual* ind;

                        if (tempList[index]->length != 0) {
                            while( (ind = nextElement(&iterator)) != NULL) {
                                if (compareIndividuals(ind, child) == 0) {
                                    add = false;
                                    break;
                                }
                            }
                        }
                        if (add) {
                            tempList = getDescendantsRecursiveN(tempList, tempList[index], object, child, maxGen - 1, index + 1);
                        }
                    }
                }
            }
        }
        return tempList;
    }
    return tempList;
}

List getAncestorListN(const GEDCOMobject* familyRecord, const Individual* person, int maxGen) {
    List gList = initializeList(&printGeneration, &deleteGeneration, &compareGenerations);
 
    if ((int)maxGen < 0) {
        return gList;
    }

    if ((int)maxGen == 0) {
        maxGen = 100;
    }
    
    List **tempList = malloc(sizeof(List*) * maxGen);
    int i;

    for (i = 0; i < maxGen; ++i) {
        tempList[i] = malloc(sizeof(List));
        *tempList[i] = initializeList(&printIndividualName, &deleteIndividual, &compareIndividualsName);
    }

    if (person == NULL) {
        return gList;
    }

    if (familyRecord == NULL) {
        return gList;
    }

    int index = 0;
    ListIterator iter = createIterator(person->families);
	Family* family;

    if (person->families.length != 0) {
        while( (family = nextElement(&iter)) != NULL) {

            ListIterator iter = createIterator(family->children);
            Individual *child;

            if (family->children.length != 0) {
                bool addHusb = true;
                bool addWife = true;
                while( (child = nextElement(&iter)) != NULL) {
                    if (compareIndividuals(child, person) == 0) {

                        if (tempList[0]->length != 0) {
                            ListIterator iterator = createIterator(*tempList[0]);
                            Individual* ind;
                            while( (ind = nextElement(&iterator)) != NULL) {
                                if (family->husband != NULL) {
                                    if (compareIndividuals(ind, family->husband) == 0) {
                                        addHusb = false;
                                        break;
                                    }
                                }

                                else {
                                    addHusb = false;
                                }
                            }
                        }

                        if (addHusb) {
                            tempList = getAncestorsRecursiveN(tempList, tempList[0], familyRecord, family->husband, maxGen - 1, index + 1);
                        }

                        if (tempList[0]->length != 0) {
                            ListIterator iterator2 = createIterator(*tempList[0]);
                            Individual* indi;
                            while( (indi = nextElement(&iterator2)) != NULL) {
                                if (family->wife != NULL) {
                                    if (compareIndividuals(indi, family->wife) == 0) {
                                        addWife = false;
                                        break;
                                    }
                                }

                                else {
                                    addWife = false;
                                }
                                
                            }
                        }

                        if (addWife) {
                            tempList = getAncestorsRecursiveN(tempList, tempList[0], familyRecord, family->wife, maxGen - 1, index + 1);
                        }
                    }
                }
            }
        }
        int j;
        for (j = 0; j < maxGen; ++j) {
            if (tempList[j]->length != 0) {
                bool add;
                add = true;
                ListIterator iterator = createIterator(gList);
                List* iList;

                while( (iList = nextElement(&iterator)) != NULL) {
                    if (compareGenerations(iList, tempList[j]) == 0) {
                        add = false;
                        break;
                    }
                }
                if (add) {
                    insertBack(&gList, tempList[j]);
                }
            }
        }
        return gList;
    }
    return gList;
}

List **getAncestorsRecursiveN(List **tempList, List *ancestorList, const GEDCOMobject *object, Individual *individual, unsigned int maxGen, int index) {

    Individual *individualCopy = malloc(sizeof(Individual));
    individualCopy->givenName = malloc(sizeof(char) * strlen(individual->givenName) + 2);
    strcpy(individualCopy->givenName, individual->givenName);
    individualCopy->surname = malloc(sizeof(char) * strlen(individual->surname) + 2);
    strcpy(individualCopy->surname, individual->surname);
    individualCopy->families = initializeList(&printFamily, &dummyDelete, &compareFamilies);
    ListIterator familyiterCopy = createIterator(individual->families);
	Family* familyCopy;
    while( (familyCopy = nextElement(&familyiterCopy)) != NULL) {
        insertBack(&individualCopy->families, familyCopy);
    }
    individualCopy->events = initializeList(&printEvent, &dummyDelete, &compareEvents);
    ListIterator eventIterCopy = createIterator(individual->events);
    Event* eventCopy;
    while( (eventCopy = nextElement(&eventIterCopy)) != NULL) {
        insertBack(&individualCopy->events, eventCopy);
    }
    individualCopy->otherFields = initializeList(&printField, &dummyDelete, &compareFields);
    ListIterator fielditerCopy = createIterator(individual->otherFields);
    Field* fieldCopy;
    while( (fieldCopy = nextElement(&fielditerCopy)) != NULL) {
        insertBack(&individualCopy->otherFields, fieldCopy);
    }
    insertSorted(ancestorList, individualCopy);
    ListIterator iter = createIterator(individual->families);
	Family* family;

    if ((int)maxGen == 0) {
        return tempList;
    }

    if (individual->families.length != 0) {
        while( (family = nextElement(&iter)) != NULL) {

            
            if (family->children.length != 0) {
                ListIterator iter = createIterator(family->children);
                Individual *child;
                bool addHusb = true;
                bool addWife = true;
                while( (child = nextElement(&iter)) != NULL) {

                    if (compareIndividuals(child, individual) == 0) {
                    
                        //Check if individual already exists in list
                        int z;
                        for (z = 0; z <= index; z++) {

                            if (tempList[z]->length != 0) {
                                ListIterator iterator = createIterator(*tempList[z]);
                                Individual* ind;
                                while( (ind = nextElement(&iterator)) != NULL) {
                                    if (family->husband != NULL) {
                                        if (compareIndividuals(ind, family->husband) == 0) {
                                            addHusb = false;
                                            break;
                                        }
                                    }

                                    else {
                                        addHusb = false;
                                    }
                                }
                            }
                        }

                        if (addHusb) {
                            tempList = getAncestorsRecursiveN(tempList, tempList[index], object, family->husband, maxGen - 1, index + 1);
                        }

                        for (z = 0; z <= index; ++z) {

                            if (tempList[z]->length != 0) {
                                ListIterator iterator2 = createIterator(*tempList[z]);
                                Individual* indi;
                                while( (indi = nextElement(&iterator2)) != NULL) {
                                    if (family->wife != NULL) {
                                        if (compareIndividuals(indi, family->wife) == 0) {
                                            addWife = false;
                                            break;
                                        }
                                    }

                                    else {
                                        addWife = false;
                                    }
                                }
                            }
                        }

                        if (addWife) {
                            tempList = getAncestorsRecursiveN(tempList, tempList[index], object, family->wife, maxGen - 1, index + 1);
                        }
                    }
                }
            }
        }
        return tempList;
    }
    return tempList;
}

GEDCOMerror writeGEDCOM(char* fileName, const GEDCOMobject* obj) {

    ErrorCode code = validateGEDCOM(obj);

    if (code != OK) {
        GEDCOMerror error;
        error.type = WRITE_ERROR;
        error.line = -1;
        return error;
    }

    FILE* outFile = NULL;
    outFile = fopen(fileName, "w+");
    if (outFile == NULL) {
        GEDCOMerror error;
        error.type = WRITE_ERROR;
        error.line = -1;
        return error;
    }


    GEDCOMerror error;
    error.type = OK;
    error.line = -1;

    List xREFIndividuals = initializeList(&printIndividual, &dummyDelete, &compareIndividuals);
    List xREFFamilies = initializeList(&printFamily, &dummyDelete, &compareIndividuals);

    char xrefID[10];
    int i = 0;
    ListIterator iterator = createIterator(obj->individuals);
    Individual* individual;
    for (i = 1; i <= obj->individuals.length; ++i) {
        individual = nextElement(&iterator);
        if (individual == NULL) {
            GEDCOMerror error;
            error.type = WRITE_ERROR;
            error.line = -1;
            return error;
        }
        strcpy(xrefID, "I");
        char id[10];
        sprintf(id, "%d", i);
        strcat(xrefID, id);
        Xref *xref = malloc(sizeof(Xref));
        strcpy(xref->id, xrefID);
        xref->data = individual;
        insertBack(&xREFIndividuals, xref);
    }

    ListIterator iterator2 = createIterator(obj->families);
    Family* family;
    for (i = 1; i <= obj->families.length; ++i) {
        family = nextElement(&iterator2);
        if (family == NULL) {
            GEDCOMerror error;
            error.type = WRITE_ERROR;
            error.line = -1;
            return error;
        }
        strcpy(xrefID, "F");
        char id[10];
        sprintf(id, "%d", i);
        strcat(xrefID, id);
        Xref *xref = malloc(sizeof(Xref));
        strcpy(xref->id, xrefID);
        xref->data = family;
        insertBack(&xREFFamilies, xref);
    }

    fprintf(outFile, "0 HEAD\n");
    fprintf(outFile, "1 SOUR %s\n", obj->header->source);
    fprintf(outFile, "1 GEDC\n");
    fprintf(outFile, "2 VERS %.1lf\n", obj->header->gedcVersion);
    fprintf(outFile, "2 FORM LINAGE-LINKED\n");
    fprintf(outFile, "1 CHAR ");

    switch(obj->header->encoding) {
        case ANSEL:
            fprintf(outFile, "ANSEL\n");
            break;

        case UTF8:
            fprintf(outFile, "UTF-8\n");
            break;

        case UNICODE:
            fprintf(outFile, "UNICODE\n");
            break;

        case ASCII:
            fprintf(outFile, "ASCII\n");

        default:
            break;
    }

    fprintf(outFile, "1 SUBM @U1@\n");

    if (xREFIndividuals.length != 0) {
        toFile(xREFIndividuals, xREFFamilies, outFile, &writeIndividual);
    }

    if (xREFFamilies.length != 0) {
        toFile(xREFFamilies, xREFIndividuals, outFile, &writeFamily);
    }
    fprintf(outFile, "0 @U1@ SUBM\n");
    fprintf(outFile, "1 NAME %s\n", obj->submitter->submitterName);
    if (strcmp(obj->submitter->address, "") != 0) {
        fprintf(outFile, "1 ADDR %s\n", obj->submitter->address);
    }

    fprintf(outFile, "0 TRLR\n");

    fclose(outFile);

    return error;
}


ErrorCode validateGEDCOM(const GEDCOMobject* obj) {

    if (obj == NULL) {
        return INV_GEDCOM;
    }

    if ((obj->header == NULL) || (obj->submitter == NULL)) {
        return INV_GEDCOM;
    }

    else if ((obj->header->encoding == -1) || (obj->header->submitter == NULL) || (strcmp(obj->header->source, "") == 0) || (obj->header->gedcVersion == -1.0)) {
        return INV_HEADER;
    }

    else if ((strcmp(obj->submitter->submitterName, "") == 0)) {
        return INV_RECORD;
    }

    else if (!validateRecord(obj)) {
        return INV_RECORD;
    }

    else {
        return OK;
    }

}

bool validateRecord(const GEDCOMobject *object) {

    ListIterator indIterator = createIterator(object->individuals);
    Individual* individual;
    int i;
    for (i = 0; i < object->individuals.length; ++i) {
        individual = nextElement(&indIterator);
        if (individual == NULL) {
            return false;
        }
        if ((strlen(individual->givenName) > 255) || (strlen(individual->surname) > 255)) {
            return false;
        }
        ListIterator eventIterator = createIterator(individual->events);
        Event* event;
        int j;
        for (j = 0; j < individual->events.length; ++j) {
            event = nextElement(&eventIterator);
            if (event == NULL) {
                return false;
            }
            if ((strlen(event->place) > 255) || (strlen(event->date) > 255)) {
                return false;
            }
        }
        
        ListIterator fieldIterator = createIterator(individual->otherFields);
        Field* field;
        int x;
        for (x = 0; x < individual->otherFields.length; ++x) {
            field = nextElement(&fieldIterator);
            if (field == NULL) {
                return false;
            }
            if ((strlen(field->tag) > 255) || (strlen(field->tag) > 255)) {
                return false;
            }
        }
    }
    ListIterator familyIterator = createIterator(object->families);
    Family* family;
    for (i = 0; i < object->families.length; ++i) {
        family = nextElement(&familyIterator);
        if (family == NULL) {
            return false;
        }
        ListIterator eventIterator = createIterator(family->events);
        Event* event;
        int j;
        for (j = 0; j < family->events.length; ++j) {
            event = nextElement(&eventIterator);
            if ((strlen(event->place) > 255) || (strlen(event->date) > 255)) {
                return false;
            }
        }
    }

    if (strlen(object->submitter->address) > 255) {
        return false;
    }

    return true;
}

char* indToJSON(const Individual* ind) {

    char *jsonString = malloc(sizeof(char) + 2);
    strcpy(jsonString, "");
    int length = 0;

    if (ind == NULL) {
        return jsonString;
    }

    length += strlen(ind->givenName);
    length += 15;
    jsonString = realloc(jsonString, sizeof(char) * length + 3);
    strcat(jsonString, "{\"givenName\":\"");
    strcat(jsonString, ind->givenName);
    length += 20;
    jsonString = realloc(jsonString, sizeof(char) * length + 3);
    strcat(jsonString, "\",\"surname\":\"");
    length += strlen(ind->surname);
    jsonString = realloc(jsonString, sizeof(char) * length + 3);
    strcat(jsonString, ind->surname);

    Field* tempField = malloc(sizeof(Field));
    tempField->tag = malloc(sizeof(char) * 5);
    tempField->value = malloc(sizeof(char) * 5);
    length += 100;
    bool found = false;
    jsonString = realloc(jsonString, sizeof(char) * length + 3);
    strcat(jsonString, "\",\"sex\":\"");
    strcpy(tempField->tag, "SEX");
    strcpy(tempField->value, "M");
    if (findElement(ind->otherFields, &compareFieldsBool, tempField) != NULL) {
        strcat(jsonString, "M");
        found = true;
    }

    strcpy(tempField->value, "F");
    if ((findElement(ind->otherFields, &compareFieldsBool, tempField) != NULL) && (!found)) {
        strcat(jsonString, "F");
    }

    else {
        if (!found) {
            strcat(jsonString, "Unknown");
        }
    }

    int numFamilyMembers = 1;
    ListIterator iter = createIterator(ind->families);
    Family* family;
    while( (family = nextElement(&iter)) != NULL){
        if (family->wife != NULL) {
            numFamilyMembers += 1;
        }

        if (family->husband != NULL) {
            numFamilyMembers += 1;
        }

        numFamilyMembers += family->children.length;
        numFamilyMembers -= 1;
    }

    char familyMembers[10];
    sprintf(familyMembers, "%d", numFamilyMembers);
    length += 50;
    jsonString = realloc(jsonString, sizeof(char) * length + 3);
    strcat(jsonString, "\",\"familyMembers\":\"");
    strcat(jsonString, familyMembers);
    strcat(jsonString, "\"}");
    return jsonString;
}

char *gedcomToJSON(char *filename) {
    GEDCOMobject **obj = malloc(sizeof(GEDCOMobject*));
    GEDCOMerror error = createGEDCOM(filename, obj);
    GEDCOMobject *object = *obj;
    char *jsonString = malloc(sizeof(char) + 2);
    strcpy(jsonString, "");

    if (error.type != OK) {
        jsonString = realloc(jsonString, sizeof(char) * 15);
        return strcpy(jsonString, "failed to parse");
    }

    int length = 0;

    length += strlen(object->header->source);
    length += 12;
    jsonString = realloc(jsonString, sizeof(char) * length + 50);
    strcat(jsonString, "{\"source\":\"");
    strcat(jsonString, object->header->source);
    length += 30;
    jsonString = realloc(jsonString, sizeof(char) * length + 50);
    strcat(jsonString, "\",\"gedcVersion\":\"");
    length += 5;
    char version[10];
    sprintf(version, "%.1f", object->header->gedcVersion);
    jsonString = realloc(jsonString, sizeof(char) * length + 50);
    strcat(jsonString, version);
    length += 20;
    strcat(jsonString, "\",\"encoding\":\"");
    switch(object->header->encoding) {
        case ANSEL:
            length += 6;
            jsonString = realloc(jsonString, sizeof(char) * length + 50);
            strcat(jsonString, "ANSEL");
            break;

        case UTF8:
            length += 6;
            jsonString = realloc(jsonString, sizeof(char) * length + 50);
            strcat(jsonString, "UTF-8");
            break;

        case UNICODE:
            length += 8;
            jsonString = realloc(jsonString, sizeof(char) * length + 50);
            strcat(jsonString, "UNICODE");
            break;

        case ASCII:
            length += 6;
            jsonString = realloc(jsonString, sizeof(char) * length + 50);
            strcat(jsonString, "ASCII");
            break;

        default:
            break;
    }

    length += 15;
    strcat(jsonString, "\",\"subName\":\"");
    length += strlen(object->submitter->submitterName);
    jsonString = realloc(jsonString, sizeof(char) * length + 50);
    strcat(jsonString, object->submitter->submitterName);
    strcat(jsonString, "\",\"subAddress\":\"");
    if (strcmp(object->submitter->address, "") != 0) {
        strcat(jsonString, object->submitter->address);
    }

    else {
        strcat(jsonString, "");
    }
    jsonString = realloc(jsonString, sizeof(char) * length + 50);
    length += 50;
    jsonString = realloc(jsonString, sizeof(char) * length + 50);
    strcat(jsonString, "\",\"numIndividuals\":\"");
    int numIndividuals = object->individuals.length;
    int numFamilies = object->families.length;
    char indString[10];
    char famString[10];
    sprintf(indString, "%d", numIndividuals);
    sprintf(famString, "%d", numFamilies);
    strcat(jsonString, indString);
    strcat(jsonString, "\",\"numFamilies\":\"");
    strcat(jsonString, famString);
    strcat(jsonString, "\",\"filename\":\"");
    char *token;
    token = strtok(filename, "/");
    char *file = malloc(sizeof(char) * 50);
    while (token != NULL) {
        strcpy(file, token);
        token = strtok(NULL, "/");
    }
    jsonString = realloc(jsonString, sizeof(char) * length + 50);
    strcat(jsonString, file);
    strcat(jsonString, "\"}");
    deleteGEDCOM(object);
    return jsonString;
}

Individual* JSONtoInd(const char* str) {
    if (str == NULL) {
        return NULL;
    }

    Individual *individual = malloc(sizeof(Individual));
    char *tempString = malloc(sizeof(char) * strlen(str) + 1);
    strcpy(tempString, str);

    char **parsedData = malloc(sizeof(char*) * 5);

    int x = 0;
    char *token;
    token = strtok(tempString, "{}:,");
    while (token != NULL) {
        if (x > 4) {
            free(individual);
            free(tempString);
            return NULL;
        }
        parsedData[x] = malloc(sizeof(char) * 50);
        strcpy(parsedData[x], token);
        token = strtok(NULL, "{}:,");
        ++x;
    }

    if (x != 4) {
        free(individual);
        free(tempString);
        return NULL;
    }

    if (strcmp(parsedData[0], "\"givenName\"") == 0) {
        individual->givenName = malloc(sizeof(char) * strlen(parsedData[1]) + 1);
        int i = 0;
        int c = 0;
        char tempString[255];
        for (i = 1; i < strlen(parsedData[1]) - 1; ++i) {
            tempString[c] = parsedData[1][i];
            ++c;
        }
        strcpy(individual->givenName, tempString);

        if (c != 0) {
            tempString[c] = '\0';
            strcpy(individual->givenName, tempString);
        }

        else {
            
            strcpy(individual->givenName, "");
        }
    }


    else {
        free(individual);
        free(tempString);
        return NULL;
    }

    if (strcmp(parsedData[2], "\"surname\"") == 0) {
        individual->surname = malloc(sizeof(char) * strlen(parsedData[3]) + 1);
        int i;
        int c = 0;
        char tempString[255];
        for (i = 1; i < strlen(parsedData[3]) - 1; ++i) {
            tempString[c] = parsedData[3][i];
            ++c;
        }
        if (c != 0) {
            tempString[c] = '\0';
            strcpy(individual->surname, tempString);
        }

        else {
            strcpy(individual->surname, "");
        }
        
    }

    else {
        free(individual);
        free(tempString);
        return NULL;
    }

    individual->families = initializeList(&printFamily, &dummyDelete, &compareFamilies);
    individual->events = initializeList(&printEvent, &deleteEvent, &compareEvents);
    individual->otherFields = initializeList(&printField, &deleteField, &compareFields);

    int i = 0;
    for (i = 0; i < x; i++) {
        free(parsedData[i]);
    }
    free(parsedData);
    free(tempString);
    return individual;
}

GEDCOMobject* JSONtoGEDCOM(const char* str) {
    if (str == NULL) {
        return NULL;
    }

    GEDCOMobject *object = malloc(sizeof(GEDCOMobject));
    Header *header = malloc(sizeof(Header));
    header->otherFields = initializeList(&printField, &deleteField, &compareFields);
    Submitter *submitter = malloc(sizeof(Submitter));
    submitter->otherFields = initializeList(&printField, &deleteField, &compareFields);
    object->header = header;
    object->submitter = submitter;
    object->header->submitter = submitter;
    char *tempString = malloc(sizeof(char) * strlen(str) + 1);
    strcpy(tempString, str);

    char **parsedData = malloc(sizeof(char*) * 11);

    int x = 0;
    char *token;
    token = strtok(tempString, "{}:,");
    while (token != NULL) {
        if (x > 10) {
            free(tempString);
            free(header);
            free(object);
            free(submitter);
            return NULL;
        }
        parsedData[x] = malloc(sizeof(char) * 50);
        strcpy(parsedData[x], token);
        token = strtok(NULL, "{}:,");
        ++x;
    }

    if (x != 10) {
        free(tempString);
        free(header);
        free(object);
        free(submitter);
        return NULL;
    }

    if (strcmp(parsedData[0], "\"source\"") == 0) {
        int i = 0;
        int c = 0;
        char tempString[255];
        for (i = 1; i < strlen(parsedData[1]) - 1; ++i) {
            tempString[c] = parsedData[1][i];
            ++c;
        }

        if (c != 0) {
            tempString[c] = '\0';
            strcpy(object->header->source, tempString);
        }

        else {
            strcpy(object->header->source, "");
        }
    }

    else {
        free(tempString);
        free(header);
        free(submitter);
        free(object);
        return NULL;
    }

    if (strcmp(parsedData[2], "\"gedcVersion\"") == 0) {
        object->header->gedcVersion = atof(parsedData[3]);
        int i = 0;
        int c = 0;
        char tempString[255];
        for (i = 1; i < strlen(parsedData[3]) - 1; ++i) {
            tempString[c] = parsedData[3][i];
            ++c;
        }

        if (c != 0) {
            tempString[c] = '\0';
            object->header->gedcVersion = atof(tempString);
        }
    }

    else {
        free(tempString);
        free(header);
        free(submitter);
        free(object);
        printf("here\n");
        return NULL;
    }

    if (strcmp(parsedData[4], "\"encoding\"") == 0) {
        if (strcmp(parsedData[5], "\"UTF-8\"") == 0) {
            object->header->encoding = UTF8;
        }

        else if (strcmp(parsedData[5], "\"ANSEL\"") == 0) {
            object->header->encoding = ANSEL;
        }

        else if (strcmp(parsedData[5], "\"ASCII\"") == 0) {
            object->header->encoding = ASCII;
        }

        else if (strcmp(parsedData[5], "\"UNICODE\"") == 0) {
            object->header->encoding = UNICODE;
        }

        else {
            free(tempString);
            free(header);
            free(submitter);
            free(object);
            return NULL;
        }
    }

    else {
        free(tempString);
        free(header);
        free(submitter);
        free(object);
        return NULL;
    }

    if (strcmp(parsedData[6], "\"subName\"") == 0) {
        object->submitter = realloc(submitter, sizeof(Submitter) + sizeof(char) * (strlen(parsedData[9] + 1)));
        int i = 0;
        int c = 0;
        char tempString[255];
        for (i = 1; i < strlen(parsedData[7]) - 1; ++i) {
            tempString[c] = parsedData[7][i];
            ++c;
        }

        if (c != 0) {
            tempString[c] = '\0';
            strcpy(object->submitter->submitterName, tempString);
        }
        
    }

    else {
        free(tempString);
        free(header);
        free(submitter);
        free(object);
        return NULL;
    }

    if (strcmp(parsedData[8], "\"subAddress\"") == 0) {
        int i = 0;
        int c = 0;
        char tempString[255];
        for (i = 1; i < strlen(parsedData[9]) - 1; ++i) {
            tempString[c] = parsedData[9][i];
            ++c;
        }

        

        if (c != 0) {
            tempString[c] = '\0';
            strcpy(object->submitter->address, tempString);
        }

        else {
            strcpy(object->submitter->address, "");
        }
        
    }

    else {
        free(tempString);
        free(header);
        free(submitter);
        free(object);
        return NULL;
    }
    object->families = initializeList(&printFamily, &deleteFamily, &compareFamilies);
    object->individuals = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);
    return object;
}

void jsonToNewFile(const char *json, char *filename) {
    GEDCOMobject *object = JSONtoGEDCOM(json);
    writeGEDCOM(filename, object);
    return;
}

void addIndividual(GEDCOMobject* obj, const Individual* toBeAdded) {

    if ((obj == NULL) || (toBeAdded == NULL)) {
        return;
    }

    insertBack(&obj->individuals, (void *)toBeAdded);
    return;
}

void addIndJSON(char *filename, const char *json) {
    Individual *individual = JSONtoInd(json);
    GEDCOMobject **object = malloc(sizeof(GEDCOMobject*));
    GEDCOMerror error = createGEDCOM(filename, object);
    if (error.type != OK) {
        return;
    }
    GEDCOMobject *obj = *object;
    addIndividual(obj, individual);
    writeGEDCOM(filename, obj);
    return;
}

char *parseIndividualsToJSON(char *filename) {
    GEDCOMobject **object = malloc(sizeof(GEDCOMobject*));
    GEDCOMerror error = createGEDCOM(filename, object);
    char *jsonString = malloc(sizeof(char) * 100000);
    if (error.type != OK) {
        strcpy(jsonString, "failure to parse");
    }
    GEDCOMobject *obj = *object;
    strcpy(jsonString, iListToJSON(obj->individuals));
    return jsonString;
}

char* iListToJSON(List iList) {
    char *jsonString = malloc(sizeof(char) + 2);
    strcpy(jsonString, "[");
    int length = 1;

    if ((iList.length != 0) && (iList.head != NULL)) {
        ListIterator iter = createIterator(iList);
        Individual* individual;
        int i = 0;
        while( (individual = nextElement(&iter)) != NULL){
            if (i != 0) {
                strcat(jsonString, ",");
            }
            ++i;
            char* currDescr = indToJSON(individual);
            length = strlen(jsonString)+50+strlen(currDescr);
            jsonString = (char*)realloc(jsonString, length);
            strcat(jsonString, currDescr);
            free(currDescr);
        }
    }

    jsonString = realloc(jsonString, sizeof(char) * strlen(jsonString) + 10);
    strcat(jsonString, "]");
    return jsonString;
}

char* gListToJSON(List gList) {
    char *jsonString = malloc(sizeof(char) + 2);
    strcpy(jsonString, "[");
    int length = 1;

    if ((gList.length != 0) && (gList.head != NULL)) {
        ListIterator iter = createIterator(gList);
        List *iList;
        int i = 0;
        while( (iList = nextElement(&iter)) != NULL){
            ++i;
            char* currDescr = iListToJSON(*iList);
            length = strlen(jsonString)+50+strlen(currDescr);
            jsonString = (char*)realloc(jsonString, length);
            strcat(jsonString, currDescr);
            if (i < gList.length) {
                strcat(jsonString, ",");
            }
            free(currDescr);
        }
    }

    jsonString = realloc(jsonString, sizeof(char) * strlen(jsonString) + 10);
    strcat(jsonString, "]");
    return jsonString;
}

char *getDescendantsJSON(char *filename, const char* json, int num) {
    unsigned int maxGen = num;
    GEDCOMobject **object = malloc(sizeof(GEDCOMobject*));
    GEDCOMerror error = createGEDCOM(filename, object);
    char *jsonString = malloc(sizeof(char) * 100000);
    if (error.type != OK) {
        strcpy(jsonString, "failure to parse");
    }
    GEDCOMobject *obj = *object;
    Individual *individual = JSONtoInd(json);
    Individual *ind = findPerson(obj, &compareIndividualsBool, individual);
    List gList = getDescendantListN(obj, ind, maxGen);
    strcpy(jsonString, gListToJSON(gList));
    return jsonString;
}

char *getAncestorsJSON(char *filename, const char* json, int num) {
    unsigned int maxGen = num;
    GEDCOMobject **object = malloc(sizeof(GEDCOMobject*));
    GEDCOMerror error = createGEDCOM(filename, object);
    char *jsonString = malloc(sizeof(char) * 100000);
    if (error.type != OK) {
        strcpy(jsonString, "failure to parse");
    }
    GEDCOMobject *obj = *object;
    Individual *individual = JSONtoInd(json);
    Individual *ind = findPerson(obj, &compareIndividualsBool, individual);
    List gList = getAncestorListN(obj, ind, maxGen);
    strcpy(jsonString, gListToJSON(gList));
    return jsonString;
}


//Helper functions
void deleteEvent(void* toBeDeleted) {

    Event *delete = (Event*)toBeDeleted;
    free(delete->date);
    free(delete->place);
    clearList(&delete->otherFields);
    free(delete);
    return;
}

int compareEvents(const void* first,const void* second) {
    Event *a = (Event*)first;
    Event *b = (Event*)second;

    return strcmp(a->type, b->type);
}

char* printEvent(void* toBePrinted) {

    Event *print = (Event*)toBePrinted;
    char *printString = malloc(sizeof(char) + 2);
    strcpy(printString, "");
    int length = 0;

    length += strlen(print->type);
    length += 7;
    printString = realloc(printString, sizeof(char) * length + 3);
    strcat(printString, "Type: ");
    strcat(printString, print->type);
    strcat(printString, "\n");

    if (strcmp(print->date, "") != 0) {
        length += strlen(print->date);
        length += 7;
        printString = realloc(printString, sizeof(char) * length + 3);
        strcat(printString, "Date: ");
        strcat(printString, print->date);
        strcat(printString, "\n");
    }

    if (strcmp(print->place, "") != 0) {
        length += strlen(print->place);
        length += 8;
        printString = realloc(printString, sizeof(char) * length + 3);
        strcat(printString, "Place: ");
        strcat(printString, print->place);
        strcat(printString, "\n");
    }

    if (print->otherFields.length != 0) {
        length += strlen(toString(print->otherFields));
        length += 14;
        printString = realloc(printString, sizeof(char) * length + 3);
        strcat(printString, "Other Fields:\n");
        strcat(printString, toString(print->otherFields));
        strcat(printString, "\n");
    }

    return printString;
}

void deleteIndividual(void* toBeDeleted) {

    Individual *delete = (Individual*)toBeDeleted;

    if (delete->surname != NULL) {
        free(delete->surname);
    }

    if (delete->givenName != NULL) {
        free(delete->givenName);
    }

    if (delete->events.length != 0) {
        clearList(&delete->events);
    }

    if (delete->families.length != 0) {
        clearList(&delete->families);
    }

    if (delete->otherFields.length != 0) {
        clearList(&delete->otherFields);
    }
    free(delete);
    return;
}

int compareIndividuals(const void* first,const void* second) {
    
    Individual *a = (Individual*)first;
    Individual *b = (Individual*)second;
    char *compareFirst;
    char *compareSecond;

    if ((a == NULL) || (b == NULL)) {
        return -1;
    }

    if (strcmp(a->surname, "") != 0) {
        compareFirst = malloc(sizeof(char) * (strlen(a->surname)) + 2);
        strcpy(compareFirst, a->surname);

        if (strcmp(a->givenName, "") != 0) {
            compareFirst = realloc(compareFirst, sizeof(char) * (strlen(a->surname) + strlen(a->givenName) + 2));
            strcat(compareFirst, ",");
            strcat(compareFirst, a->givenName);
        }
    }

    else if (strcmp(a->givenName, "") != 0) {
        compareFirst = malloc(sizeof(char) * strlen(a->givenName) + 2);
        strcpy(compareFirst, a->givenName);
    }
    
    else {
        compareFirst = malloc(sizeof(char) + 2);
        strcpy(compareFirst, "");
    }

    if (strcmp(b->surname, "") != 0) {
        compareSecond = malloc(sizeof(char) * (strlen(b->surname)) + 2);
        strcpy(compareSecond, b->surname);

        if (strcmp(b->givenName, "") != 0) {
            compareSecond = realloc(compareSecond, sizeof(char) * (strlen(b->surname) + strlen(b->givenName) + 2));
            strcat(compareSecond, ",");
            strcat(compareSecond, b->givenName);
        }
    }

    else if (strcmp(b->givenName, "") != 0) {
        compareSecond = malloc(sizeof(char) * strlen(b->givenName) + 2);
        strcpy(compareSecond, b->givenName);
    }
    
    else {
        compareSecond = malloc(sizeof(char) + 2);
        strcpy(compareSecond, "");
    }

    if (strcmp(compareFirst, compareSecond) == 0) {
        if (a->families.length == b->families.length) {
            if (a->events.length == b->events.length) {

                if (a->otherFields.length == b->otherFields.length) {
                    return 0;
                }

                else {
                    return 1;
                }
            }
            else {
                return 1;
            }
        }

        else {
            return 1;
        }
    }

    return strcmp(compareFirst, compareSecond);
}

char* printIndividual(void* toBePrinted) {
    Individual *print = (Individual*)toBePrinted;
    char *printString = malloc(sizeof(char) + 2);
    strcpy(printString, "");
    int length = 0;

    if (strcmp(print->givenName, "") != 0) {
        length += strlen(print->givenName);
        length += 12;
        printString = realloc(printString, sizeof(char) * length + 3);
        strcat(printString, "Given Name: ");
        strcat(printString, print->givenName);
        strcat(printString, "\n");
    }

    if (strcmp(print->surname, "") != 0) {
        length += 10;
        printString = realloc(printString, sizeof(char) * length + 3);
        strcat(printString, "Surname: ");
        length += strlen(print->surname);
        printString = realloc(printString, sizeof(char) * length + 3);
        strcat(printString, print->surname);
        printString = realloc(printString, sizeof(char) * length + 3);
        strcat(printString, "\n");
    }

    if (print->events.length != 0) {
        length += strlen(toString(print->events));
        length += 10;
        printString = realloc(printString, sizeof(char) * length + 3);
        strcat(printString, "Events: ");
        strcat(printString, toString(print->events));
        strcat(printString, "\n");
    }
    
    if (print->families.length != 0) {
        length += strlen(toString(print->families));
        length += 11;
        printString = realloc(printString, sizeof(char) * length + 3);
        strcat(printString, "Families:\n");
        strcat(printString, toString(print->families));
        strcat(printString, "\n");
    }

    if (print->otherFields.length != 0) {
        length += strlen(toString(print->otherFields));
        length += 14;
        printString = realloc(printString, sizeof(char) * length + 3);
        strcat(printString, "Other Fields:\n");
        strcat(printString, toString(print->otherFields));
        strcat(printString, "\n");
    }

    return printString;
}

void deleteFamily(void* toBeDeleted) {

    Family* delete = (Family*)toBeDeleted;
    if (delete->children.length != 0) {
        clearList(&delete->children);
    }
    if (delete->otherFields.length != 0) {
        clearList(&delete->otherFields);
    }
    free(delete);
    return;
}

int compareFamilies(const void* first,const void* second) {
    int membersFirst = 0;
    int membersSecond = 0;

    Family *a = (Family*)first;
    Family *b = (Family*)second;

    if ((a == NULL) || (b == NULL)) {
        return -1;
    }

    if (a == b) {
        return 0;
    }

    else {
        return 1;
    }

    if (a->wife != NULL) {
        membersFirst += 1;
    }

    if (a->husband != NULL) {
        membersFirst += 1;
    }

    if (a->children.length != 0) {
        membersFirst += getLength(a->children);
    } 

    if (b->wife != NULL) {
        membersSecond += 1;
    }

    if (b->husband != NULL) {
        membersSecond += 1;
    }

    if (b->children.length != 0) {
        membersSecond += getLength(b->children);
    }

    else {
        return 0;
    }

    if (membersFirst > membersSecond) {
        return -1;
    }

    else if (membersFirst < membersSecond){
        return 1;
    }

    else {
        return 0;
    }
}

char* printFamily(void* toBePrinted) {
    Family *print = (Family*)toBePrinted;
    char *printString = malloc(sizeof(char) + 2);
    strcpy(printString, "");
    int length = 0;

    if (print->husband != NULL) {
        length += strlen(printIndividualName(print->husband));
        length += 10;
        printString = realloc(printString, sizeof(char) * length + 3);
        strcat(printString, "Husband: ");
        strcat(printString, printIndividualName(print->husband));
        strcat(printString, "\n");
    }


    if (print->wife != NULL) {
        length += strlen(printIndividualName(print->wife));
        length += 7;
        printString = realloc(printString, sizeof(char) * length + 3);
        strcat(printString, "Wife: ");
        strcat(printString, printIndividualName(print->wife));
        strcat(printString, "\n");
    }

    if (print->children.length != 0) {
        length += strlen(toString(print->children));
        length += 11;
        printString = realloc(printString, sizeof(char) * length + 3);
        strcat(printString, "Children:\n");
        strcat(printString, toString(print->children));
        strcat(printString, "\n");
    }

    if (print->events.length != 0) {
        length += strlen(toString(print->events));
        length += 10;
        printString = realloc(printString, sizeof(char) * length + 3);
        strcat(printString, "Events: ");
        strcat(printString, toString(print->events));
        strcat(printString, "\n");
    }

    if (print->otherFields.length != 0) {
        length += strlen(toString(print->otherFields));
        length += 14;
        printString = realloc(printString, sizeof(char) * length + 3);
        strcat(printString, "Other Fields:\n");
        strcat(printString, toString(print->otherFields));
        strcat(printString, "\n");
    }

    return printString;
}

void deleteField(void* toBeDeleted) {
    Field *delete = (Field*)toBeDeleted;

    if (delete != NULL) {
        free(delete->tag);
        free(delete->value);
        free(delete);
    }
    return;
}

int compareFields(const void* first,const void* second) {

    Field *a = (Field*)first;
    Field *b = (Field*)second;

    char *compareFirst = malloc(sizeof(char) * (strlen(a->tag) + strlen(a->value) + 2));
    char *compareSecond = malloc(sizeof(char) * (strlen(b->tag) + strlen(b->value) + 2));

    strcpy(compareFirst, a->tag);
    strcat(compareFirst, " ");
    strcat(compareFirst, a->value);

    strcpy(compareSecond, b->tag);
    strcat(compareSecond, " ");
    strcat(compareSecond, b->value);

    return strcmp(compareFirst, compareSecond);
}

char* printField(void* toBePrinted) {
    Field *print = (Field*)toBePrinted;
    char *printString = malloc(sizeof(char) + 2);
    strcpy(printString, "");
    int length = 0;

    if (strcmp(print->tag, "") != 0) {
        length += strlen(print->tag);
        length += 6;
        printString = realloc(printString, sizeof(char) * length + 3);
        strcat(printString, "Tag: ");
        strcat(printString, print->tag);
        strcat(printString, "\n");
    }

    if (strcmp(print->value, "") != 0) {
        length += strlen(print->value);
        length += 8;
        printString = realloc(printString, sizeof(char) * length + 3);
        strcat(printString, "Value: ");
        strcat(printString, print->value);
        strcat(printString, "\n");
    }

    return printString;
}

HTable* createTable(size_t size, int (*hashFunction)(size_t tableSize, char* key),void (*destroyData)(void *data),void (*printNode)(void *toBePrinted)) {
    HTable* hashTable = calloc(1, sizeof(HTable));
    if (hashTable != NULL) {
        hashTable->size = size;
        hashTable->table = calloc(hashTable->size, sizeof(HashNode*) * hashTable->size);
        hashTable->hashFunction = hashFunction;
        hashTable->destroyData = destroyData;
        hashTable->printNode = printNode;
    }

    return hashTable;
}

HashNode *createNode(char* key, void *data) {
    HashNode* node = calloc(1, sizeof(HashNode));
    node->key = calloc(strlen(key), sizeof(char) * strlen(key));
    strcpy(node->key, key);
    node->data = data;
    node->next = NULL;
    return node;
}

void insertData(HTable *hashTable, char* key, void *data) {
    int index = hashTable->hashFunction(hashTable->size, key);
    HashNode* node = createNode(key, data);
    if (hashTable->table[index] == NULL) {
        hashTable->table[index] = node;
        return;
    }

    else if (hashTable->table[index]->key[0] != '@') {
        hashTable->table[index] = node;
        return;
    }

    else {
        HashNode* tempNode = hashTable->table[index];
        while (tempNode->next != NULL) {
            tempNode = tempNode->next;
        }
        tempNode->next = node;
        return;
    }
}

void *lookupData(HTable *hashTable, char* key) {
    int index = hashTable->hashFunction(hashTable->size, key);
    HashNode* tempNode = hashTable->table[index];

    while (tempNode != NULL) {
        if (strncmp(tempNode->key, key, strlen(tempNode->key)) == 0) {
            return tempNode->data;
        }
        
        tempNode = tempNode->next;
    }
    return NULL;
}

void removeData(HTable *hashTable, char* key) {
    int index = hashTable->hashFunction(hashTable->size, key);
    HashNode* tempNode = hashTable->table[index];

    while (tempNode->next != NULL) {

        if (strcmp(tempNode->next->key, key) == 0) {
            hashTable->destroyData(tempNode->next->data);

            if (tempNode->next->next != NULL) {
                tempNode->next = tempNode->next->next;
                return;
            }

            else {
                free(tempNode->next);
                tempNode->next = NULL;
                return;
            }
        }
        else if (strcmp(tempNode->key, key) == 0) {
            hashTable->destroyData(tempNode->data);
            hashTable->table[index] = tempNode->next;
            return;
        }
        
        tempNode = tempNode->next;
    }
    
    if (strcmp(tempNode->key, key) == 0) {
        hashTable->destroyData(tempNode->data);
        free(hashTable->table[index]);
        hashTable->table[index] = NULL;
    }
    return;
}

void destroyTable(HTable *hashTable) {
    int i;
    for (i = 0; i < hashTable->size; i++) {
        HashNode* node = hashTable->table[i];
        while (node != NULL) {
            node->data = NULL;
            free(node->key);
            node = node->next;
        }
    }
    free(hashTable->table);
    hashTable->table = NULL;
    free(hashTable);
    hashTable = NULL;
    return;
}

FILE *openFile(char *filename) {
    FILE* infile = NULL;
    infile = fopen(filename, "r");
    
    if (infile == NULL) {
        return NULL;
    }

    if ((filename[strlen(filename) - 1] != 'd') || (filename[strlen(filename) - 2] != 'e') || (filename[strlen(filename) - 3] != 'g') || (filename[strlen(filename) - 4] != '.')) {
        return NULL;
    }

    return infile; 
}

void *getData(HashNode * node) {
    return node->data;
}

char *getKey(HashNode * node) {
    return node->key;
}

int hashNode(size_t tableSize, char* key) {
    char ch = key[strlen(key) - 2]; 
    if (ch == '@') {
        ch = key[strlen(key) - 3]; 
    }
    return (ch - '0')%tableSize;
}

void dummyDelete(void *data) {
    return;
}

void printNodeData(void *toBePrinted) {
    printf("%s\n", getKey(toBePrinted));
    return;
}

char *printIndividualName(void *toBePrinted) {

    Individual *print = (Individual*)toBePrinted;
    char *printString = malloc(sizeof(char) + 2);
    strcpy(printString, "");
    int length = 0;

    if (strcmp(print->givenName, "") != 0) {
        length += strlen(print->givenName);
        printString = realloc(printString, sizeof(char) * length + 3);
        strcat(printString, print->givenName);
        strcat(printString, " ");
    }

    if ((strcmp(print->surname, "") != 0)) {
        length += strlen(print->surname);
        printString = realloc(printString, sizeof(char) * length + 3);
        strcat(printString, print->surname);
    }
    printString = realloc(printString, sizeof(char) * length + 3);
    strcat(printString, "\n");

    return printString;
}

char *myfgets(char *dst, int max, FILE *fp)
{
	int c;
	char *p;

	/* get max bytes or upto a newline */

	for (p = dst, max--; max > 0; max--) {
		if ((c = fgetc (fp)) == EOF)
			break;
		*p++ = c;
		if (c == '\r')
			break;
	}
	*p = 0;
	if (p == dst || c == EOF)
		return NULL;
	return (p);
}

//File Helper Functions

void writeEvent(void* toBePrinted, FILE *outFile, List list) {
    Event *print = (Event*)toBePrinted;

    fprintf(outFile, "1 %s\n", print->type);

    if (strcmp(print->date, "") != 0) {
        fprintf(outFile, "2 DATE %s\n", print->date);
    }

    if (strcmp(print->place, "") != 0) {
        fprintf(outFile, "2 PLAC %s\n", print->place);
    }

    return;
}

void writeFamily(void* toBePrinted, FILE *outFile, List list) {
    Xref *xref = (Xref*)toBePrinted;
    Family *print = (Family*)xref->data;

    fprintf(outFile, "0 @%s@ FAM\n", xref->id);

    if (print->husband != NULL) {
        Xref* xref = findXREF(list, &compareIndividuals, print->husband);
        fprintf(outFile, "1 HUSB @%s@\n", xref->id);
    }


    if (print->wife != NULL) {
        Xref* xref = findXREF(list, &compareIndividuals, print->wife);
        fprintf(outFile, "1 WIFE @%s@\n", xref->id);
    }

    if (print->events.length != 0) {
        toFile(print->events, list, outFile, &writeEvent);
    }

    if (print->children.length != 0) {
        ListIterator iter = createIterator(print->children);
	    Individual* child;
        while ((child = nextElement(&iter)) != NULL) {
            Xref* xref = findXREF(list, &compareIndividuals, child);
            fprintf(outFile, "1 CHIL @%s@\n", xref->id);
            }
        }

    return;
}

void writeField(void* toBePrinted, FILE *outFile, List list) {
    Field *print = (Field*)toBePrinted;

    if ((strcmp(print->tag, "GIVN") == 0) || (strcmp(print->tag, "SURN") == 0)) {
        return;
    }
    fprintf(outFile, "1 %s %s\n", print->tag, print->value);
    return;
}

void writeIndividual(void* toBePrinted, FILE *outFile, List list) {
    Xref *xref = (Xref*)toBePrinted;
    Individual *print = (Individual*)xref->data;

    fprintf(outFile, "0 @%s@ INDI\n", xref->id);
    fprintf(outFile, "1 NAME ");

    if (strcmp(print->givenName, "") != 0) {
        fprintf(outFile, "%s ", print->givenName);
    }

    if (strcmp(print->surname, "") != 0) {
        fprintf(outFile, "/%s/\n", print->surname);
    }

    else {
        fprintf(outFile, "//\n");
    }

    Field* tempField = malloc(sizeof(Field));
    tempField->tag = malloc(sizeof(char) * 5);
    tempField->value = malloc(sizeof(char) * strlen(print->givenName) + 2);
    strcpy(tempField->tag, "GIVN");
    strcpy(tempField->value, print->givenName);
    if (findElement(print->otherFields, &compareFieldsBool, tempField) != NULL) {
        fprintf(outFile, "2 GIVN %s\n", print->givenName);
    }
    deleteField(tempField);

    Field* tempField2 = malloc(sizeof(Field));
    tempField2->tag = malloc(sizeof(char) * 5);
    tempField2->value = malloc(sizeof(char) * strlen(print->surname) + 2);
    strcpy(tempField2->tag, "SURN");
    strcpy(tempField2->value, print->surname);
    if (findElement(print->otherFields, &compareFieldsBool, tempField2) != NULL) {
        fprintf(outFile, "2 SURN %s\n", print->surname);
    }
    deleteField(tempField2);
    

    if (print->events.length != 0) {
        toFile(print->events, list, outFile, &writeEvent);
    }

    if (print->otherFields.length != 0) {
        toFile(print->otherFields, list, outFile, &writeField);
    }
    
    if (print->families.length != 0) {
        ListIterator iter = createIterator(print->families);
	    Family* family;
        while ((family = nextElement(&iter)) != NULL) {
            Xref* xref = findXREF(list, &compareFamilies, family);

            if ((compareIndividuals(print, family->husband) == 0) || (compareIndividuals(print, family->wife) == 0)) {
                fprintf(outFile, "1 FAMS @%s@\n", xref->id);
            }

            else {
                fprintf(outFile, "1 FAMC @%s@\n", xref->id);
            }
        }
    }

    return;

}

void toFile(List list, List compareList, FILE *outFile, void (*printFunction)(void *toBePrinted, FILE *outFile, List list)) {

	ListIterator iter = createIterator(list);
		
	void* elem;
	while( (elem = nextElement(&iter)) != NULL){
		printFunction(elem, outFile, compareList);
	}
}

void* findXREF(List list, int (*customCompare)(const void* first,const void* second), const void* searchRecord) {
    ListIterator iter = createIterator(list);
	Xref* elem;

	while( (elem = nextElement(&iter)) != NULL){

        if (customCompare(elem->data, searchRecord) == 0) {
            return elem;
        }
    }
    return NULL;
}

bool compareFieldsBool(const void* first,const void* second) {
    if (compareFields(first, second) == 0) {
        return true;
    }

    else {
        return false;
    }
}

bool compareIndividualsBool(const void* first, const void *second) {
    Individual *a = (Individual*)first;
    Individual *b = (Individual*)second;

    if (strcmp(a->surname, b->surname) == 0) {
        if (strcmp(a->givenName, b->givenName) == 0) {
            return true;
        }
    }

    return false;
}

int compareIndividualsName(const void *first, const void* second) {
    Individual *a = (Individual*)first;
    Individual *b = (Individual*)second;
    if (strcmp(a->surname, b->surname) == 0) {
        return strcmp(a->givenName, b->givenName);
    }

    return strcmp(a->surname, b->surname);
}

void deleteGeneration(void* toBeDeleted) {
    List *delete = (List*)toBeDeleted;
    clearList(delete);
}
int compareGenerations(const void* first,const void* second) {
    List *a = (List*)first;
    List *b = (List*)second;

    ListIterator iterator = createIterator(*a);
    ListIterator iterator2 = createIterator(*b);
    Individual* individual;
    Individual* individual2;

    while( (individual = nextElement(&iterator)) != NULL) {
        individual2 = nextElement(&iterator2);
        if (individual2 == NULL) {
            return 1;
        }
        if (compareIndividuals(individual, individual2) != 0) {
            return 1;
        }
    }
    return 0;
}

char* printGeneration(void* toBePrinted) {
    List *print = (List*)toBePrinted;
    char *printString = malloc(sizeof(char) * strlen(toString(*print)) + 500);
    strcpy(printString, "**********************GENERATION*********************************\n");
    strcat(printString, toString(*print));
    return printString;
}


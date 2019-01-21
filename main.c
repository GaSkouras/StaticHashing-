#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifndef HT_H__
#define HT_H__

#include "HT.h"

#endif /* HT_H__*/

#include "SHT.h"

#define MAXBUCKETS 249

#define LINE_SIZE 256

int main(int argc, char **argv) {
    char buf[LINE_SIZE], *token;
    Record primaryRecord;
    SecondaryRecord secondaryRecord;
    FILE *fp;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <soure-file>\n", argv[0]);
        return 1;
    }

    if ((fp = fopen(argv[1], "r")) == NULL) {
        perror("fopen source-file");
        return 1;
    }

    BF_Init();

    HT_CreateIndex("HashTable", INT, "id", 15, MAXBUCKETS);

    //HT_PrintIndex("HashTable");

    HT_info *info = HT_OpenIndex("HashTable");
    printf("Till here all is good\n");

    char *tmp;

    while (fgets(buf, LINE_SIZE, fp) != NULL) {
        token = strtok(buf + 1, ",");
        primaryRecord.id = atoi(token);

        token = strtok(NULL, ",");

        token++;
        token[strlen(token) - 1] = 0;
        strncpy(primaryRecord.name, token, sizeof(primaryRecord.name));

        token = strtok(NULL, ",");
        token++;
        token[strlen(token) - 1] = 0;
        strncpy(primaryRecord.surname, token, sizeof(primaryRecord.surname));

        token = strtok(NULL, "}");
        strncpy(primaryRecord.address, token, sizeof(primaryRecord.address));

        HT_InsertEntry(info, primaryRecord);
    }

    fclose(fp);

    //HT_PrintIndex("HashTable");

    HT_CloseIndex(info);

    info = HT_OpenIndex("HashTable");

    printf("\n\n\n\n:::AFTER INSERT:::\n\n");

    HT_PrintIndex("HashTable");

    printf("\n\n\n\n:::DELETE:::\n\n");
    for (int i = 0; i < 10; i++) {
        Record record;
        record.id = i;
        sprintf(record.name, "name_%d", i);
        sprintf(record.surname, "surname_%d", i);
        sprintf(record.address, "address_%d", i);
        int err = HT_DeleteEntry(*info, (void *) &record.id);
    }

    HT_PrintIndex("HashTable");

    printf("\n\n\n\n:::DELETION COMPLETED:::\n\n");

    printf("\n\n\n\n:::AFTER DELETE:::\n\n");

    HT_PrintIndex("HashTable");

    printf("\n\n\n\n:::GET ALL ENTRIES OPERATION::\n\n");
    for (int i = 0; i < 1; i++) {
        Record record;
        record.id = 10;
        sprintf(record.name, "name_%d", i);
        sprintf(record.surname, "surname_%d", i);
        sprintf(record.address, "address_%d", i);
        int count = HT_GetAllEntries(*info, (void *) &record.id);
        printf("\nBlocks found: %d\n\n", count);
    }

    printf("\n\n\n\n:::GET ALL ENTRIES COMPLETED:::\n\n");

    printf("\n\n\n:::Secondary Index Creation:::\n\n\n");

    // Create secondary
    int createErrorCode = SHT_CreateSecondaryIndex("secondary.index", "name", 10, 20, "HashTable");
    if (createErrorCode < 0) {
        printf("Checkpoint Result 8: FAILED\n");
        return -1;
    }

    // Print secondary
    SHT_PrintIndex("secondary.index");

    // Open secondary
    SHT_info *shtInfo = SHT_OpenSecondaryIndex("secondary.index");

    // Insert secondar

    primaryRecord.id = 40;
    strcpy(primaryRecord.name, "Giannis");
    strcpy(primaryRecord.surname, "Xatzopoulos");
    strcpy(primaryRecord.address, "Taxilou");
    secondaryRecord.blockId = 10;
    secondaryRecord.offset = 2;
    secondaryRecord.record = primaryRecord;
    SHT_SecondaryInsertEntry(*shtInfo, secondaryRecord);

    // Print secondary
    SHT_PrintIndex("secondary.index");

    // Close secondary
    SHT_CloseSecondaryIndex(shtInfo);


    return 0;
}

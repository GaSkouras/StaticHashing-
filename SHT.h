#ifndef HT_H__
#define HT_H__

#include "HT.h"

#endif /* HT_H__*/

#define SHT_OK 0
#define SHT_ERROR -1

#pragma pack(1)
typedef struct {
    int fileDesc;
    int attrLength;
    long int numBuckets;
    char *attrName;
    char *fileName;
} SHT_info;

typedef struct {
    int blockId;
    int offset;
    Record record;
} SecondaryRecord;

void SHT_PrintIndex(char *sfileName);

int SHT_CreateSecondaryIndex(char *sfileName, char *attrName, int attrLength, int buckets, char *fileName);

SHT_info *SHT_OpenSecondaryIndex(char *sfileName);

int SHT_CloseSecondaryIndex(SHT_info *header_info);

int SHT_SecondaryInsertEntry(SHT_info header_info, SecondaryRecord record);

int SHT_SecondaryGetAllEntries(SHT_info header_info_sht, HT_info header_info_ht, void *value);

#include "SHT.h"


/*Private functions*/

void _SHT_printRecord(SHT_info *header_info, SecondaryRecord *record) {
    printf("{ 'blockId':'%d', 'offset':'%d', 'id':'%d', ", record->blockId, record->offset, record->record.id);
    if (!strcmp(header_info->attrName, NAME)) {
        printf("'name':'%s' }\n", record->record.name);
    } else if (!strcmp(header_info->attrName, SURNAME)) {
        printf("'surname':'%s' }\n", record->record.surname);
    } else if (!strcmp(header_info->attrName, ADDRESS)) {
        printf("'address':'%s' }\n", record->record.address);
    } else
        exit(1);
}

void _SHT_getCount(void *block, int *count) {
    memcpy(count, block + BLOCK_SIZE - 2 * sizeof(int), sizeof(int));
}

void _SHT_setCount(void *block, const int count) {
    memcpy(block + BLOCK_SIZE - 2 * sizeof(int), &count, sizeof(int));
}

void _SHT_getNext(void *block, int *next) {
    memcpy(next, block + BLOCK_SIZE - sizeof(int), sizeof(int));
}

void _SHT_setNext(void *block, const int next) {
    memcpy(block + BLOCK_SIZE - sizeof(int), &next, sizeof(int));
}

void _SHT_getRecord(SHT_info *header_info, void *block, const int recordOffset, SecondaryRecord *record) {
    int fieldOffset = 0;
    memcpy(&record->blockId, block + recordOffset, sizeof(record->blockId));
    fieldOffset += sizeof(record->blockId);
    memcpy(&record->offset, block + recordOffset + fieldOffset, sizeof(record->offset));
    fieldOffset += sizeof(record->offset);
    memcpy(&record->record.id, block + recordOffset + fieldOffset, sizeof(record->record.id));
    fieldOffset += sizeof(record->record.id);
    if (!strcmp(header_info->attrName, NAME)) {
        memcpy(&record->record.name, block + recordOffset + fieldOffset, sizeof(record->record.name));
    } else if (!strcmp(header_info->attrName, SURNAME)) {
        memcpy(&record->record.surname, block + recordOffset + fieldOffset, sizeof(record->record.surname));
    } else if (!strcmp(header_info->attrName, ADDRESS)) {
        memcpy(&record->record.address, block + recordOffset + fieldOffset, sizeof(record->record.address));
    } else
        exit(1);
}

void _SHT_setRecord(SHT_info *header_info, void *block, const int recordOffset, const SecondaryRecord *record) {
    int fieldOffset = 0;
    memcpy(block + recordOffset, &record->blockId, sizeof(record->blockId));
    fieldOffset += sizeof(record->blockId);
    memcpy(block + recordOffset + fieldOffset, &record->offset, sizeof(record->offset));
    fieldOffset += sizeof(record->offset);
    memcpy(block + recordOffset + fieldOffset, &record->record.id, sizeof(record->record.id));
    fieldOffset += sizeof(record->record.id);
    if (!strcmp(header_info->attrName, NAME)) {
        memcpy(block + recordOffset + fieldOffset, &record->record.name, sizeof(record->record.name));
    } else if (!strcmp(header_info->attrName, SURNAME)) {
        memcpy(block + recordOffset + fieldOffset, &record->record.surname, sizeof(record->record.surname));
    } else if (!strcmp(header_info->attrName, ADDRESS)) {
        memcpy(block + recordOffset + fieldOffset, &record->record.address, sizeof(record->record.address));
    } else
        exit(1);
}

void _SHT_getBucketValue(void *block, const int offset, int *bucketValue) {
    memcpy(bucketValue, block + offset, sizeof(int));
}

void _SHT_setBucketValue(void *block, const int offset, const int bucketValue) {
    memcpy(block + offset, &bucketValue, sizeof(int));
}

int _SHT_getMaxBuckets(const int offset) {
    return (BLOCK_SIZE - (2 * sizeof(int)) - offset) / sizeof(int);
}

/*http://www.cse.yorku.ca/~oz/hash.html*/
long int _SHT_int_h(long int buckets, int value) {
    return (long int) value % buckets;
}

/*http://www.cse.yorku.ca/~oz/hash.html*/
long int _SHT_char_h(long int buckets, char *hash_name) {
    long int hash_value = 0;
    int i;
    while ((i = *hash_name++))
        hash_value = i + (hash_value << 6) + (hash_value << 16) - hash_value;
    hash_value = hash_value % buckets;
    return (long int) hash_value;
}

int _SHT_recInitializeBuckets(const int fd, int offset, const int startBLock, const int buckets) {
    int maxBuckets = (BLOCK_SIZE - (2 * sizeof(int)) - offset) / sizeof(int);
    int count = 0, next = 0, remainingBuckets = buckets;
    void *block;

    if (startBLock > 0) {
        if (BF_AllocateBlock(fd) < 0) {
            BF_PrintError("");
            return SHT_ERROR;
        }
    }

    if (BF_ReadBlock(fd, startBLock, &block) < 0) {
        BF_PrintError("");
        return SHT_ERROR;
    }

    int x = (buckets > maxBuckets) ? maxBuckets : buckets;

    for (int i = offset; i < offset + x * sizeof(int); i += sizeof(int)) {
        _SHT_setBucketValue(block, i, -1);
        remainingBuckets--;
    }

    if (remainingBuckets) {
        next = _SHT_recInitializeBuckets(fd, 0, startBLock + 1, remainingBuckets);
    }

    _SHT_setCount(block, --count);

    _SHT_setNext(block, next);

    if (BF_WriteBlock(fd, startBLock) < 0) {
        BF_PrintError("");
        return SHT_ERROR;
    }

    return startBLock;
}

int _SHT_insertEntry(SHT_info *header_info, const int bucketValue, SecondaryRecord *record) {
    int currentBlock = 0, maxRecords = 0, next = 0, count = 0, offset = 0;;
    void *block;
    if (!strcmp(header_info->attrName, ID)) {
        maxRecords = (BLOCK_SIZE - 2 * sizeof(int)) /
                     (sizeof(record->record.id) + sizeof(record->blockId) + sizeof(record->offset));
    } else if (!strcmp(header_info->attrName, NAME)) {
        maxRecords = (BLOCK_SIZE - 2 * sizeof(int)) /
                     (sizeof(record->record.name) + sizeof(record->blockId) + sizeof(record->offset));
    } else if (!strcmp(header_info->attrName, SURNAME)) {
        maxRecords = (BLOCK_SIZE - 2 * sizeof(int)) /
                     (sizeof(record->record.surname) + sizeof(record->blockId) + sizeof(record->offset));
    } else if (!strcmp(header_info->attrName, ADDRESS)) {
        maxRecords = (BLOCK_SIZE - 2 * sizeof(int)) /
                     (sizeof(record->record.address) + sizeof(record->blockId) + sizeof(record->offset));
    } else
        return SHT_ERROR;

    // Check if the bucket points to a block
    if (bucketValue > 0) {
        currentBlock = bucketValue;
        do {

            if (BF_ReadBlock(header_info->fileDesc, currentBlock, &block) < 0) {
                BF_PrintError("");
                return SHT_ERROR;
            }

            _SHT_getCount(block, &count);

            _SHT_getNext(block, &next);

            if (count < maxRecords) {

                _SHT_setRecord(header_info, block, count, record);

                _SHT_setCount(block, ++count);
                next = 0;
            } else {
                if (next <= 0) {

                    if (BF_AllocateBlock(header_info->fileDesc) < 0) {
                        BF_PrintError("");
                        return SHT_ERROR;
                    }

                    next = BF_GetBlockCounter(header_info->fileDesc) - 1;
                    if (next < 0) {
                        BF_PrintError("");
                        exit(1);
                    }

                    _SHT_setNext(block, next);
                }
            }

            if (BF_WriteBlock(header_info->fileDesc, currentBlock) < 0) {
                BF_PrintError("");
                return SHT_ERROR;
            }

            currentBlock = next;
        } while (next);

        return bucketValue;

    } else {
        // Bucket does not point to any block yet
        if (BF_AllocateBlock(header_info->fileDesc) < 0) {
            BF_PrintError("");
            return SHT_ERROR;
        }

        currentBlock = BF_GetBlockCounter(header_info->fileDesc) - 1;
        if (currentBlock < 0) {
            BF_PrintError("");
            exit(1);
        }

        if (BF_ReadBlock(header_info->fileDesc, currentBlock, &block) < 0) {
            BF_PrintError("");
            return SHT_ERROR;
        }

        _SHT_setRecord(header_info, block, 0, record);

        _SHT_setCount(block, 1);

        _SHT_setNext(block, 0);

        if (BF_WriteBlock(header_info->fileDesc, currentBlock) < 0) {
            BF_PrintError("");
            return SHT_ERROR;
        }
        return currentBlock;
    }
}


void _SHT_init(char *filename) {
    void *bucketBlock, *recordBlock;
    int headerInfoSize = 0, startOffset = 0, currentBlock = 0, next = 0, maxBuckets = 0,
            i = 0, bucketValue = 0, currentRecordBlock = 0, countRecordBlock = 0,
            nextRecordBlock = 0;
    Record record;
    HT_info *info = HT_OpenIndex(filename);

    headerInfoSize = sizeof(HT_info) - 8 + strlen(info->attrName) + 1;
    startOffset = headerInfoSize;
    do {
        if (BF_ReadBlock(info->fileDesc, currentBlock, &bucketBlock) < 0) {
            BF_PrintError("");
        }
        _SHT_getNext(bucketBlock, &next);
        maxBuckets = _SHT_getMaxBuckets(startOffset);

        printf("\nBLOCK: %d\n", currentBlock);
        for (i = startOffset; i < maxBuckets * sizeof(int); i += sizeof(int)) {
            _SHT_getBucketValue(bucketBlock, i, &bucketValue);
            if (bucketValue > 0) {
                printf("::%d::\n", bucketValue);
                currentRecordBlock = bucketValue;
                do {
                    if (BF_ReadBlock(info->fileDesc, currentRecordBlock, &recordBlock) < 0) {
                        BF_PrintError("");
                    }
                    _SHT_getCount(recordBlock, &countRecordBlock);

                    _SHT_getNext(recordBlock, &nextRecordBlock);

                    for (int j = 0; j < countRecordBlock; j++) {
                        //_SHT_getRecord(recordBlock, j, &record);
                       // _SHT_printRecord(&record);
                    }
                    currentRecordBlock = nextRecordBlock;
                } while (nextRecordBlock);

            }
            startOffset = 0;
        }

        currentBlock = next;
    } while (next);


    HT_CloseIndex(info);
}


/*Public funtions*/

void SHT_PrintIndex(char *sfileName) {
    void *block;

    SHT_info *header_info = SHT_OpenSecondaryIndex(sfileName);

    printf("\n\nHT header_info NODE: { fileDesc: %d, attrLength: %d, numBuckets: %ld, attrName: %s, fileName: %s}\n",
           header_info->fileDesc,
           header_info->attrLength,
           header_info->numBuckets,
           header_info->attrName,
           header_info->fileName
    );

    int blocks = BF_GetBlockCounter(header_info->fileDesc);

    if (blocks < 0) {
        BF_PrintError("");
    }

    int offset =
            sizeof(SHT_info) - 8 + (int) strlen(header_info->attrName) + 1 - 8 + (int) strlen(header_info->fileName) +
            1;
    int value = 0;
    int firstBlockMaxBuckets = (BLOCK_SIZE - offset - 2 * sizeof(int)) / sizeof(int);
    int otherBlocksMaxBuckets = (BLOCK_SIZE - (2 * sizeof(int))) / sizeof(int);
    for (int i = 0; i < blocks; i++) {
        printf("\n\nBLOCK: %d\n", i);
        if (BF_ReadBlock(header_info->fileDesc, i, &block) < 0) {
            BF_PrintError("");
            exit(1);
        }
        int x = i > 0 ? otherBlocksMaxBuckets : firstBlockMaxBuckets;
        int next = 0, count = 0;

        _SHT_getCount(block, &count);

        _SHT_getNext(block, &next);

        if (count >= 0) {
            SecondaryRecord record;
            for (int j = 0; j < count; j++) {
                _SHT_getRecord(header_info, block, j, &record);
                _SHT_printRecord(header_info, &record);
            }
        } else {
            for (int j = offset; j < offset + x * sizeof(int); j += sizeof(int)) {
                memcpy(&value, block + j, sizeof(int));
                if (value > 0)
                    printf("%d ", value);
                else if (value == -1)
                    printf("• ");
                else {
                    printf("  ");
                }
            }
        }

        printf("\nCount: %d, Next: %d\n\n", count, next);

        offset = 0;
    }

    if (BF_CloseFile(header_info->fileDesc) < 0) {
        BF_PrintError("");
        exit(1);
    }

}

int SHT_CreateSecondaryIndex(char *sfileName, char *attrName, int attrLength, int buckets, char *fileName) {
    void *initBlock;
    int offset = 0, fd = 0, x = 0;

    if (buckets <= 0) {
        return SHT_ERROR;
    }

    if (BF_CreateFile(sfileName) < 0) {
        BF_PrintError("");
        BF_CloseFile(fd);
    }

    fd = BF_OpenFile(sfileName);
    if (fd < 0) {
        BF_PrintError("");
        return SHT_ERROR;
    }

    if (BF_AllocateBlock(fd) < 0) {
        BF_PrintError("");
        BF_CloseFile(fd);
        return SHT_ERROR;
    }

    if (BF_ReadBlock(fd, 0, &initBlock) < 0) {
        BF_PrintError("");
        BF_CloseFile(fd);
        return SHT_ERROR;
    }

    memcpy(initBlock, &fd, sizeof(int));
    offset += sizeof(int);

    memcpy(initBlock + offset, &attrLength, sizeof(int));
    offset += sizeof(int);

    long int b = (long int) buckets;
    memcpy(initBlock + offset, &(b), sizeof(long int));
    offset += sizeof(long int);

    memcpy(initBlock + offset, attrName, sizeof(char) * strlen(attrName) + 1);
    offset += sizeof(char) * strlen(attrName) + 1;

    memcpy(initBlock + offset, fileName, sizeof(char) * strlen(fileName) + 1);
    offset += sizeof(char) * strlen(fileName) + 1;

    x = _SHT_recInitializeBuckets(fd, offset, 0, buckets);
    if (x < 0) {
        return SHT_ERROR;
    }

    if (BF_WriteBlock(fd, 0) < 0) {
        BF_PrintError("");
        BF_CloseFile(fd);
    }

    if (BF_CloseFile(fd) < 0) {
        BF_PrintError("");
        return SHT_ERROR;
    }

    return SHT_OK;
}

SHT_info *SHT_OpenSecondaryIndex(char *sfileName) {
    int fd = 0, offset = 0;
    void *block;
    SHT_info tmp;
    SHT_info *info;
    fd = BF_OpenFile(sfileName);

    if (fd < 0) {
        BF_PrintError("");
        exit(1);
    }

    if (BF_ReadBlock(fd, 0, &block) < 0) {
        BF_PrintError("");
        BF_CloseFile(fd);
    }

    memcpy(&tmp.fileDesc, block, sizeof(int));
    offset += sizeof(int);

    memcpy(&tmp.attrLength, block + offset, sizeof(int));
    offset += sizeof(int);

    memcpy(&tmp.numBuckets, block + offset, sizeof(long int));
    offset += sizeof(long int);

    tmp.attrName = (char *) malloc(strlen((char *) (block + offset)) * sizeof(char) + 1);
    strcpy(tmp.attrName, (char *) (block + offset));
    offset += (int) strlen((char *) (block + offset)) * sizeof(char) + 1;

    tmp.fileName = (char *) malloc(strlen((char *) (block + offset)) * sizeof(char) + 1);
    strcpy(tmp.fileName, (char *) (block + offset));
    offset += (int) strlen((char *) (block + offset)) * sizeof(char) + 1;

    info = malloc(sizeof(SHT_info));
    info->fileDesc = fd;
    info->attrLength = tmp.attrLength;
    info->numBuckets = tmp.numBuckets;
    info->attrName = tmp.attrName;
    info->fileName = tmp.fileName;
    return info;
}

int SHT_CloseSecondaryIndex(SHT_info *header_info) {
    if (BF_CloseFile(header_info->fileDesc) < 0) {
        BF_PrintError("");
        return SHT_ERROR;
    }
    free(header_info);
    return SHT_OK;
}

int SHT_SecondaryInsertEntry(SHT_info header_info, SecondaryRecord record) {
    void *block;
    int headerInfoSize = 0, bucketValue = 0, maxBuckets = 0, nextBlock = 0, p = 0, bucketOffset = 0, startOffset = 0, targetBlock = 0;
    long int targetBucket;

    if (!strcmp(header_info.attrName, ID)) {
        targetBucket = _SHT_int_h(header_info.numBuckets, record.record.id);
    } else if (!strcmp(header_info.attrName, NAME)) {
        targetBucket = _SHT_char_h(header_info.numBuckets, record.record.name);
    } else if (!strcmp(header_info.attrName, SURNAME)) {
        targetBucket = _SHT_char_h(header_info.numBuckets, record.record.surname);
    } else if (!strcmp(header_info.attrName, ADDRESS)) {
        targetBucket = _SHT_char_h(header_info.numBuckets, record.record.address);
    } else
        return SHT_ERROR;

    headerInfoSize =
            sizeof(SHT_info) - 8 + (int) strlen(header_info.attrName) + 1 - 8 + (int) strlen(header_info.fileName) + 1;
    startOffset = headerInfoSize;
    do {
        maxBuckets = _SHT_getMaxBuckets(startOffset);
        p = p + maxBuckets;
        bucketOffset = ((int) targetBucket - (p - (p / maxBuckets * maxBuckets))) % maxBuckets;

        if (BF_ReadBlock(header_info.fileDesc, targetBlock, &block) < 0) {
            BF_PrintError("");
            return SHT_ERROR;
        }

        memcpy(&nextBlock, block + BLOCK_SIZE - sizeof(int), sizeof(int));

        if (nextBlock > 0 && targetBucket >= p) {
            targetBlock = nextBlock;
        }
        if (targetBlock > 0)
            startOffset = 0;
    } while (targetBucket >= p);


    _SHT_getBucketValue(block, startOffset + (bucketOffset * sizeof(int)), &bucketValue);

    bucketValue = _SHT_insertEntry(&header_info, bucketValue, &record);

    _SHT_setBucketValue(block, startOffset + (bucketOffset * sizeof(int)), bucketValue);

    if (BF_WriteBlock(header_info.fileDesc, targetBlock) < 0) {
        BF_PrintError("");
        return SHT_ERROR;
    }

    return SHT_OK;
}

int SHT_SecondaryGetAllEntries(SHT_info header_info_sht, HT_info header_info_ht, void *value) {

}

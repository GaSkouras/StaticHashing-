#include <printf.h>
#include "HT.h"
#include "lib/BF.h"


/*Private functions*/

void _printRecord(Record *record) {
    printf("{ 'id':'%d', 'name':'%s', 'surname':'%s', 'address':'%s' }\n",
           record->id,
           record->name,
           record->surname,
           record->address);
}

void _getCount(void *block, int *count) {
    memcpy(count, block + BLOCK_SIZE - 2 * sizeof(int), sizeof(int));
}

void _setCount(void *block, const int count) {
    memcpy(block + BLOCK_SIZE - 2 * sizeof(int), &count, sizeof(int));
}

void _getNext(void *block, int *next) {
    memcpy(next, block + BLOCK_SIZE - sizeof(int), sizeof(int));
}

void _setNext(void *block, const int next) {
    memcpy(block + BLOCK_SIZE - sizeof(int), &next, sizeof(int));
}

void _getRecord(void *block, const int offset, Record *record) {
    memcpy(record, block + offset * sizeof(Record), sizeof(Record));
}

void _setRecord(void *block, const int offset, const Record *record) {
    memcpy(block + offset * sizeof(Record), record, sizeof(Record));
}

void _getBucketValue(void *block, const int offset, int *bucketValue) {
    memcpy(bucketValue, block + offset, sizeof(int));
}

void _setBucketValue(void *block, const int offset, const int bucketValue) {
    memcpy(block + offset, &bucketValue, sizeof(int));
}

int _getMaxRecords() {
    return (BLOCK_SIZE - 2 * sizeof(int)) / sizeof(Record);
}

int _getMaxBuckets(const int offset) {
    return (BLOCK_SIZE - (2 * sizeof(int)) - offset) / sizeof(int);
}

long int _int_h(long int buckets, int value) {
    return (long int) value % buckets;
}

long int _char_h(long int buckets, char *hash_name) {
    long int hash_value = 0;
    int i;
    while ((i = *hash_name++))
        hash_value = i + (hash_value << 6) + (hash_value << 16) - hash_value;
    hash_value = hash_value % buckets;
    return (long int) hash_value;
}

int _recordCmp(HT_info *header_info, Record *record, void *value) {
    switch (header_info->attrType) {
        case INT:
            if (!strcmp(header_info->attrName, ID))
                return record->id == *((int *) value);
        case CHAR:
            if (!strcmp(header_info->attrName, NAME))
                return !strcmp(record->name, (char *) value);
            else if (!strcmp(header_info->attrName, SURNAME))
                return !strcmp(record->surname, (char *) value);
            else if (!strcmp(header_info->attrName, ADDRESS))
                return !strcmp(record->address, (char *) value);
        default:
            return -9999;
    }
}

int _recInitializeBuckets(const int fd, int offset, const int startBLock, const int buckets) {
    int maxBuckets = (BLOCK_SIZE - (2 * sizeof(int)) - offset) / sizeof(int);
    int count = 0, next = 0, remainingBuckets = buckets;
    void *block;

    if (startBLock > 0) {
        if (BF_AllocateBlock(fd) < 0) {
            BF_PrintError("");
            return HT_ERROR;
        }
    }

    if (BF_ReadBlock(fd, startBLock, &block) < 0) {
        BF_PrintError("");
        return HT_ERROR;
    }

    int x = (buckets > maxBuckets) ? maxBuckets : buckets;

    for (int i = offset; i < offset + x * sizeof(int); i += sizeof(int)) {
        _setBucketValue(block, i, -1);
        remainingBuckets--;
    }

    if (remainingBuckets) {
        next = _recInitializeBuckets(fd, 0, startBLock + 1, remainingBuckets);
    }

    _setCount(block, --count);

    _setNext(block, next);

    if (BF_WriteBlock(fd, startBLock) < 0) {
        BF_PrintError("");
        return HT_ERROR;
    }

    return startBLock;
}

int _insertEntry(const int fd, const int bucketValue, Record *record) {
    int currentBlock = 0, maxRecords = 0, next = 0, count = 0;
    void *block;
    maxRecords = _getMaxRecords();

    // Check if the bucket points to a block
    if (bucketValue > 0) {
        currentBlock = bucketValue;
        do {

            if (BF_ReadBlock(fd, currentBlock, &block) < 0) {
                BF_PrintError("");
                return HT_ERROR;
            }

            _getCount(block, &count);

            _getNext(block, &next);

            if (count < maxRecords) {
                _setRecord(block, count, record);
                _setCount(block, ++count);
                next = 0;
            } else {
                if (next <= 0) {

                    if (BF_AllocateBlock(fd) < 0) {
                        BF_PrintError("");
                        return HT_ERROR;
                    }

                    next = BF_GetBlockCounter(fd) - 1;
                    if (next < 0) {
                        BF_PrintError("");
                        exit(1);
                    }

                    _setNext(block, next);

                }
            }

            if (BF_WriteBlock(fd, currentBlock) < 0) {
                BF_PrintError("");
                return HT_ERROR;
            }

            currentBlock = next;
        } while (next);

        return bucketValue;

    } else {
        // Bucket does not point to any block yet
        if (BF_AllocateBlock(fd) < 0) {
            BF_PrintError("");
            return HT_ERROR;
        }

        currentBlock = BF_GetBlockCounter(fd) - 1;
        if (currentBlock < 0) {
            BF_PrintError("");
            exit(1);
        }

        if (BF_ReadBlock(fd, currentBlock, &block) < 0) {
            BF_PrintError("");
            return HT_ERROR;
        }

        _setRecord(block, 0, record);

        _setCount(block, 1);

        _setNext(block, 0);

        if (BF_WriteBlock(fd, currentBlock) < 0) {
            BF_PrintError("");
            return HT_ERROR;
        }
        return currentBlock;
    }
}

int _deleteEntry(HT_info *info, int bucketValue, void *value) {
    int currentBlock = 0, next = 0, count = 0, offset = 0, targetBlockNumber = 0, lastRecordBlockNumber = 0;
    void *block = NULL, *targetBlock = NULL, *lastRecordBlock = NULL;
    Record record;

    // Check if the bucket points to a block
    if (bucketValue > 0) {
        currentBlock = bucketValue;
        do {
            if (BF_ReadBlock(info->fileDesc, currentBlock, &block) < 0) {
                BF_PrintError("");
                return HT_ERROR;
            }

            _getCount(block, &count);

            _getNext(block, &next);

            for (int i = 0; i < count; i++) {

                _getRecord(block, i, &record);

                lastRecordBlock = block;
                lastRecordBlockNumber = currentBlock;

                if (_recordCmp(info, &record, value)) {
                    targetBlock = block;
                    offset = i * sizeof(Record);
                    targetBlockNumber = currentBlock;
                }
            }

            currentBlock = next;

        } while (next);

        if (targetBlock != NULL) {

            memcpy(targetBlock + offset, &record, sizeof(Record));

            if (BF_WriteBlock(info->fileDesc, targetBlockNumber) < 0) {
                BF_PrintError("");
                return HT_ERROR;
            }

            _getCount(lastRecordBlock, &count);

            if (count > 0) {
                _setCount(lastRecordBlock, --count);
                if (BF_WriteBlock(info->fileDesc, lastRecordBlockNumber) < 0) {
                    BF_PrintError("");
                    return HT_ERROR;
                }
            }

            return HT_OK;
        }

    }
    return HT_ERROR;
}

int _getAllEntries(HT_info *info, int bucketValue, void *value) {
    void *block;
    int currentBlock = 0, next = 0, count = 0, totalBlocks = 0, blocks = 0;
    Record record;

    if (bucketValue > 0) {

        currentBlock = bucketValue;
        do {

            if (BF_ReadBlock(info->fileDesc, currentBlock, &block) < 0) {
                BF_PrintError("");
                return HT_ERROR;
            }

            _getCount(block, &count);

            _getNext(block, &next);

            for (int i = 0; i < count; i++) {
                _getRecord(block, i, &record);
                _printRecord(&record);
                if (_recordCmp(info, &record, value)) {
                    printf("Requested value found in record!\n");
                    printf("|| %d || %s || %s || %s || \n \n", record.id, record.name, record.surname, record.address);
                    return blocks;
                } else {
                    printf("Not such value found in any record..\n");

                }
            }
            currentBlock = next;
            blocks++;
        } while (next);
    }
    return HT_ERROR;
}


/*Public functions*/

void HT_PrintIndex(char *fileName) {
    void *block;

    HT_info *header_info = HT_OpenIndex(fileName);

    printf("\n\nHT header_info NODE: { fileDesc: %d, attrType: %c, attrName: %s, attrLength: %d, numBuckets: %ld }\n",
           header_info->fileDesc,
           header_info->attrType,
           header_info->attrName,
           header_info->attrLength,
           header_info->numBuckets
    );

    int blocks = BF_GetBlockCounter(header_info->fileDesc);

    if (blocks < 0) {
        BF_PrintError("");
    }

    int offset = sizeof(HT_info) - 8 + strlen(header_info->attrName) + 1;
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

        _getCount(block, &count);

        _getNext(block, &next);

        if (count >= 0) {
            Record record;
            for (int j = 0; j < count; j++) {
                _getRecord(block, j, &record);
                _printRecord(&record);
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

int HT_CreateIndex(char *fileName, char attrType, char *attrName, int attrLength, int buckets) {
    void *initBlock;
    int offset = 0, fd = 0, x = 0;

    if (buckets <= 0) {
        return HT_ERROR;
    }

    if (BF_CreateFile(fileName) < 0) {
        BF_PrintError("");
        BF_CloseFile(fd);
    }

    fd = BF_OpenFile(fileName);
    if (fd < 0) {
        BF_PrintError("");
        return HT_ERROR;
    }

    if (BF_AllocateBlock(fd) < 0) {
        BF_PrintError("");
        BF_CloseFile(fd);
        return HT_ERROR;
    }

    if (BF_ReadBlock(fd, 0, &initBlock) < 0) {
        BF_PrintError("");
        BF_CloseFile(fd);
        return HT_ERROR;
    }

    memcpy(initBlock, &fd, sizeof(int));
    offset += sizeof(int);

    memcpy(initBlock + offset, &attrType, sizeof(char));
    offset += sizeof(char);

    memcpy(initBlock + offset, &attrLength, sizeof(int));
    offset += sizeof(int);

    long int b = (long int) buckets;
    memcpy(initBlock + offset, &(b), sizeof(long int));
    offset += sizeof(long int);

    memcpy(initBlock + offset, attrName, sizeof(char) * (int) strlen(attrName) + 1);
    offset += sizeof(char) * (int) strlen(attrName) + 1;

    x = _recInitializeBuckets(fd, offset, 0, buckets);
    if (x < 0) {
        return HT_ERROR;
    }

    if (BF_WriteBlock(fd, 0) < 0) {
        BF_PrintError("");
        BF_CloseFile(fd);
    }

    if (BF_CloseFile(fd) < 0) {
        BF_PrintError("");
        return HT_ERROR;
    }

    return HT_OK;
}

HT_info *HT_OpenIndex(char *fileName) {
    int fd = 0, offset = 0;
    void *block;
    HT_info tmp;
    HT_info *header_info;
    fd = BF_OpenFile(fileName);

    if (fd < 0) {
        BF_PrintError("");
        exit(1);
    }

    int x = BF_GetBlockCounter(fd);
    if (x < 0) {
        BF_PrintError("");
        exit(1);
    }

    if (BF_ReadBlock(fd, 0, &block) < 0) {
        BF_PrintError("");
        BF_CloseFile(fd);
    }

    memcpy(&tmp.fileDesc, block, sizeof(int));
    offset += sizeof(int);

    memcpy(&tmp.attrType, block + offset, sizeof(char));
    offset += sizeof(char);

    memcpy(&tmp.attrLength, block + offset, sizeof(int));
    offset += sizeof(int);

    memcpy(&tmp.numBuckets, block + offset, sizeof(long int));
    offset += sizeof(long int);

    tmp.attrName = (char *) malloc((int) strlen((char *) (block + offset)) * sizeof(char) + 1);
    strcpy(tmp.attrName, (char *) (block + offset));
    offset += (int) strlen((char *) (block + offset)) * sizeof(char) + 1;

    header_info = malloc(sizeof(HT_info));
    header_info->fileDesc = fd; //tmp.fileDesc;
    header_info->attrType = tmp.attrType;
    header_info->attrLength = tmp.attrLength;
    header_info->numBuckets = tmp.numBuckets;
    header_info->attrName = tmp.attrName;
    return header_info;
}

int HT_CloseIndex(HT_info *header_info) {
    if (BF_CloseFile(header_info->fileDesc) < 0) {
        BF_PrintError("");
        return HT_ERROR;
    }
    free(header_info);
    return HT_OK;
}

int HT_InsertEntry(HT_info *header_info, Record record) {
    void *block;
    int headerInfoSize = 0, bucketValue = 0, maxBuckets = 0, nextBlock = 0, p = 0, targetBlock = 0, bucketOffset = 0, startOffset = 0;

    long int targetBucket;

    switch (header_info->attrType) {
        case INT:
            if (!strcmp(header_info->attrName, ID))
                targetBucket = _int_h(header_info->numBuckets, record.id);
            else
                targetBucket = HT_ERROR;
            break;
        case CHAR:
            if (!strcmp(header_info->attrName, NAME))
                targetBucket = _char_h(header_info->numBuckets, record.name);
            else if (!strcmp(header_info->attrName, SURNAME))
                targetBucket = _char_h(header_info->numBuckets, record.surname);
            else if (!strcmp(header_info->attrName, ADDRESS))
                targetBucket = _char_h(header_info->numBuckets, record.address);
            else
                targetBucket = HT_ERROR;
            break;
        default:
            return HT_ERROR;
    }

    headerInfoSize = sizeof(HT_info) - 8 + strlen(header_info->attrName) + 1;

    startOffset = headerInfoSize;
    do {
        maxBuckets = _getMaxBuckets(startOffset);
        p = p + maxBuckets;
        bucketOffset = ((int) targetBucket - (p - (p / maxBuckets * maxBuckets))) % maxBuckets;

        if (BF_ReadBlock(header_info->fileDesc, targetBlock, &block) < 0) {
            BF_PrintError("");
            return HT_ERROR;
        }

        memcpy(&nextBlock, block + BLOCK_SIZE - sizeof(int), sizeof(int));

        if (nextBlock > 0 && targetBucket >= p) {
            targetBlock = nextBlock;
        }
        if (targetBlock > 0)
            startOffset = 0;
    } while (targetBucket >= p);


    _getBucketValue(block,  startOffset + (bucketOffset * sizeof(int)), &bucketValue);

    bucketValue = _insertEntry(header_info->fileDesc, bucketValue, &record);

    _setBucketValue(block, startOffset + (bucketOffset * sizeof(int)), bucketValue);

    if (BF_WriteBlock(header_info->fileDesc, targetBlock) < 0) {
        BF_PrintError("");
        return HT_ERROR;
    }

    return HT_OK;
}

int HT_DeleteEntry(HT_info header_info, void *value) {
    void *block;
    int bucketValue = 0, headerInfoSize = 0, maxBuckets = 0, nextBlock = 0, p = 0, targetBlock = 0, bucketOffset = 0, startOffset = 0;
    long int targetBucket;


    switch (header_info.attrType) {
        case INT:
            if (!strcmp(header_info.attrName, ID))
                targetBucket = _int_h(header_info.numBuckets, *((int *) value));
            else
                targetBucket = -9999;
            break;
        case CHAR:
            if (!strcmp(header_info.attrName, NAME))
                targetBucket = _char_h(header_info.numBuckets, (char *) value);
            else if (!strcmp(header_info.attrName, SURNAME))
                targetBucket = _char_h(header_info.numBuckets, (char *) value);
            else if (!strcmp(header_info.attrName, ADDRESS))
                targetBucket = _char_h(header_info.numBuckets, (char *) value);
            else
                targetBucket = -9999;
            break;
        default:
            targetBucket = -999;
    }

    headerInfoSize = sizeof(HT_info) - 8 + strlen(header_info.attrName) + 1;

    startOffset = headerInfoSize;
    do {
        maxBuckets = _getMaxBuckets(startOffset);

        p = p + maxBuckets;

        bucketOffset = ((int) targetBucket - (p - (p / maxBuckets * maxBuckets))) % maxBuckets;

        if (BF_ReadBlock(header_info.fileDesc, targetBlock, &block) < 0) {
            BF_PrintError("");
            return HT_ERROR;
        }

        _getNext(block, &nextBlock);

        if (nextBlock > 0 && targetBucket >= p) {
            targetBlock = nextBlock;
        }

        if (targetBlock > 0) {
            startOffset = 0;
        }


    } while (targetBucket >= p);

    _getBucketValue(block, startOffset + (bucketOffset * sizeof(int)), &bucketValue);

    return _deleteEntry(&header_info, bucketValue, value);
}

int HT_GetAllEntries(HT_info header_info, void *value) {

    void *block;
    int bucketValue = 0, headerInfoSize = 0, maxBuckets = 0, nextBlock = 0, p = 0, targetBlock = 0, bucketOffset = 0, startOffset = 0;
    long int targetBucket;

    switch (header_info.attrType) {
        case INT:
            if (!strcmp(header_info.attrName, ID))
                targetBucket = _int_h(header_info.numBuckets, *((int *) value));
            else
                targetBucket = -9999;
            break;
        case CHAR:
            if (!strcmp(header_info.attrName, NAME))
                targetBucket = _char_h(header_info.numBuckets, (char *) value);
            else if (!strcmp(header_info.attrName, SURNAME))
                targetBucket = _char_h(header_info.numBuckets, (char *) value);
            else if (!strcmp(header_info.attrName, ADDRESS))
                targetBucket = _char_h(header_info.numBuckets, (char *) value);
            else
                targetBucket = -9999;
            break;
        default:
            targetBucket = -999;
    }

    headerInfoSize = sizeof(HT_info) - 8 + strlen(header_info.attrName) + 1;

    startOffset = headerInfoSize;
    do {
        maxBuckets = _getMaxBuckets(startOffset);

        p = p + maxBuckets;

        bucketOffset = ((int) targetBucket - (p - (p / maxBuckets * maxBuckets))) % maxBuckets;

        if (BF_ReadBlock(header_info.fileDesc, targetBlock, &block) < 0) {
            BF_PrintError("");
            return HT_ERROR;
        }

        _getNext(block, &nextBlock);

        if (nextBlock > 0 && targetBucket >= p) {
            targetBlock = nextBlock;
        }

        if (targetBlock > 0) {
            startOffset = 0;
        }

    } while (targetBucket >= p);

    _getBucketValue(block, startOffset + (bucketOffset * sizeof(int)), &bucketValue);

    return _getAllEntries(&header_info, bucketValue, value);
}

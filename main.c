/*
 * Copyright (c) 2015 - Charles Timko
 * License: MIT Licensed
 *
 * Author:
 *  Charles `sparticvs` Timko - sparticvs@popebp.com
 *
 * Description:
 *  Parser for the Intel MCS-86 File Format that is used by Xilinx for
 *  programming the PROM*.
 *
 *  *-That statement may be wrong, can't really tell, just wanted to read this
 *  file and make an extractor. I saw that there were many people that were
 *  also interested in parsing this data.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

typedef struct {
    uint8_t len;
    uint16_t address;
    uint8_t rec_type;
} __attribute__((__packed__)) mcs_header_t;

typedef struct {
    uint8_t *data;
    uint8_t checksum;
} __attribute__((__packed__)) mcs_input_data_rec_t;

typedef struct {
    uint8_t checksum;
} __attribute__((__packed__)) mcs_eof_rec_t;

typedef struct {
    uint16_t offset;
    uint8_t checksum;
} __attribute__((__packed__)) mcs_ext_seg_addr_rec_t;

typedef struct {
    uint16_t offset;
    uint8_t checksum;
} __attribute__((__packed__)) mcs_ext_lin_addr_rec_t;

uint8_t mcs_header_checksum(mcs_header_t *head) {
    size_t ndx;
    uint8_t checksum = 0;
    for(ndx = 0; ndx < sizeof(mcs_header_t); ndx++) {
        checksum -= ((uint8_t *)head)[ndx];
    }
    return checksum;
}

int main(int argc, char **argv) {

    size_t read = 0, ndx;
    FILE *fp = NULL;
    uint8_t checksum;

    if(2 != argc) {
        fprintf(stderr, "Must hand an MCS file as an argument\n");
        return 1;
    }

    fp = fopen(argv[1], "r");

    while(!feof(fp)) {
        mcs_header_t *mcs_header = malloc(sizeof(mcs_header_t));

        fscanf(fp, ":%2hhx%4hx%2hhx", &mcs_header->len, &mcs_header->address, &mcs_header->rec_type);

        printf("* Element is %d bytes\n", mcs_header->len);
        switch(mcs_header->rec_type) {
            case 0:
                printf("* Found an Input Data Record\n");
                mcs_input_data_rec_t *data_record = malloc(sizeof(mcs_input_data_rec_t));
                data_record->data = malloc(sizeof(uint8_t) * mcs_header->len);
                printf("* Destination Offset 0x%04x\n", mcs_header->address);
                uint8_t i = 0;
                for(i = 0; i < mcs_header->len; i++) {
                    fscanf(fp, "%2hhx", &(data_record->data[i]));
                }
                fscanf(fp, "%2hhx\n", &data_record->checksum);
                checksum = mcs_header_checksum(mcs_header);
                for(ndx = 0; ndx < mcs_header->len; ndx++) {
                    checksum -= data_record->data[ndx];
                }
                if(checksum == data_record->checksum) {
                    printf("* Checksum Valid\n");
                } else {
                    printf("! Invalid Checksum (%02x != %02x)\n", checksum, data_record->checksum);
                }
                free(data_record->data);
                free(data_record);
                break;
            case 1:
                printf("* Found an End of File Record\n");
                mcs_eof_rec_t *eof_rec = malloc(sizeof(mcs_eof_rec_t));
                fscanf(fp, "%2hhx\n", &eof_rec->checksum);
                checksum = mcs_header_checksum(mcs_header);
                if(checksum == eof_rec->checksum) {
                    printf("* Checksum Valid\n");
                } else {
                    printf("! Invalid Checksum (%02x != %02x)\n", checksum, eof_rec->checksum);
                }
                break;
            case 2:
                printf("* Found an Extended Segment Address Record\n");
                mcs_ext_seg_addr_rec_t *seg_addr = malloc(sizeof(mcs_ext_seg_addr_rec_t));
                fscanf(fp, "%4hx%2hhx\n", &seg_addr->offset, &seg_addr->checksum);
                printf("* Segment Address = 0x%08x\n", seg_addr->offset);
                checksum = mcs_header_checksum(mcs_header);
                for(ndx = 0; ndx < sizeof(uint16_t); ndx++) {
                    checksum -= ((uint8_t*)&seg_addr->offset)[ndx];
                }
                if(checksum == seg_addr->checksum) {
                    printf("* Checksum Valid\n");
                } else {
                    printf("! Invalid Checksum (%02x != %02x)\n", checksum, seg_addr->checksum);
                }
                free(seg_addr);
                break;
            case 4:
                printf("* Found an Extended Linear Address Record\n");
                mcs_ext_lin_addr_rec_t *lin_addr = malloc(sizeof(mcs_ext_lin_addr_rec_t));
                fscanf(fp, "%4hx%2hhx\n", &lin_addr->offset, &lin_addr->checksum);
                printf("* Base Address = 0x%04x\n", lin_addr->offset);
                // Compute Checksum
                checksum = mcs_header_checksum(mcs_header);
                for(ndx = 0; ndx < sizeof(uint16_t); ndx++) {
                    checksum -= ((uint8_t*)&lin_addr->offset)[ndx];
                }
                if(checksum == lin_addr->checksum) {
                    printf("* Checksum Valid\n");
                } else {
                    printf("! Invalid Checksum (%02x != %02x)\n", checksum, lin_addr->checksum);
                }
                free(lin_addr);
                break;
            default:
                printf("! Unknown Record Type Found %d\n", mcs_header->rec_type);
                break;
        }
        free(mcs_header);

    }

    return 0;
}

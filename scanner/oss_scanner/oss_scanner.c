#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <math.h>
#include <time.h>
#include "ext4_attributes.h"
#include "extended_attributes.h"

#define FD_DEVICE "/dev/sda8" /* Modify FD_DEVICE according to your device name */

#define EXT4_SUPER_MAGIC 0xEF53
#define BASE_OFFSET 1024
#define BLOCK_OFFSET(block, size) ((block) * (size))
#define IHDR(inode) ((struct ext4_xattr_ibody_header *)((void *)inode + EXT4_GOOD_OLD_INODE_SIZE + *inode.i_extra_isize))
#define IFIRST(hdr) ((struct ext4_xattr_entry *)((hdr) + 1))
#define IS_LAST_ENTRY(entry) (*(__u32 *)(entry) == 0)
#define EXT4_XATTR_NEXT(entry) ((struct ext4_xattr_entry *)((char *)(entry) + EXT4_XATTR_LEN((entry)->e_name_len)))
#define EXT4_XATTR_LEN(name_len) (((name_len) + EXT4_XATTR_ROUND + sizeof(struct ext4_xattr_entry)) & ~EXT4_XATTR_ROUND)
#define PFID(fid) (unsigned long long)(fid)->f_seq, (fid)->f_oid, (fid)->f_ver
#define EXT4_DIRENT_LUFID 0x10
#define EXT4_XATTR_PAD_BITS 2
#define EXT4_XATTR_PAD (1 << EXT4_XATTR_PAD_BITS)
#define EXT4_XATTR_ROUND (EXT4_XATTR_PAD - 1)

/*Check if filesystem structure is not initialized*/
int isAllZeros(size_t const size, void const * const ptr)
{
    int i;
    unsigned char *b = (unsigned char*) ptr;
    for (i = 0; i < size; i++)
    {
        if (b[i] != 0)
            return 0;
    }
    return 1;
}

int main(void) {
    /*time start fot scanning code*/
    clock_t start, end;
    double cpu_time_used;
    start = clock();

    // accessing the disk
    int fd;
    if ((fd = open(FD_DEVICE, O_RDONLY)) < 0) {
        perror(FD_DEVICE);
        exit(1);
    }

    // verify if ext4 filesystem
    struct ext4_super_block super;
    lseek(fd, BASE_OFFSET, SEEK_SET);
    read(fd, &super, sizeof(super));
    if (super.s_magic != EXT4_SUPER_MAGIC) {
        fprintf(stderr, "Not a Ext4 filesystem\n");
        exit(1);
    }

    //check if superblock is read correctly
    static unsigned int block_size;
    block_size = pow(2, 10 + super.s_log_block_size);
    int total_inodes = super.s_inodes_count;
    int free_inodes = super.s_free_inodes_count;
    int used_inodes = total_inodes - free_inodes;
    int inode_size = super.s_inode_size; /*512 for oss*/
    int max_inodes_per_group = super.s_inodes_per_group;
    int groups_in_flex_group = pow(2, super.s_log_groups_per_flex);
//    printf("block size: %d\n"
//           "total inodes: %d\n"
//           "free inodes: %d\n"
//           " used inodes: %d\n"
//           " inode size:%d\n"
//           "number of inodes per group:%d\n",block_size, total_inodes, free_inodes, used_inodes, inode_size, max_inodes_per_group);
//    printf("number of groups in flex group: %d\n", groups_in_flex_group);

    /*initialising output files*/
    FILE *f_edge_list;
    FILE *f_existing_nodes;
    f_edge_list = fopen("oss7_graph_test_800K.txt", "w");
    f_existing_nodes = fopen("oss7_existing_test_800K.txt", "w");

    // reading group descriptor from block 1
    struct ext4_group_desc group;
    int group_id = 0;
    int handled_inodes = 0;
    int empty_but_checked_inodes = 0;
    int exit_flag = 0;
    int group_desc_size = 32;

    int global_inode_id = 0;
    void *ibitmap_cache = malloc(block_size); // bitmap is one block

    while (exit_flag != 1) {
        //read group descriptor one at a time
        lseek(fd, BLOCK_OFFSET(1, block_size) + group_id * group_desc_size, SEEK_SET);
        read(fd, &group, group_desc_size);
        group_id += 1;
        //printf("Reading the group: Done\n");

        //read inode bitmap and inode table location offset
        struct ext4_inode cur_inode;
        int inode_table = group.bg_inode_table;
        int inode_bitmap = group.bg_inode_bitmap;
        int datablock_bitmap = group.bg_block_bitmap;
        //printf("databitmap:%d, inodebitmap:%d, inodetable:%d\n", datablock_bitmap, inode_bitmap, inode_table);

        if (inode_table == 0) {
            printf("all groups have been iterated, handle %d inodes\n", handled_inodes);
            //close group decsriptors for file
            end = clock();
            cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
            printf("total execution time : %f seconds\n", cpu_time_used);
            return 0;
            //continue;
        }

        //Read inode bitmap; if Entire bitmap has zeroes then go for next group
        memset(ibitmap_cache, 0, block_size);
        uint64_t bitmap_block_start = BLOCK_OFFSET(inode_bitmap, block_size);
        lseek(fd, bitmap_block_start, SEEK_SET);
        read(fd, ibitmap_cache, block_size);//read one block of data
        if (isAllZeros(block_size, ibitmap_cache) == 1)
            continue;

        int i, j;
        int READ_CHUNKS = 1876; //chunk of inodes = 3752(inodes per group)/2
        int CURR_CHUNK = 0;
        char *chunk_of_inodes = (char *) malloc(inode_size * READ_CHUNKS);
        uint64_t inode_chunk_start;

        for (i = global_inode_id; i < (global_inode_id + max_inodes_per_group); i++) {

            if (i >= CURR_CHUNK * READ_CHUNKS) {
                //Read inode (chunk of 1024 inodes at a time)
                inode_chunk_start = BLOCK_OFFSET(inode_table, block_size) + CURR_CHUNK * READ_CHUNKS * inode_size;
                lseek(fd, inode_chunk_start, SEEK_SET);
                read(fd, chunk_of_inodes, inode_size * READ_CHUNKS);
                CURR_CHUNK += 1;
            }
            memcpy(&cur_inode, chunk_of_inodes + (i % READ_CHUNKS) * inode_size, inode_size);

            if (isAllZeros(512, &cur_inode) == 1) {
                empty_but_checked_inodes += 1;
                continue;
            } else {
                handled_inodes += 1;
            }
            if (handled_inodes >= used_inodes) {
                printf("Handled inodes > Used inodes: exiting the while loop\n");
                exit_flag = 1;
                break;
            }

            int parent_exist = 0;

            __u64 lma_seq;
            __u32 lma_objid;
            __u32 lma_version;

            if ((cur_inode.i_extra_isize != 0) &&
                (EXT4_GOOD_OLD_INODE_SIZE + cur_inode.i_extra_isize + sizeof(struct ext4_xattr_ibody_header) +
                 EXT4_XATTR_PAD <= 512)) {

                if ( S_ISREG(cur_inode.i_mode) != 0) {

                    struct ext4_xattr_ibody_header *header;
                    header = IHDR(&cur_inode);
                    if (header->h_magic == ATTR_MAGIC) {

                        struct ext4_xattr_entry *first_entry;
                        first_entry = IFIRST(header);
                        struct ext4_xattr_entry *entry = first_entry;

                        while (!IS_LAST_ENTRY(entry)) {

                            unsigned char value[entry->e_value_size + 1];
                            memset(value, '\0', sizeof(char) * (entry->e_value_size + 1));
                            uint64_t value_offset =
                                    (i % READ_CHUNKS) * inode_size + EXT4_GOOD_OLD_INODE_SIZE +
                                    cur_inode.i_extra_isize +
                                    sizeof(struct ext4_xattr_ibody_header) + entry->e_value_offs;
                            memcpy(&value, chunk_of_inodes + value_offset, entry->e_value_size);

                            //Read FID of OST object
                            if ((char) entry->e_name[0] == 'l' && (char) entry->e_name[1] == 'm' &&
                                (char) entry->e_name[2] == 'a') {
                                struct lustre_mdt_attrs *lma = (struct lustre_mdt_attrs *) value;
                                lma_seq= lma->lma_self_fid.f_seq;
                                lma_objid = lma->lma_self_fid.f_oid;
                                lma_version = lma->lma_self_fid.f_ver;
                                //fprintf(f_existing_nodes, "%llx%x%x\n", seq, f_oid, ver);
                                //printf("LMA : %llx%x%x\n", lma_seq, lma_objid, lma_version);
                            }

                            //Read Linkea (also known as Filter FID or just FID) for the OST object
                            if ((char) entry->e_name[0] == 'f' && (char) entry->e_name[1] == 'i' &&
                                (char) entry->e_name[2] == 'd') {
                                parent_exist = 1;
                                struct filter_fid * fid = (struct filter_fid *) value;
                                //printf("LMA: [0x%llx:0x%x:0x%x]\n", seq, f_oid, ver);
                                //printf("Parent FID: [0x%llx:0x%x:0x%x]\n", fid.ff_parent.f_seq, fid.ff_parent.f_oid,fid.ff_parent.f_ver);
                                //printf("stripe size: %u bytes, stripe count: %u\n", fid.ff_layout.ol_stripe_size, fid.ff_layout.ol_stripe_count);
                                __u32 pfid_version = 0;
                                fprintf(f_edge_list, "%llx%x%x %llx%x%x\n", lma_seq, lma_objid, lma_version, fid->ff_parent.f_seq,fid->ff_parent.f_oid, pfid_version);
                                //printf("PFID: %llx%x%x %llx%x%x\n", lma_seq, lma_objid, lma_version, fid->ff_parent.f_seq,fid->ff_parent.f_oid, pfid_version);
                            }
                            entry = EXT4_XATTR_NEXT(entry);
                        }
                    }
                }

                if (parent_exist == 1) {
                    fprintf(f_existing_nodes, "%llx%x%x\n", lma_seq, lma_objid, lma_version);
                    parent_exist = 0;
                }
            } else {
                continue;
            }
        }
        //global_inode_id = global_inode_id + max_inodes_per_group;
    }

    printf("all groups have been iterated, handle %d inodes\n", handled_inodes);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("total execution time : %f seconds\n", cpu_time_used);

    exit(0);
}

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <math.h>
#include <time.h>
#include "ext4_attributes.h"
#include "extended_attributes.h"

#define FD_DEVICE "/dev/nvme1n1p1" /* Modify FD_DEVICE according to your device name*/

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
    int inode_size = super.s_inode_size; /*1024 for mds*/
    int max_inodes_per_group = super.s_inodes_per_group; /*32768 for mds*/
    int groups_in_flex_group = pow(2, super.s_log_groups_per_flex); /*16 for mds*/
    printf("block size: %d\n"
           "total inodes: %d\n"
           "free inodes: %d\n"
           " used inodes: %d\n"
           " inode size:%d\n"
           "number of inodes per group:%d\n",block_size, total_inodes, free_inodes, used_inodes, inode_size, max_inodes_per_group);
    //printf("Read superblock complete\n");

    /*initialising output files*/
    FILE *mds_partial_graph;
    FILE *mds_existing_nodes;
    mds_partial_graph = fopen("mds_partial_graph_test_800K.txt", "w");
    mds_existing_nodes = fopen("mds_existing_nodes_test_800K.txt", "w");

    int parent_exist =0;
    int child_exist=0;

    // reading group descriptor from block 1
    struct ext4_group_desc group;
    int group_id = 0; /*group number*/
    int group_desc_size = 32; /*verified*/
    int group_count = 0;

    int handled_inodes = 0;
    int empty_but_checked_inodes = 0;
    int global_inode_id = 0; /*inode number*/
    void *ibitmap_cache = malloc(block_size); // bitmap is one block
    int exit_flag = 0;

    while (exit_flag != 1) {
        //note group descriptor table is located in the beginning of the layout and ALL groups are stored there.
        //read one group from the descriptor table at a time
        lseek(fd, BLOCK_OFFSET(1, block_size) + group_id * group_desc_size, SEEK_SET);
        read(fd, &group, group_desc_size);
        group_id += 1; /*increment group number*/
        group_count++; /*increment number of group read*/
        //printf("Reading the group: Done\n");

        //check group descriptor is read correctly
        //read inode table location from group descriptor table
        struct ext4_inode cur_inode;
        int inode_table = group.bg_inode_table;
        int inode_bitmap = group.bg_inode_bitmap;
        //int datablock_bitmap = group.bg_block_bitmap;

        /*indicate group is not used yet so stop scanning*/
        if (inode_table == 0) {
            printf("all groups have been iterated, handle %d inodes\n", handled_inodes);
            //close group decsriptors for file
            end = clock();
            cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
            printf("total execution time : %f seconds\n", cpu_time_used);
            printf("Total groups scanned:%d\n\n", group_count);
            printf("Handled (non-empty) inodes: %d \n", handled_inodes);
            printf("Empty but checked inodes:%d\n",empty_but_checked_inodes);
            printf("total lma for files+dirs:%d\n", lma_file_count+lma_dir_count);
            return 0;
        }

        //Read inode bitmap for the group; if Entire bitmap has zeroes then go for next group.
        memset(ibitmap_cache, 0, block_size);
        uint64_t bitmap_block_start = BLOCK_OFFSET(inode_bitmap, block_size);
        lseek(fd, bitmap_block_start, SEEK_SET);
        read(fd, ibitmap_cache, block_size);//read one block of data
        if (isAllZeros(block_size, ibitmap_cache) == 1)
            continue;

        int i, j;
        int READ_CHUNKS = 1024;
        int CURR_CHUNK = 0;

        /*allocating space for chunk of inodes*/
        char *chunk_of_inodes = (char *) malloc(inode_size * READ_CHUNKS);
        uint64_t inode_chunk_start;

        /* scan inodes :
         * i is inode number*/
        for (i = global_inode_id; i < (global_inode_id + max_inodes_per_group); i++)
        {

            /*scan next chunk of inodes*/
            if (i >= CURR_CHUNK * READ_CHUNKS)
            {
                //scan 1024 inodes
                inode_chunk_start = BLOCK_OFFSET(inode_table, block_size) + CURR_CHUNK * READ_CHUNKS * inode_size;
                lseek(fd, inode_chunk_start, SEEK_SET);
                read(fd, chunk_of_inodes, inode_size * READ_CHUNKS);
                CURR_CHUNK += 1;
            }
            memcpy(&cur_inode, chunk_of_inodes + (i % READ_CHUNKS) * 1024, 1024);

            /*inode is empty?*/
            if (isAllZeros(1024, &cur_inode) == 1) {
                empty_but_checked_inodes += 1;
                continue;
            } else {
                handled_inodes += 1;
            }

            /*exit if all used inodes are handled*/
            if (handled_inodes >= used_inodes) {
                printf("Handled inodes > Used inodes: exiting the while loop\n");
                exit_flag = 1;
                break;
            }

            /*reading extended attributes of inodes*/
            __u64 lma_seq;
            __u32 lma_objid;
            __u32 lma_version;

            if ((cur_inode.i_extra_isize != 0) &&
                (EXT4_GOOD_OLD_INODE_SIZE + cur_inode.i_extra_isize + sizeof(struct ext4_xattr_ibody_header) +
                 EXT4_XATTR_PAD <= 1024))
            {
                //read when inode is of type directory
                if (S_ISDIR(cur_inode.i_mode) != 0)
                {
                    struct ext4_xattr_ibody_header *header;
                    header = IHDR(&cur_inode);
                    if (header->h_magic == ATTR_MAGIC)
                    {
                        struct ext4_xattr_entry *first_entry;
                        first_entry = IFIRST(header);
                        struct ext4_xattr_entry *entry = first_entry;
                        while (!IS_LAST_ENTRY(entry))
                        {
                            unsigned char value[entry->e_value_size + 1];
                            memset(value, '\0', sizeof(char) * (entry->e_value_size + 1));
                            uint64_t value_offset =
                                    (i % READ_CHUNKS) * inode_size + EXT4_GOOD_OLD_INODE_SIZE +
                                    cur_inode.i_extra_isize +
                                    sizeof(struct ext4_xattr_ibody_header) + entry->e_value_offs;
                            memcpy(&value, chunk_of_inodes + value_offset, entry->e_value_size);

                            //Read FID of directory object
                            if ((char) entry->e_name[0] == 'l' && (char) entry->e_name[1] == 'm' &&
                                (char) entry->e_name[2] == 'a') {
                                struct lustre_mdt_attrs *lma = (struct lustre_mdt_attrs *) value;
                                lma_seq = lma->lma_self_fid.f_seq;
                                lma_objid = lma->lma_self_fid.f_oid;
                                lma_version = lma->lma_self_fid.f_ver;
                                //printf("LMA : %llx%x%x\n", lma_seq, lma_objid, lma_version);
                            }

                            //Read Linkea for the directory object
                            if ((char) entry->e_name[0] == 'l' && (char) entry->e_name[1] == 'i' &&
                                (char) entry->e_name[2] == 'n' && (char) entry->e_name[3] == 'k')
                            {
                                struct link_ea_header *leh;
                                struct link_ea_entry *lee;
                                leh = (struct link_ea_header *) value;
                                lee = (struct link_ea_entry *) (leh + 1);
                                if (leh->leh_magic == LINK_EA_MAGIC) {
                                    struct lu_fid *f = (struct lu_fid *) lee->lee_parent_fid;
                                    // printf("name of the file: %s\t", lee->lee_name);
                                    struct lu_fid *pfid;
                                    pfid = (struct lu_fid *) lee->lee_parent_fid;
                                    // memcpy(pfid, &lee->lee_parent_fid, sizeof(*pfid));
                                    fid_be_to_cpu(pfid, lee->lee_parent_fid);
                                    //printf("LMA : %llx%x%x\n", lma_seq, lma_objid, lma_version);
                                    //printf("LINK: %llx%x%x %llx%x%x\n", lma_seq, lma_objid, lma_version, pfid->f_seq,pfid->f_oid, pfid->f_ver);
                                    fprintf(mds_partial_graph,"%llx%x%x %llx%x%x\n", lma_seq, lma_objid, lma_version, pfid->f_seq, pfid->f_oid, pfid->f_ver);
                                    parent_exist = 1;
                                } else {
                                    printf("ERROR on Linkea : Magic Header donot match\n");
                                }
                            }
                            entry = EXT4_XATTR_NEXT(entry);
                        }

                        // Read DIRENT for directory object
                        void *block_buf = (void *) malloc(4096);
                        int block_id = 0;
                        int reach_the_end = 0;

                        if (!reach_the_end && block_id < 12)
                        {
                            uint64_t dirent_block = BLOCK_OFFSET(cur_inode.i_block[block_id], block_size);
                            lseek(fd, cur_inode.i_block[block_id] * 4096, SEEK_SET);
                            //lseek(fd, dirent_block, SEEK_SET);
                            read(fd, block_buf, 4096);
                            //printf("inode.blockid:%d\n", cur_inode.i_block[block_id]);

                            struct ext4_dir_entry_2 *ed;
                            void *p = block_buf;
                            void *end_block = block_buf + 4096;
                            void *q;

                            while (p < end_block)
                            {
                                ed = (struct ext4_dir_entry_2 *) p;
                                if ((ed->file_type & EXT4_DIRENT_LUFID) && (ed->name_len != 0) &&
                                    (ed->name_len < 255) && (ed->rec_len != 0))
                                {
                                    q = ed->name + ed->name_len + 2;
                                    struct lu_fid *FID_in_Dirent;
                                    FID_in_Dirent = (struct lu_fid *) q;
                                    struct lu_fid *FID;
                                    FID = (struct lu_fid *) FID_in_Dirent;
                                    fid_be_to_cpu(FID, FID_in_Dirent);

                                    if (FID->f_ver != 0 && FID->f_ver != 1) {
                                        break;
                                    }
                                    else
                                    {
                                        child_exist = 1;
                                        fprintf(mds_partial_graph,"%llx%x%x %llx%x%x\n", lma_seq, lma_objid, lma_version, FID->f_seq, FID->f_oid, FID->f_ver);
                                    }

                                } else
                                {
                                    reach_the_end = 1;
                                    break;
                                }

                                p = p + ed->rec_len;
                            }
                            block_id += 1;
                        }
                        if (block_id >= 12)
                        {
                            printf("ERROR, we encounter a huge directory and have not implemented the indirect block yet\n");
                        }

                    }
                }

                //read when inode is of type file
                if (S_ISREG(cur_inode.i_mode) != 0)
                {
                    struct ext4_xattr_ibody_header *header;
                    header = IHDR(&cur_inode);
                    if (header->h_magic == ATTR_MAGIC)
                    {
                        struct ext4_xattr_entry *first_entry;
                        first_entry = IFIRST(header);
                        struct ext4_xattr_entry *entry = first_entry;

                        // read FID of file object
                        unsigned char value_lma[entry->e_value_size + 1];
                        memset(value_lma, '\0', sizeof(char) * (entry->e_value_size + 1));
                        uint64_t value_lma_offset =
                                (i % READ_CHUNKS) * inode_size + EXT4_GOOD_OLD_INODE_SIZE + cur_inode.i_extra_isize +
                                sizeof(struct ext4_xattr_ibody_header) + entry->e_value_offs;
                        memcpy(&value_lma, chunk_of_inodes + value_lma_offset, entry->e_value_size);
                        if ((char) entry->e_name[0] == 'l' && (char) entry->e_name[1] == 'm' &&
                            (char) entry->e_name[2] == 'a') {
                            lma_file_count++;
                            struct lustre_mdt_attrs *lma = (struct lustre_mdt_attrs *) value_lma;
                            lma_seq = lma->lma_self_fid.f_seq;
                            lma_objid = lma->lma_self_fid.f_oid;
                            lma_version = lma->lma_self_fid.f_ver;
                            //printf("LMA: [0x%llx:0x%x:0x%x]\n", lma->lma_self_fid.f_seq, lma->lma_self_fid.f_oid,lma->lma_self_fid.f_ver);
                            entry = EXT4_XATTR_NEXT(entry);
                        }

                        while (!IS_LAST_ENTRY(entry))
                        {
                            unsigned char value[entry->e_value_size + 1];
                            memset(value, '\0', sizeof(char) * (entry->e_value_size + 1));
                            uint64_t value_offset = (i % READ_CHUNKS) * inode_size + EXT4_GOOD_OLD_INODE_SIZE +
                                                    cur_inode.i_extra_isize + sizeof(struct ext4_xattr_ibody_header) +
                                                    entry->e_value_offs;
                            memcpy(&value, chunk_of_inodes + value_offset, entry->e_value_size);

                            //Read LINKEA for file object
                            if ((char) entry->e_name[0] == 'l' && (char) entry->e_name[1] == 'i' &&
                                (char) entry->e_name[2] == 'n' && (char) entry->e_name[3] == 'k') {
                                struct link_ea_header *leh;
                                struct link_ea_entry *lee;
                                leh = (struct link_ea_header *) value;
                                lee = (struct link_ea_entry *) (leh + 1);
                                if (leh->leh_magic == LINK_EA_MAGIC) {
                                    struct lu_fid *f = (struct lu_fid *) lee->lee_parent_fid;
                                    struct lu_fid *pfid;
                                    pfid = (struct lu_fid *) lee->lee_parent_fid;
                                    fid_be_to_cpu(pfid, lee->lee_parent_fid);
                                    //printf("parent FID: [0x%llx:0x%x:0x%x]\n", pfid->f_seq, pfid->f_oid, pfid->f_ver);
                                    //printf("LINK: %llx%x%x %llx%x%x\n", lma_seq, lma_objid, lma_version, pfid->f_seq,pfid->f_oid, pfid->f_ver);
                                    fprintf(mds_partial_graph,"%llx%x%x %llx%x%x\n", lma_seq, lma_objid, lma_version, pfid->f_seq, pfid->f_oid, pfid->f_ver);
                                    parent_exist = 1;
                                } else {
                                    printf("no match on LINKEA magic, error\n");
                                }
                            }

                            //read LOVEA for file object
                            if ((char) entry->e_name[0] == 'l' && (char) entry->e_name[1] == 'o' &&
                                (char) entry->e_name[2] == 'v')
                            {
                                struct lov_user_md *lum = (struct lov_user_md *) value;
                                int l = 0;
                                __u64 oi_ost_seq;
                                struct ost_id ost_id_temp;
                                int ost_numbers = (lum->lmm_stripe_count == 65535) ? 3 : lum->lmm_stripe_count;
                                for (l = 0; l < ost_numbers; l++) {
                                    ost_id_temp.oi.oi_id = lum->lmm_objects[l].l_ost_oi.oi.oi_id;
                                    ost_id_temp.oi.oi_seq = lum->lmm_objects[l].l_ost_oi.oi.oi_seq;

                                    ostid_set_seq_mdt0(&ost_id_temp);
                                    oi_ost_seq = fid_idif_seq(ost_id_temp.oi.oi_seq,
                                                              lum->lmm_objects[l].l_ost_idx); // FID of an OST object
                                    __u32 oi_ost_oid = lum->lmm_objects[l].l_ost_oi.oi.oi_id;
                                    __u32 oi_ost_ver = 0;
                                    //printf("LOVEA: %llx%x%x %llx%x%x\n", lma_seq, lma_objid, lma_version, oi_ost_seq,oi_ost_oid, oi_ost_ver);
                                    fprintf(mds_partial_graph, "%llx%x%x %llx%x%x\n", lma_seq, lma_objid, lma_version, oi_ost_seq,oi_ost_oid, oi_ost_ver);
                                }
                                child_exist = 1;
                            }
                            entry = EXT4_XATTR_NEXT(entry);
                        }
                    }
                }


                if (parent_exist == 1 || child_exist == 1)
                {
                    fprintf(mds_existing_nodes, "%llx%x%x\n", lma_seq, lma_objid, lma_version);
                    parent_exist = 0;
                    child_exist = 0;
                }

            }
        }
    }

    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("total execution time : %f seconds\n", cpu_time_used);

    exit(0);
}
#include <stdbool.h>
#include "/usr/include/linux/swab.h"
#include "/usr/include/linux/byteorder/little_endian.h"

#define ATTR_MAGIC 0xEA020000 /*magic number to verify extended attributes*/


/* ext4 extended attribute header */
struct ext4_xattr_header
{
    __le32 h_magic;		 /* magic number for identification */
    __le32 h_refcount;	 /* reference count */
    __le32 h_blocks;	 /* number of disk blocks used */
    __le32 h_hash;		 /* hash value of all attributes */
    __le32 h_checksum;	 /* crc32c(uuid+id+xattrblock) */
    /* id = inum if refcount=1, blknum otherwise */
    __u32 h_reserved[3]; /* zero right now */
};

/* ext4 extended attribute i_body header*/
struct ext4_xattr_ibody_header
{
    __le32 h_magic; /* magic number for identification */
};

/* ext4 extended attribute */
struct ext4_xattr_entry
{
    __u8 e_name_len;	  /* length of name */
    __u8 e_name_index;	  /* attribute name index */
    __le16 e_value_offs;  /* offset in disk block of value */
    __le32 e_value_block; /* disk block attribute is stored on (n/i) */
    __le32 e_value_size;  /* size of attribute value */
    __le32 e_hash;		  /* hash value of name and value */
    char e_name[0];		  /* attribute name */
};

struct lu_extent
{
    __u64 e_start;
    __u64 e_end;
};

/* FID */
struct lu_fid
{
    /**
       * FID sequence. Sequence is a unit of migration: all files (objects)
        * with FIDs from a given sequence are stored on the same server.
        * Lustre should support 2^64 objects, so even if each sequence
        * has only a single object we can still enumerate 2^64 objects.
        **/
    __u64 f_seq;
    /* FID number within sequence. */
    __u32 f_oid;
    /**
         * FID version, used to distinguish different versions (in the sense
         * of snapshots, etc.) of the same file system object. Not currently
         * used.
         **/
    __u32 f_ver;
};

struct ost_id
{
    union
    {
        struct
        {
            __u64 oi_id;
            __u64 oi_seq;
        } oi;
        struct lu_fid oi_fid;
    };
};

struct lustre_mdt_attrs {
    /**
     * Bitfield for supported data in this structure. From enum lma_compat.
     * lma_self_fid and lma_flags are always available.
     */
    __u32   lma_compat;
    /**
     * Per-file incompat feature list. Lustre version should support all
     * flags set in this field. The supported feature mask is available in
     * LMA_INCOMPAT_SUPP.
     */
    __u32   lma_incompat;
    /** FID of this inode */
    struct lu_fid  lma_self_fid;
};


/* LOVEA */
#define LOV_MAGIC_MAGIC 0x0BD0
#define LOV_MAGIC_V1 (0x0BD10000 | LOV_MAGIC_MAGIC)
#define LOV_MAGIC_V3 (0x0BD30000 | LOV_MAGIC_MAGIC)
#define LOV_USER_MAGIC_V1 0x0BD10BD0
#define LOV_USER_MAGIC LOV_USER_MAGIC_V1
#define LOV_MAGIC_COMP_V1 (0x0BD60000 | LOV_MAGIC_MAGIC)
#define LOV_PATTERN_NONE 0x000
#define LOV_PATTERN_RAID0 0x001
#define LOV_PATTERN_RAID1 0x002
#define LOV_PATTERN_MDT 0x100
#define LOV_PATTERN_CMOBD 0x200

struct lov_ost_data_v1
{							/* per-stripe data structure (little-endian)*/
    struct ost_id l_ost_oi; /* OST object ID */
    __u32 l_ost_gen;		/* generation of this l_ost_idx */
    __u32 l_ost_idx;		/* OST index in LOV (lov_tgt_desc->tgts) */
};

struct lov_mds_md_v1
{						   /* LOV EA mds/wire data (little-endian) */
    __u32 lmm_magic;	   /* magic number = LOV_MAGIC_V1 */
    __u32 lmm_pattern;	   /* LOV_PATTERN_RAID0, LOV_PATTERN_RAID1 */
    struct ost_id lmm_oi;  /* LOV object ID */
    //struct ost_id;
    __u32 lmm_stripe_size; /* size of stripe in bytes */
    /* lmm_stripe_count used to be __u32 */
    __u16 lmm_stripe_count;				   /* num stripes in use for this object */
    __u16 lmm_layout_gen;				   /* layout generation number */
    struct lov_ost_data_v1 lmm_objects[0]; /* per-stripe data */
};

struct lov_comp_md_entry_v1
{
    __u32 lcme_id;				  /* unique identifier of component */
    __u32 lcme_flags;			  /* LCME_FL_XXX */
    struct lu_extent lcme_extent; /* file extent for component */
    __u32 lcme_offset;			  /* offset of component blob in layout */
    __u32 lcme_size;			  /* size of component blob data */
    __u64 lcme_padding[2];
};

struct lov_comp_md_v1
{
    __u32 lcm_magic; /* LOV_USER_MAGIC_COMP_V1 */
    __u32 lcm_size;	 /* overall size including this struct */
    __u32 lcm_layout_gen;
    __u16 lcm_flags;
    __u16 lcm_entry_count;
    /* lcm_mirror_count stores the number of actual mirrors minus 1,
     * so that non-flr files will have value 0 meaning 1 mirror. */
    __u16 lcm_mirror_count;
    __u16 lcm_padding1[3];
    __u64 lcm_padding2;
    struct lov_comp_md_entry_v1 lcm_entries[0];
} __attribute__((packed));

/*from inlcude/uapi/linux/lustre/lustre_user.h*/

#define lov_user_ost_data lov_user_ost_data_v1
struct lov_user_ost_data_v1
{							/* per-stripe data structure */
    struct ost_id l_ost_oi; /* OST object ID */
    __u32 l_ost_gen;		/* generation of this OST index */
    __u32 l_ost_idx;		/* OST index in LOV */
} __attribute__((packed));

#define lov_user_md lov_user_md_v1
struct lov_user_md_v1
{							/* LOV EA user data (host-endian) */
    __u32 lmm_magic;		/* magic number = LOV_USER_MAGIC_V1 */
    __u32 lmm_pattern;		/* LOV_PATTERN_RAID0, LOV_PATTERN_RAID1 */
    struct ost_id lmm_oi;	/* MDT parent inode id/seq (id/0 for 1.x) */
    __u32 lmm_stripe_size;	/* size of stripe in bytes */
    __u16 lmm_stripe_count; /* num stripes in use for this object */
    union
    {
        __u16 lmm_stripe_offset; /* starting stripe offset in
					   * lmm_objects, use when writing */
        __u16 lmm_layout_gen;	 /* layout generation number
					   * used when reading */
    };
    struct lov_user_ost_data_v1 lmm_objects[0]; /* per-stripe data */
} __attribute__((packed, __may_alias__));


/* DIRENT */
#define	EXT4_NDIR_BLOCKS		12
#define	EXT4_IND_BLOCK			EXT4_NDIR_BLOCKS
#define	EXT4_DIND_BLOCK			(EXT4_IND_BLOCK + 1)
#define	EXT4_TIND_BLOCK			(EXT4_DIND_BLOCK + 1)
#define	EXT4_N_BLOCKS			(EXT4_TIND_BLOCK + 1)

struct ext4_inode {
    __le16	i_mode;		/* File mode */
    __le16	i_uid;		/* Low 16 bits of Owner Uid */
    __le32	i_size_lo;	/* Size in bytes */
    __le32	i_atime;	/* Access time */
    __le32	i_ctime;	/* Inode Change time */
    __le32	i_mtime;	/* Modification time */
    __le32	i_dtime;	/* Deletion Time */
    __le16	i_gid;		/* Low 16 bits of Group Id */
    __le16	i_links_count;	/* Links count */
    __le32	i_blocks_lo;	/* Blocks count */
    __le32	i_flags;	/* File flags */
    union {
        struct {
            __le32  l_i_version;
        } linux1;
        struct {
            __u32  h_i_translator;
        } hurd1;
        struct {
            __u32  m_i_reserved1;
        } masix1;
    } osd1;				/* OS dependent 1 */
    __le32	i_block[EXT4_N_BLOCKS];/* Pointers to blocks */
    __le32	i_generation;	/* File version (for NFS) */
    __le32	i_file_acl_lo;	/* File ACL */
    __le32	i_size_high;
    __le32	i_obso_faddr;	/* Obsoleted fragment address */
    union {
        struct {
            __le16	l_i_blocks_high; /* were l_i_reserved1 */
            __le16	l_i_file_acl_high;
            __le16	l_i_uid_high;	/* these 2 fields */
            __le16	l_i_gid_high;	/* were reserved2[0] */
            __le16	l_i_checksum_lo;/* crc32c(uuid+inum+inode) LE */
            __le16	l_i_reserved;
        } linux2;
        struct {
            __le16	h_i_reserved1;	/* Obsoleted fragment number/size which are removed in ext4 */
            __u16	h_i_mode_high;
            __u16	h_i_uid_high;
            __u16	h_i_gid_high;
            __u32	h_i_author;
        } hurd2;
        struct {
            __le16	h_i_reserved1;	/* Obsoleted fragment number/size which are removed in ext4 */
            __le16	m_i_file_acl_high;
            __u32	m_i_reserved2[2];
        } masix2;
    } osd2;				/* OS dependent 2 */
    __le16	i_extra_isize;
    __le16	i_checksum_hi;	/* crc32c(uuid+inum+inode) BE */
    __le32  i_ctime_extra;  /* extra Change time      (nsec << 2 | epoch) */
    __le32  i_mtime_extra;  /* extra Modification time(nsec << 2 | epoch) */
    __le32  i_atime_extra;  /* extra Access time      (nsec << 2 | epoch) */
    __le32  i_crtime;       /* File Creation time */
    __le32  i_crtime_extra; /* extra FileCreationtime (nsec << 2 | epoch) */
    __le32  i_version_hi;	/* high 32 bits for 64-bit version */
};

#define EXT4_NAME_LEN 255
struct ext4_dir_entry {
    __le32	inode;			/* Inode number */
    __le16	rec_len;		/* Directory entry length */
    __le16	name_len;		/* Name length */
    char	name[];	/* File name */
};

struct ext4_dir_entry_2 {
    __le32	inode;			/* Inode number */
    __le16	rec_len;		/* Directory entry length */
    __u8	name_len;		/* Name length */
    __u8	file_type;
    //char	name[EXT4_NAME_LEN];	/* File name */
    char	name[EXT4_NAME_LEN];	/* File name */
};

# define EXTRA_LENGTH 255
struct extra {
    __u8	extra_len;
    char	extra_name[EXTRA_LENGTH];

};


/* LINKEA */
#define LINK_EA_MAGIC 0x11EAF1DFUL
struct link_ea_header {
    __u32 leh_magic;
    __u32 leh_reccount;
    __u64 leh_len;	/* total size */
    __u32 leh_overflow_time;
    __u32 leh_padding;
};

struct link_ea_entry {
    /** __u16 stored big-endian, unaligned */
    unsigned char      lee_reclen[2];
    unsigned char      lee_parent_fid[sizeof(struct lu_fid)];
    char               lee_name[0];
} __attribute__((packed));

/** returns fid object sequence */
static inline __u64 fid_seq(const struct lu_fid *fid)
{
    return fid->f_seq;
}

/** returns fid object id */
static inline __u32 fid_oid(const struct lu_fid *fid)
{
    return fid->f_oid;
}

/** returns fid object version */
static inline __u32 fid_ver(const struct lu_fid *fid)
{
    return fid->f_ver;
}

static  inline void fid_be_to_cpu(struct lu_fid *dst, const struct lu_fid *src)
{

    dst->f_seq = __be64_to_cpu(fid_seq(src));
    dst->f_oid = __be32_to_cpu(fid_oid(src));
    dst->f_ver = __be32_to_cpu(fid_ver(src));

}

static inline void fid_le_to_cpu(struct lu_fid *dst, const struct lu_fid *src)
{
    printf("I give segmentation fault :(\n");
    dst->f_seq = __le64_to_cpu(fid_seq(src));
    dst->f_oid = __le32_to_cpu(fid_oid(src));
    dst->f_ver = __le32_to_cpu(fid_ver(src));
}






/* Extended Atrributes on OST */

struct lustre_ost_attrs {
    /* Use lustre_mdt_attrs directly for now, need a common header
     * structure if want to change lustre_mdt_attrs in future. */
    struct lustre_mdt_attrs loa_lma;

    /* Below five elements are for OST-object's PFID EA, the
     * lma_parent_fid::f_ver is composed of the stripe_count (high 16 bits)
     * and the stripe_index (low 16 bits), the size should not exceed
     * 5 * sizeof(__u64)) to be accessable by old Lustre. If the flag
     * LMAC_STRIPE_INFO is set, then loa_parent_fid and loa_stripe_size
     * are valid; if the flag LMAC_COMP_INFO is set, then the next three
     * loa_comp_* elements are valid. */
    struct lu_fid	loa_parent_fid;
    __u32		loa_stripe_size;
    __u32		loa_comp_id;
    __u64		loa_comp_start;
    __u64		loa_comp_end;
};

struct ost_layout {
    __u32	ol_stripe_size;
    __u32	ol_stripe_count;
    __u64	ol_comp_start;
    __u64	ol_comp_end;
    __u32	ol_comp_id;
} __attribute__((packed));


/* LINKEA (a.k.a filter fid) on OST */
struct filter_fid {
    struct lu_fid		ff_parent;	/* stripe_idx in f_ver */
    struct ost_layout	ff_layout;
    __u32			ff_layout_version;
    __u32			ff_range; /* range of layout version that
					   * write are allowed */
} __attribute__((packed));


struct filter_fid_18_23 {
    struct lu_fid		ff_parent;	/* stripe_idx in f_ver */
    __u64			ff_objid;
    __u64			ff_seq;
};

//read oss id from lovea and covert it to ostid
#define FID_SEQ_OST_MDT0 0

static inline void ostid_set_seq(struct ost_id *oi, __u64 seq){
    oi->oi_fid.f_seq = seq;
}

static inline void ostid_set_seq_mdt0(struct ost_id *oi){
    ostid_set_seq(oi, FID_SEQ_OST_MDT0);
}

#define FID_SEQ_IDIF 0x100000000ULL
// convert an OST objid into an IDIF FID SEQ number
static inline __u64 fid_idif_seq(__u64 id, __u32 ost_idx)
{
    return FID_SEQ_IDIF | (ost_idx << 16) | ((id >> 32) & 0xffff);
}



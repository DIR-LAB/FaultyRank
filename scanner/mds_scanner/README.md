# Metadata Extraction on MDS

## Quick Overview
* `mds_scanner.c`: scans the MDS node and builds a partial graph 
* `ext4_attributes.h`: header file for ext4 data structures
* `extended_attributes.h`: header file for extended attributes

## Personalize Our Code for Your Environment

- Configure **FD_DEVICE** according to your device name in `mds_scanner.c`.

``` 
$ vi mds_scanner.c

/* Modify FD_DEVICE according to your device name */
# define FD_DEVICE "dev/your_mds_device_name" 
```

## Build and Run

```
$ make
$ ./mds_scanner
```
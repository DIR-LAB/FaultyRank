# Metadata Extraction on OSS

## Quick Overview
* `oss_scanner.c`: scans the OSS node and builds a partial graph
* `ext4_attributes.h`: header file for ext4 data structures
* `extended_attributes.h`: header file for extended attributes

## Personalize Our Code for Your Environment

- Configure **FD_DEVICE** according to your device name in `oss_scanner.c`.

``` 
$ vi oss_scanner.c

/* Modify FD_DEVICE according to your device name */
# define FD_DEVICE "dev/your_oss_device_name " 
```

## Build and Run

```
$ make
$ ./oss_scanner
```
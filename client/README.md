# Filesystem Aging

# Quick Overview

* `partition_dataset.py`: the LANL dataset is divided into partitions of equal size, each corresponding to a client.
* `fsaging.py`: uses the partitioned logs to age the Lustre filesystem.


# Personalize Our Code for Your Environment

- Configure the number of client nodes according to your Lustre Cluster in `partition_dataset.py`

``` diff
$ vi partition_dataset.py

# change number of clients
NUMBER_OF_CLIENTS = Number_of_clients
```

- Configure the Lustre mount point in `fsaging.py`

``` diff
$ vi fsaging.py

# Lustre filesystem mount point
lustre_mount_point = "path_to_mountpoint"
```

- Configure the number of OST nodes according to your Lustre Cluster in `fsaging.py`

``` diff
$ vi fsaging.py

# Configure according to the number of OSTs in Lustre Cluster
SCALE_SIZE = number_of_OSTs
```









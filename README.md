# FaultyRank

FaultyRank is a graph based Parallel Filesystem Checker. We model important Parallel Filesystem metadata into a graph and then generalize the logic of cross-checking and repairing into graph analytic tasks.
We implement a prototype of FaultyRank on Lustre and compare it with Lustre’s default file system checker LFSCK.

- You can learn more about FaultyRank in our [IPDPS 2023 paper](todo: url of paper).
- If you use this software please cite us:

```
@inproceedings{faultyrank2023,
  author={Kamat, Saisha and Islam, Abdullah Al Raqibul and Zheng, Mai  and Dai, Dong},
  title={FaultyRank: A Graph-based Parallel File System Checker},
  booktitle={2023 37th IEEE International Parallel & Distributed Processing Symposium (IPDPS)},
  year={2023},
}
```

# Table of Contents
[Directory Structure](https://github.com/SaishaKamat/Git_Personal/tree/master/FaultyRank_Personal#directory-structure)  
[Getting Started](https://github.com/SaishaKamat/Git_Personal/tree/master/FaultyRank_Personal#getting-started)  
[Design and Implementation](https://github.com/SaishaKamat/Git_Personal/tree/master/FaultyRank_Personal#design-and-implementation)  
[Test Experiment on a Pre-built Graph](https://github.com/SaishaKamat/Git_Personal/tree/master/FaultyRank_Personal#test-experiment-on-a-pre-built-graph)  
[Contact](https://github.com/SaishaKamat/Git_Personal/tree/master/FaultyRank_Personal#contact)

# Directory Structure

```
data/: test data
client/: filesystem aging scripts (run from client nodes)
scanner/：metadata extractor (run from mds and oss nodes)
aggregator/: aggregate partial graphs into a unified graph (run it from mds nodes)
core/: implementation of FaultyRank algorithm
```

# Getting Started

The filesystem aging scripts has been implemented in Python 3.6.8. The rest of the codes is implemented in C/C++ and tested on CentOS Linux 7.9.2009 with g++ (GCC)  4.8.5.

## Testbed Setup
- We tested FaultyRank on a local Lustre cluster with 1 MDS/MGS server and 8 OSS servers.
  - The MDS/MGS server uses Intel(R) Xeon(R) Bronze 3204 CPU with 128GB DRAM and 256GB local SSD.
  - The eight OSS servers use Intel(R) Xeon(R) CPU E5-2630 CPU with 32GB DRAM and 1TB hard disk (partially partitioned for Lustre).
- Our installed Lustre instance contains 2.4TB of storage space.

## Software Requirements
- Python 3.6.8
- g++ (GCC) 4.8.5
- CentOS Linux 7.9.2009
- Lustre 2.12.8 (with the latest LFSCK implementation)

# Design and Implementation
FaultyRank have four major steps. A detailed description of each step is provided in the following sections.

## Filesystem Aging
We create a realistic Lustre instance by aging it. We are using Archive and NSF Metadata dataset released by USRC (Ultrascale Systems Research Center) from LANL National Lab.
The LANL dataset have around 2PB of files and contains  file system walk of LANL's HPC systems. It contains detailed information like file sizes, creation time, modification time, UID/GID, anonymized file path, etc.

We use the actual file paths to re-create the same directory structures in our local testbed. We also shrink the sizes of files in the 2PB file system without affecting the representativeness of generated Layout metadata. To do this, we set the stripe_size of our Lustre directories to be extremely small (i.e., 64KB) and stripe_count to be −1.

- Download an Archive and NSF Metadata dataset at [LANL(USRC)](https://usrc.lanl.gov/ds-storage-data.php) in `FaultyRank/client` directory.

- Parse the LANL dataset into X (# of clients) files.

```
$ cd FaultyRank/client
$ python partition_dataset.py
```

- Copy all these partitioned log files into the lustre client nodes. Run the filesystem aging scripts from every lustre client nodes:

```
$ cd FaultyRank/client
$ python fsaging.py -i [path_to_data/partitioned_data.txt]
```

- Please follow [FileSystemAgingCustomized](https://github.com/DIR-LAB/FaultyRank/blob/main/client/README.md) for customization.

## Metadata Extraction
Lustre metadata is stored in two places:
1) The metadata like FID, LINKEA and LOVEA are stored in the Extended Attributes (EA) of the local inodes.
2) The DIRENT metadata between the directory and its sub-directories or files are stored as the content of the directory.

To extract Lustre metadata, we scan the extended attributes of inodes and the contents of directories of all MDS and OSS servers in our network. A partial graph is created on each server.
This graph contains a list of edges, where each edge has a source vertex and destination vertex, each representing a Lustre directory, file, or stripe object. All the vertices have a unique global FID.

- Run the scripts in `FaultyRank/scanner/mds_scanner` on each MDS nodes to extract metadata from MDS node and create a partial graph.

```
$ cd FaultyRank/scanner/mds_scanner
$ make
```

- Run the scripts in `FaultyRank/scanner/oss_scanner` on each OSS nodes to extract metadata from OSS nodes and create a partial graph.

```
$ cd FaultyRank/scanner/oss_scanner
$ make
```

- Please follow [MetadataExtractionMDSCustomized](https://github.com/DIR-LAB/FaultyRank/tree/main/scanner/mds_scanner#readme) and [MetadataExtractionOSSCustomized](https://github.com/DIR-LAB/FaultyRank/blob/main/scanner/oss_scanner/README.md) for customization.

## Unified Graph Creation
All the partial graphs created in the previous step are combined in one global graph on the main MDS server. The graph vertex IDs, which are 128-bit Lustre non-continuous FIDs are mapped to vertex GIDs from 0 to MAX_VERTEX_NUM-1.

Move all the partial graphs created in the above step to `FaultyRank/aggregator`. Run the aggregator scripts to combine the partial graphs into one unified graph.

```
$ cd FaultyRank/aggregator
$ make
```

- Next we note the number of Vertices and Edges in the unified graph created.

``` diff
$ cd FaultyRank/aggregator

# Read number of Vertices
$ tail -1 final_graph.txt

# Read number of Edges
$ wc -l final_graph.txt
```

## Run FaultyRank Algorithm
Run FaultyRank algorithm on the global graph created in the previous step.

- Add the number of Vertices and Edges from the previous step and run FaultyRank algorithm.

```
$ cd FaultyRank/core
$ ./faultyrank_core -N (# of Vertices) -E (# of Edges) -f FaultyRank/aggregator/final_graph.txt
```

# Fault Injection


# Test Experiment on a Pre-built Graph
We have included a simple dataset that features a pre-existing graph with an incorporated inconsistency. This test dataset offers an easy means of gaining insight into the working of FaultyRank.

We can view the dataset in `FaultyRank/data`. The dataset graph has 4 Vertices and 6 Edges. This can be confirmed by running the following commands.

``` diff
$ cd FaultyRank/data

# Read number of Vertices
$ tail -1 test_graph.txt

# Read number of Edges
$ wc -l test_graph.txt
```

Run FaultyRank on the test dataset.

``` diff
$ cd FaultyRank/core
$ ./faultyrank_core -N 4 -E 6 -f FaultyRank/data/test_graph.txt
```

# Contact
`@todo`








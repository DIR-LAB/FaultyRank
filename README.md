# FaultyRank

FaultyRank is a graph-based Parallel Filesystem Checker. We model important Parallel Filesystem metadata into a graph and then generalize the logic of cross-checking and repairing into graph analytic tasks.
We implement a prototype of FaultyRank on Lustre and compare it with Lustre’s default file system checker, LFSCK.

- Learn about FaultyRank in our [IPDPS 2023 paper](https://daidong.github.io/files/faultyrank_ipdps23.pdf).
- If you use this software, please cite us:

```
@inproceedings{faultyrank2023,
  author={Kamat, Saisha and Islam, Abdullah Al Raqibul and Zheng, Mai and Dai, Dong},
  title={FaultyRank: A Graph-based Parallel File System Checker},
  booktitle={37th IEEE International Parallel & Distributed Processing Symposium (IPDPS)},
  year={2023},
  pages={200-210},
  doi={10.1109/IPDPS54959.2023.00029}
}
```

## Table of Contents
[Directory Structure](https://github.com/DIR-LAB/FaultyRank#directory-structure)  
[Getting Started](https://github.com/DIR-LAB/FaultyRank#getting-started)  
[Design and Implementation](https://github.com/DIR-LAB/FaultyRank#design-and-implementation)  
[Test Experiment on a Pre-built Graph](https://github.com/DIR-LAB/FaultyRank#test-experiment-on-a-pre-built-graph)  
[Contact](https://github.com/DIR-LAB/FaultyRank#contact)

## Directory Structure
```
aggregator/: aggregate partial graphs into a unified graph (run it from mds nodes)
client/: filesystem aging scripts (run from client nodes)
core/: implementation of FaultyRank algorithm
data/: test data
scanner/：metadata extractor (run from MDS and OSS nodes)
```

## Getting Started

The filesystem aging scripts has been implemented in Python 3.6.8. The rest of the codes are implemented in C/C++ and tested on CentOS Linux 7.9.2009 with g++ (GCC) 4.8.5.

### Testbed Setup
- We tested FaultyRank on a local Lustre cluster with 1 MDS/MGS server and 8 OSS servers.
  - The MDS/MGS server uses Intel(R) Xeon(R) Bronze 3204 CPU with 128GB DRAM and 256GB local SSD.
  - The eight OSS servers use Intel(R) Xeon(R) CPU E5-2630 CPU with 32GB DRAM and 1TB hard disk (partially partitioned for Lustre).
- Our installed Lustre instance contains 2.4TB of storage space.

### Software Requirements
- Python 3.6.8
- g++ (GCC) 4.8.5
- CentOS Linux 7.9.2009
- Lustre 2.12.8 (with the latest LFSCK implementation)

## Design and Implementation
FaultyRank has four major steps. A detailed description of each step is provided in the following sections.

### Step 1: Filesystem Aging
We create a realistic Lustre instance by aging it. We used the `Archive and NSF Metadata` dataset released by USRC (Ultrascale Systems Research Center) from LANL National Lab. The LANL dataset has around 2PB of files and contains the file system walk of LANL's HPC systems. In addition, it contains detailed information like file sizes, creation time, modification time, UID/GID, anonymized file path, etc.

We use the actual file paths to re-create the same directory structures in our local testbed. We also shrink the sizes of files in the 2PB file system without affecting the representativeness of generated Layout metadata. To do this, we set the `stripe_size` of our Lustre directories to be extremely small (i.e., `64KB`) and `stripe_count` to be `−1`. More details can be found in the paper (see section `V.A. Evaluation Testbed and Dataset`).

- Download the `Archive and NSF Metadata` dataset at [LANL(USRC)](https://usrc.lanl.gov/ds-storage-data.php) in the `FaultyRank/client` directory.
- Parse the LANL dataset into `X` (e.g., `# of clients`) files.

```
$ cd FaultyRank/client
$ python partition_dataset.py -i [path_to_input_data/data.txt] -o [path_to_output_data/] -x [npartitions]
```

- Copy all these partitioned data files into the Lustre client nodes. Then, run the filesystem aging scripts from every Lustre client nodes:

```
$ cd FaultyRank/client
$ python fsaging.py -i [path_to_data/partitioned_data.txt]
```

- Please follow [FileSystemAgingCustomized](https://github.com/DIR-LAB/FaultyRank/tree/main/client) for customization.

### Step 2: Metadata Extraction
Lustre metadata is stored in two places:
1) The metadata like `FID`, `LINKEA`, and `LOVEA` are stored in the `Extended Attributes (EA)` of the local inodes.
2) The `DIRENT` metadata between the directory and its sub-directories or files are stored as the content of the directory file.

To extract Lustre metadata, we scan the extended attributes of inodes and the contents of directories of all `MDS` and `OSS` servers in our testbed. A partial graph is created on each server. This graph contains a list of edges, where each edge has a source vertex and destination vertex, each representing a Lustre directory, file, or stripe object. All the vertices have a unique global `FID`.

- Build and run the code from `FaultyRank/scanner/mds_scanner` on each MDS node to extract metadata and create a partial graph.

```
$ cd FaultyRank/scanner/mds_scanner
$ make
$ ./mds_scanner
```

- Build and run the code from `FaultyRank/scanner/oss_scanner` on each OSS node to extract metadata and create a partial graph.

```
$ cd FaultyRank/scanner/oss_scanner
$ make
$ ./oss_scanner
```

- Please follow [MetadataExtractionMDSCustomized](https://github.com/DIR-LAB/FaultyRank/tree/main/scanner/mds_scanner) and [MetadataExtractionOSSCustomized](https://github.com/DIR-LAB/FaultyRank/tree/main/scanner/oss_scanner) for customization.

### Step 3: Create Unified Graph
All the partial graphs created in the previous step are combined in one global graph on the main `MDS` server. The graph vertex IDs, which are `128-bit` Lustre non-continuous FIDs are mapped to vertex GIDs from `0` to `(MAX_VERTEX_NUM - 1)`.

Move all the partial graphs created in the previous step to `FaultyRank/aggregator`. Then, run the aggregator scripts to combine the partial graphs into one unified graph.

```
$ cd FaultyRank/aggregator
$ make
$ ./graph_build_flathm > final_graph.txt
```

Next, we record the number of vertices and edges of the unified graph. These records will be required in future steps.

```
$ cd FaultyRank/aggregator

# Read number of Vertices
$ tail -1 final_graph.txt

# Read number of Edges
$ wc -l final_graph.txt
```

### Step 4: Run FaultyRank Algorithm
Run the FaultyRank algorithm on the unified graph created in the previous step.

- Run the FaultyRank algorithm (use the number of vertices and edges from the previous step).

```
$ cd FaultyRank/core
$ ./faultyrank -N (# of Vertices) -E (# of Edges) -f /path_to_final_graph/final_graph.txt
```

- If there was a `final_unfilled.txt` file created in the previous step, then run:

```
$ cd FaultyRank/core
$ ./faultyrank -N (# of Vertices) -E (# of Edges) -f /path_to_final_graph/final_graph.txt -u /path_to_final_unfilled/final_unfilled.txt
```

## Test Experiment on a Pre-built Graph
We have included a simple dataset that features a pre-existing graph with an incorporated inconsistency. This test dataset offers an easy means of gaining insight into the working of FaultyRank.

The dataset can be found in `FaultyRank/data`. The dataset graph has `4` Vertices and `6` Edges. This can be confirmed by running the following commands.

```
$ cd FaultyRank/data

# Read number of Vertices
$ tail -1 test_graph.txt

# Read number of Edges
$ wc -l test_graph.txt
```

Run FaultyRank on the test dataset.

```
$ cd FaultyRank/core
$ ./faultyrank -N 4 -E 6 -f FaultyRank/data/test_graph.txt
```

## Contribution

We highly appreciate your contribution to this project! Here are some of the ways you can contribute:

- Bug fixes, whether for performance or correctness of the existing implementation.
- Improvement of the documentation.
- Add `scanner` code for parallel file systems other than `Lustre`; we are open to adding support for more parallel file systems.

For code contributions, please focus on code simplicity and readability. If you open a pull request, we will do a quick sanity check and respond as soon as possible.

## Contact
[Saisha Kamat (UNC Charlotte)](https://github.com/SaishaKamat)  
[Abdullah Al Raqibul Islam (UNC Charlotte)](https://biqar.github.io)  
[Dr. Dong Dai (UNC Charlotte)](https://daidong.github.io/)  
[Dr. Mai Zheng(Iowa State University)](https://www.ece.iastate.edu/~mai/lab/dsl.html)

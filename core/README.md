# Run FaultyRank Algorithm

The core idea of FaultyRank, such as the graph-based metadata abstraction and the iterative credibility calculation, is generic to be applied to any parallel file systems. To implement it on a different PFS, the iterative calculation should remain the same once the metadata graph is built. But, the scanning and graph-building phases (described in [scanner](https://github.com/DIR-LAB/FaultyRank/tree/main/scanner) directory) will be different and depend on the file system implementation.

## Build and Run
```
$ make
$ ./faultyrank -N (# of Vertices) -E (# of Edges) -f /path_to_final_graph/final_graph.txt -u /path_to_final_unfilled/final_unfilled.txt
```

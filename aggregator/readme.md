# Graph building code

This tool help us to remap the inode-ids to vertex-ids.

| Implementation    | Description                                                                                                                                                   |
|-------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------|
| flat-hashmap      | Used parallel-hashmap from this [GitHub repo](https://github.com/greg7mdp/parallel-hashmap/tree/master/parallel_hashmap). Used RHH-hash as the hash function. |
| std-unordered_map | Used `std::unordered_map` with RHH-hash as the hash function.                                                                                                 |

# Build

* Build command
```
> make clean && make
```

* Run command
```
> ./graph_build_stdum 
> ./graph_build_flathm
```

# Todo
* Take input files as the parameter to this program.

# Reference
* [Hashmaps Benchmarks - Overview (Finding the Fastest, Memory Efficient Hashmap)](https://martin.ankerl.com/2019/04/01/hashmap-benchmarks-01-overview/)
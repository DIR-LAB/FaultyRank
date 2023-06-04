# Create Unified Graph

This tool helps to remap the inode-ids to vertex-ids. We provided two implementations using different `Map` implementations. We used the `flat-hashmap` in our evaluation.

| Implementation    | Description                                                                                                                                                   |
|-------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------|
| flat-hashmap      | Used parallel-hashmap from this [GitHub repo](https://github.com/greg7mdp/parallel-hashmap/tree/master/parallel_hashmap). Used RHH-hash as the hash function. |
| std-unordered_map | Used `std::unordered_map` with RHH-hash as the hash function.                                                                                                 |

## Build and Run

* Build command
```
> make clean && make
```

* Run command
```
> ./graph_build_stdum 
> ./graph_build_flathm
```

## Reference
* [Hashmaps Benchmarks - Overview (Finding the Fastest, Memory Efficient Hashmap)](https://martin.ankerl.com/2019/04/01/hashmap-benchmarks-01-overview/)
// Copyright (c) 2022, The DIR-LAB of the University of North Carolina at Charlotte
// See LICENSE.txt for license details

#ifndef GRAPH_H_
#define GRAPH_H_

#include <chrono>
#include <algorithm>
#include <cinttypes>
#include <cstddef>
#include <iostream>
#include <fstream>
#include <type_traits>
#include <assert.h>
#include <cstring>
#include <vector>

using namespace std;

#define debug 0

typedef int64_t NodeID;

/*
Class:  Graph
Author: Raqib Islam

Simple container for graph in Adjacency List format
 - Stores the out-edges of the graph
 - Intended to be constructed either by an input file or another Graph instance
 - When building from another Graph instance, it will build a reverse graph
*/


/// Used to hold destination node-id & marker whether the edge is an undirected one
struct EdgeItem {
  NodeID v;           // destination of this edge in the graph
  bool paired_flag;   // mark whether the edge is paired/undirected

  EdgeItem() {}

  EdgeItem(NodeID v) : v(v), paired_flag(false) {}

  EdgeItem(NodeID v, bool paired_flag) : v(v), paired_flag(paired_flag) {}

  bool operator<(const EdgeItem &rhs) const {
    return v < rhs.v;
  }

  // doesn't check paired-flag_s, needed to remove duplicate edges
  bool operator==(const EdgeItem &rhs) const {
    return v == rhs.v;
  }

  // doesn't check paired-flag_s, needed to remove self edges
  bool operator==(const NodeID &rhs) const {
    return v == rhs;
  }

  operator NodeID() {
    return v;
  }
};

std::ostream &operator<<(std::ostream &os, const EdgeItem &e) {
  os << e.v << " " << e.paired_flag;
  return os;
}

std::istream &operator>>(std::istream &is, EdgeItem &e) {
  is >> e.v >> e.paired_flag;
  return is;
}

/// structure for the vertices
struct VertexItem {
  uint32_t out_degree_paired = 0;
  bool unfilled_property_flag = false;
  vector <EdgeItem> neighbors;
};

class Graph {
public:
  vector <VertexItem> graph_;

  /// Graph Constructor: Does not insert any edges
  Graph(uint64_t n_vertices, uint64_t n_edges) {
    num_nodes_ = n_vertices;
    num_edges_ = n_edges;
  }

  /// Graph Constructor: Build inverse graph for @reverse
  Graph(const Graph &reverse) {
    num_nodes_ = reverse.num_nodes_;
    num_edges_ = reverse.num_edges_;

    graph_.resize(reverse.num_nodes_);
    build_graph(reverse);
  }

  /// Graph Constructor: Build graph from @filename
  Graph(std::string filename, uint64_t n_vertices, uint64_t n_edges) {
    num_nodes_ = n_vertices;
    num_edges_ = n_edges;

    graph_.resize(n_vertices);

    build_graph(filename);
    sort_neighbors();
    mark_paired_edges();
  }

  /// Graph Destructor: Will deallocate all the inner adjacency lists first and then deallocate the @graph_
  ~Graph() {
    ReleaseResources();
  }

  /// Mark the unfilled-property nodes from @filename
  void mark_unfilled_property_vertices(const std::string &filename) {
    // open the input file
    std::ifstream file(filename);
    if (!file.is_open()) {
      std::cout << "Couldn't open file " << filename << std::endl;
      std::exit(-2);
    }

    NodeID u;
    while (file >> u) {
      graph_[u].unfilled_property_flag = true;
    }
  }

  /// Sugar for number of nodes
  int64_t num_nodes() const {
    return num_nodes_;
  }

  /// Sugar for number of edges
  int64_t num_edges() const {
    return num_edges_;
  }

  /// Sugar for out-degree of node-id @v
  int64_t out_degree(NodeID v) const {
    return graph_[v].neighbors.size();
  }

  /// Sugar for out-degree-paired of node-id @v
  int64_t out_degree_paired(NodeID v) const {
    return graph_[v].out_degree_paired;
  }

  /// Sugar for out-neighbors
  // todo: will implement it later if needed, currently putting the @graph_ in the public domain
//  Neighborhood out_neigh(NodeID_ n, OffsetT start_offset = 0) const {
//    return Neighborhood(graph_[n], start_offset);
//  }

  /// Print the graph properties
  void PrintStats() const {
    std::cout << "Graph has " << num_nodes_ << " nodes and " << num_edges_ << " directed edges for avg. degree: ";
    std::cout << num_edges_ / num_nodes_ << std::endl;
  }

  /// Print the whole graph
  void PrintTopology() const {
    for (NodeID i = 0; i < num_nodes_; i++) {
      std::cout << i << ": ";
      for (EdgeItem j: graph_[i].neighbors) {
        std::cout << "(" << j.v << ", " << j.paired_flag << ") ";
      }
      std::cout << std::endl;
    }
  }

  /// Print the neighbors of node-id @src
  void PrintTopology(NodeID src) const {
    std::cout << src << ": ";
    for (EdgeItem j: graph_[src].neighbors) {
      std::cout << "(" << j.v << ", " << j.paired_flag << ") ";
    }
    std::cout << std::endl;
  }

  void BenchmarkFullGraphAccess() const {
    volatile int count = 0;
    auto start = std::chrono::high_resolution_clock::now();
    for (NodeID u = 0; u < num_nodes_; u+=1) {
      for (EdgeItem v: graph_[u].neighbors) {
        count += (v.paired_flag ? 1 : 0);
      }
    }

    auto end = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    cout << count << "# time to run the full access benchmark: " << duration/1000000000 << " seconds." << endl;
  }

private:
  int64_t num_nodes_;
  int64_t num_edges_;

  /// Release the vectors; first going through the inner vectors and then release @graph_
  void ReleaseResources() {
    for (NodeID i = 0; i < num_nodes_; i += 1) {
      graph_[i].neighbors = vector<EdgeItem>();
    }
    graph_ = vector<VertexItem>();
  }

  /// Traverse the @graph_ and mark paired-edges
  void mark_paired_edges() {
    for (NodeID u = 0; u < num_nodes_; u += 1) {
      for (int64_t i = 0; i < out_degree(u); i += 1) {
        // check if the reverse edge exist
        if (is_edge_exist(graph_[u].neighbors[i].v, u)) {
          graph_[u].neighbors[i].paired_flag = true;
          graph_[u].out_degree_paired += 1;
        }
      }
    }
  }

  /// Sugar for checking the existence of a directed-edge from node-id @u to @v
  bool is_edge_exist(NodeID u, NodeID v) {
    return binary_search(graph_[u].neighbors.begin(), graph_[u].neighbors.end(), EdgeItem(v));
  }

  /// Sort the neighbors list for each vertex
  void sort_neighbors() {
    for (NodeID u = 0; u < num_nodes_; u += 1) {
      sort(graph_[u].neighbors.begin(), graph_[u].neighbors.end());
    }
  }

  /// Sugar for inserting the edges from the input-file @filename
  void build_graph(std::string &filename) {
    // open the input file
    std::ifstream file(filename);
    if (!file.is_open()) {
      std::cout << "Couldn't open file " << filename << std::endl;
      std::exit(-2);
    }

    NodeID u, v;
    while (file >> u >> v) {
      // remove self edges
//      if(u > num_nodes_ || v > num_nodes_) cout << u << " " << v << endl;
//      assert(u>=0 && u<num_nodes_ && "Node-id can not be larger than the NUM_NODES");
//      assert(v>=0 && v<num_nodes_ && "Node-id can not be larger than the NUM_NODES");
      if (u != v) graph_[u].neighbors.push_back(EdgeItem(v));
    }
  }

  /// Sugar for inserting the reverse edges from graph @reverse
  void build_graph(const Graph &reverse) {
    for (NodeID u = 0; u < num_nodes_; u += 1) {
      for (EdgeItem neigh: reverse.graph_[u].neighbors) {
        // if edge "u -> neigh.v" is paired, then "neigh.v -> u" is also paired
        graph_[neigh.v].neighbors.push_back(EdgeItem(u, neigh.paired_flag));
        if (neigh.paired_flag) graph_[neigh.v].out_degree_paired += 1;
      }
    }
  }
};

#endif  // GRAPH_H_

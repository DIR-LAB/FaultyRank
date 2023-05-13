#include <iostream>
#include <fstream>
#include <chrono>
#include <unordered_map>
#include <vector>
#include "include/rhh/hash.hpp"

using namespace std;

#define INPUT_GRAPH "/mnt/cci-files/dataset-faultyrank/graph_build_1M/graph.txt"
#define EXISTING    "/mnt/cci-files/dataset-faultyrank/graph_build_1M/existing.txt"
#define FINAL_GRAPH "final_graph.txt"
#define UNFILLED    "unfilled.txt"

unordered_map<long long int, int, rhh::hash<long long int>> umap;

int main() {
  int count = 0;
  long long int src, dst;
  string src_str, dst_str;
  int line_num = 0;

  auto start_map = std::chrono::high_resolution_clock::now();
  //read line-by-line of existing.txt file
  std::ifstream existing(EXISTING);
  if (existing.is_open()) {
    std::string line;
    while (std::getline(existing, line)) {
      umap[stoll(line, 0, 16)] = count;
      count++;
    }
    //close existing.txt
    existing.close();
  }
  auto end_map = std::chrono::high_resolution_clock::now();
  double duration_map = std::chrono::duration_cast<std::chrono::nanoseconds>(end_map - start_map).count();
  cout << "Time to map the nodes " << duration_map / 1000000000 << " seconds." << endl;

  //read line-by-line input_graph.txt file
  auto start_graph = std::chrono::high_resolution_clock::now();
  std::ifstream input_graph(INPUT_GRAPH);
  auto end_it = umap.end();
  vector <pair<int, int>> graph;
  vector<int> unfilled_prop;
  while (input_graph >> src_str >> dst_str) {    //read src fid and dst fid
    src = stoll(src_str, 0, 16);
    dst = stoll(dst_str, 0, 16);

    auto src_it = umap.find(src);
    auto dst_it = umap.find(dst);

    //check if the dst fid is present in unordered fid => if no then put the node id in the unfilled.txt
    if (src_it != end_it) {
      if (dst_it == end_it) {
        //add src (node number) to unfilled.txt
        unfilled_prop.push_back(src_it->second);
      }
        //if dst fid is present put the edge list in final_graph.txt
      else {
        graph.push_back(make_pair(src_it->second, dst_it->second));
      }
    }
  }
  auto end_graph = std::chrono::high_resolution_clock::now();
  double duration_graph = std::chrono::duration_cast<std::chrono::nanoseconds>(end_graph - start_graph).count();
  cout << "Time to build final edge list " << duration_graph / 1000000000 << " seconds." << endl;

  //close input_graph.txt
  input_graph.close();

  cout << "graph size: " << graph.size() << ", unfilled property size: " << unfilled_prop.size() << endl;

  auto start_graph_write = std::chrono::high_resolution_clock::now();
  ofstream final_graph(FINAL_GRAPH);
  for (auto it: graph) final_graph << it.first << " " << it.second << endl;

  //close final_graph.txt
  final_graph.close();
  auto end_graph_write = std::chrono::high_resolution_clock::now();
  double duration_graph_write = std::chrono::duration_cast<std::chrono::nanoseconds>(
      end_graph_write - start_graph_write).count();
  cout << "Time to write final edge list " << duration_graph_write / 1000000000 << " seconds." << endl;

  auto start_unfilled_write = std::chrono::high_resolution_clock::now();
  ofstream unfilled(UNFILLED);

  for (int u: unfilled_prop) unfilled << u << endl;

  //close unfilled.txt
  unfilled.close();

  auto end_unfilled_write = std::chrono::high_resolution_clock::now();
  double duration_unfilled_write = std::chrono::duration_cast<std::chrono::nanoseconds>(
      end_unfilled_write - start_unfilled_write).count();
  cout << "Time to write unfilled property " << duration_unfilled_write / 1000000000 << " seconds." << endl;

  return 0;
}





//
// Created by Islam, Abdullah Al Raqibul on 9/6/22.
//

#include "graph.h"
#include "command_line.h"

/// small file
//#define INPUT_GRAPH "test-graphs/lma_c.txt"
//#define UNFILLED_PROPERTY_FILE "test-graphs/up_lma_c.txt"
//#define NUM_VERTICES 4
//#define NUM_EDGES 5
//#define MAX_ITERATION 20

typedef float ScoreT;
using namespace std::chrono;

/// Sugar for printing the rank-values (@id_rank and @p_rank) after @iter iterations
void print_ranks(const Graph &g, const Graph &rg, int iter, vector <ScoreT> &id_rank, vector <ScoreT> &p_rank) {
  cout << "\nafter " << iter << " iteration" << endl;
  cout << "id-rank: ";
  for (NodeID u = 0; u < g.num_nodes(); u += 1) cout << id_rank[u] << " ";
  cout << endl;

  cout << "p-rank: ";
  for (NodeID u = 0; u < rg.num_nodes(); u += 1) cout << p_rank[u] << " ";
  cout << endl;
}

void print_graph_property(const Graph &g, const Graph &rg) {
  int64_t count_isolated_nodes = 0;
  int64_t count_zero_in_g = 0;
  int64_t count_zero_in_rg = 0;

  for (NodeID u = 0; u < g.num_nodes(); u++) {
    if (g.out_degree(u) == 0 && rg.out_degree(u) == 0) count_isolated_nodes += 1;
    else if (g.out_degree(u)) count_zero_in_g += 1;
    else if (rg.out_degree(u)) count_zero_in_rg += 1;
    else {
      cout << "ERROR: Should not happen!" << endl;
      exit(-1);
    }
  }

  cout << "The graph have: " << count_isolated_nodes << " isolated nodes, ";
  cout << count_zero_in_g << " zero-degree nodes in G, and ";
  cout << count_zero_in_rg << " zero-degree nodes in RG" << endl;
}

void FaultyPageRank(const Graph &g, const Graph &rg,
                    int max_iters, double epsilon = 0) {
  const ScoreT init_score = 1.0f;
  const ScoreT zero_score = 0.0f;

  vector <ScoreT> p_rank_prev(g.num_nodes(), init_score);
  vector <ScoreT> id_rank(g.num_nodes(), zero_score);
  vector <ScoreT> p_rank(g.num_nodes(), zero_score);

  /// going to update @id_rank values from Graph @G
  for (int iter = 0; iter < max_iters; iter += 1) {
    ///update id rank
    for (NodeID u = 0; u < g.num_nodes(); u++) {
      if (g.out_degree(u) == 0) {
        cout << "Sink node in G: " << u << endl;
        ScoreT share = p_rank_prev[u] / (g.num_nodes() - 1);
        for (NodeID v = 0; v < g.num_nodes(); v++) {
          if (u != v) id_rank[v] += share;
        }
      } else {
        cout << "Nodes except sink node in G: " << u << endl;
        ScoreT share = p_rank_prev[u] / g.out_degree(u);
        for (EdgeItem neigh: g.graph_[u].neighbors) {
          id_rank[neigh.v] += share;
        }
      }
    }
    ///update property rank
    for (NodeID u = 0; u < rg.num_nodes(); u++) {
      if (rg.out_degree(u) == 0) {
        cout << "Sink node in RG" << u << endl;
        ScoreT share = id_rank[u] / (rg.num_nodes() - 1);
        for (NodeID v = 0; v < rg.num_nodes(); v++) {
          if (u != v) p_rank[v] += share;
        }
      } else if (rg.out_degree(u) > rg.out_degree_paired(u) && !rg.graph_[u].unfilled_property_flag &&
                 rg.out_degree_paired(u)) {
        cout << "Weighted node in RG" << u << endl;
        ScoreT share = id_rank[u] / rg.out_degree_paired(u);
        for (EdgeItem neigh: rg.graph_[u].neighbors) {
          if (neigh.paired_flag) p_rank[neigh.v] += share;
        }
      } else {
        cout << "remaining node in RG" << u << endl;
        ScoreT share = id_rank[u] / rg.out_degree(u);
        for (EdgeItem neigh: rg.graph_[u].neighbors) {
          p_rank[neigh.v] += share;
        }
      }
    }

    print_ranks(g, rg, iter, id_rank, p_rank);
    //#pragma omp parallel for
    for (NodeID u = 0; u < g.num_nodes(); u++) {
      p_rank_prev[u] = p_rank[u];
      p_rank[u] = 0.0;
      id_rank[u] = 0.0;
    }

    cout << "Done " << iter << " iteration." << endl;
  }
}

int main(int argc, char *argv[]) {
  CLApp cli(argc, argv);
  if (!cli.ParseArgs()) return -1;

  /// building the graph from @file
  auto start_g = std::chrono::high_resolution_clock::now();
  Graph G(cli.input_filename(), cli.num_nodes(), cli.num_edges());
  G.PrintTopology();
  auto end_g = std::chrono::high_resolution_clock::now();
  double duration_g = std::chrono::duration_cast<std::chrono::nanoseconds>(end_g - start_g).count();
  cout << "time to build the original graph: " << duration_g / 1000000000 << " seconds." << endl;

  /// building the reverse graph of Graph @G
  auto start_rg = std::chrono::high_resolution_clock::now();
  Graph RG(G);
  if (cli.has_unfilled_property()) RG.mark_unfilled_property_vertices(cli.up_filename());
  RG.PrintTopology();
  auto end_rg = std::chrono::high_resolution_clock::now();
  double duration_rg = std::chrono::duration_cast<std::chrono::nanoseconds>(end_rg - start_rg).count();
  cout << "time to build the reverse graph: " << duration_rg / 1000000000 << " seconds." << endl;

  /// call faulty page-rank algorithm
  auto start = std::chrono::high_resolution_clock::now();
  FaultyPageRank(G, RG, cli.max_iters());
  auto end = std::chrono::high_resolution_clock::now();
  double duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  cout << "time to run the faulty-rank algorithm: " << duration / 1000000000 << " seconds." << endl;
}

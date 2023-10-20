// Copyright (c) 2022, The DIR-LAB of the University of North Carolina at Charlotte
// See LICENSE.txt for license details

#ifndef COMMAND_LINE_H_
#define COMMAND_LINE_H_

#include <getopt.h>

#include <algorithm>
#include <cinttypes>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>


/*
FaultyRank
Class:  CLBase
Author: Raqib Islam

Handles command line argument parsing
 - Through inheritance, can add more options to object
 - For example, most kernels will use CLApp
*/


class CLBase {
protected:
  int argc_;
  char **argv_;
  std::string get_args_ = "N:E:f:u:i:t:";
  std::vector <std::string> help_strings_;

  int64_t num_nodes_ = 0;
  int64_t num_edges_ = 0;
  std::string input_filename_ = "";
  std::string up_filename_ = "";
  int max_iters_ = 10;
  double tolerance_ = 1e-4;

  void AddHelpLine(char opt, std::string opt_arg, std::string text,
                   std::string def = "") {
    const int kBufLen = 100;
    char buf[kBufLen];
    if (opt_arg != "")
      opt_arg = "<" + opt_arg + ">";
    if (def != "")
      def = "[" + def + "]";
    snprintf(buf, kBufLen, " -%c %-9s: %-54s%10s", opt, opt_arg.c_str(),
             text.c_str(), def.c_str());
    help_strings_.push_back(buf);
  }

public:
  CLBase(int argc, char **argv) : argc_(argc), argv_(argv) {
    AddHelpLine('h', "", "print this help message");
    AddHelpLine('N', "base_filename_", "load base-graph from file");
    AddHelpLine('E', "dynamic_filename_", "load dynamic-graph from file");
    AddHelpLine('f', "input_file", "load graph from file");
    AddHelpLine('u', "unfilled_property_file", "load unfilled property vertices from file");
    AddHelpLine('i', "i", "perform at most i iterations", std::to_string(max_iters_));
    AddHelpLine('t', "t", "use tolerance t", std::to_string(tolerance_));
  }

  bool ParseArgs() {
    signed char c_opt;
    extern char *optarg;          // from and for getopt
    while ((c_opt = getopt(argc_, argv_, get_args_.c_str())) != -1) {
      HandleArg(c_opt, optarg);
    }
    if (input_filename_ == "") {
      std::cout << "No graph input specified. (Use -h for help)" << std::endl;
      return false;
    }
    if (!num_nodes_ || !num_edges_) {
      std::cout << "Need to specify graph properties. (Use -h for help)" << std::endl;
      return false;
    }
    return true;
  }

  void virtual HandleArg(signed char opt, char *opt_arg) {
    switch (opt) {
      case 'N':
        num_nodes_ = atol(opt_arg);
        break;
      case 'E':
        num_edges_ = atol(opt_arg);
        break;
      case 'f':
        input_filename_ = std::string(opt_arg);
        break;
      case 'u':
        up_filename_ = std::string(opt_arg);
        break;
      case 'i':
        max_iters_ = atoi(opt_arg);
        break;
      case 't':
        tolerance_ = std::stod(opt_arg);
        break;
    }
  }

  void PrintUsage() {
    for (std::string h: help_strings_)
      std::cout << h << std::endl;
    std::exit(0);
  }

  int64_t num_nodes() const { return num_nodes_; }

  int64_t num_edges() const { return num_edges_; }

  std::string input_filename() const { return input_filename_; }

  std::string up_filename() const { return up_filename_; }

  bool has_unfilled_property() const { return (up_filename_ != ""); }

  int max_iters() const { return max_iters_; }

  double tolerance() const { return tolerance_; }
};


class CLApp : public CLBase {
  bool do_analysis_ = false;
  int num_trials_ = 16;
  bool do_verify_ = false;

public:
  CLApp(int argc, char **argv) : CLBase(argc, argv) {
    get_args_ += "an:v";
    AddHelpLine('a', "", "output analysis of last run", "false");
    AddHelpLine('n', "n", "perform n trials", std::to_string(num_trials_));
    AddHelpLine('v', "", "verify the output of each run", "false");
  }

  void HandleArg(signed char opt, char *opt_arg) override {
    switch (opt) {
      case 'a':
        do_analysis_ = true;
        break;
      case 'n':
        num_trials_ = atoi(opt_arg);
        break;
      case 'v':
        do_verify_ = true;
        break;
      default:
        CLBase::HandleArg(opt, opt_arg);
    }
  }

  bool do_analysis() const { return do_analysis_; }

  int num_trials() const { return num_trials_; }

  bool do_verify() const { return do_verify_; }
};

#endif  // COMMAND_LINE_H_

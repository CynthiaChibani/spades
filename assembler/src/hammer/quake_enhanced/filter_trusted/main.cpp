#include <stdint.h>
#include <cstdlib>
#include <cstdio>
#include <string>
#include "logging.hpp"

using std::string;

namespace {
/**
 * @variable Length of string buffer which will store k-mer.
 */
const uint32_t kMaxK = 100;
/**
 * @variable Every kStep k-mer will appear in the log.
 */
const int kStep = 1e5;

DECL_LOGGER("filter_trusted")

struct Options {
  string ifile;
  string ofile;
  string badfile;
  float threshold;
  bool valid;
  Options()
      : ifile(""),
        ofile(""),
        badfile(""),
        threshold(-1),
        valid(true) {}
};

void PrintHelp(char *progname) {
  printf("Usage: %s ifile.[q]cst ofile.trust ofile.bad threshold\n", progname);
  printf("Where:\n");
  printf("\tifile.[q]cst\tfile with k|q-mer statistics\n");
  printf("\tofile.trust\ta filename where filtered data will be outputted\n");
  printf("\tofile.bud\ta filename where filtered garbage will be outputted\n");
  printf("\tthreshold\tq-mer threshold\n");
}

Options ParseOptions(int argc, char *argv[]) {
  Options ret;
  if (argc != 5) {
    ret.valid = false;
  } else {
    ret.ifile = argv[1];
    ret.ofile = argv[2];
    ret.badfile = argv[3];
    ret.threshold = atof(argv[4]);
    if (ret.threshold <= -1e-5) {
      ret.valid = false;
    }
  }
  return ret;
}

void run(const Options &opts) {
  INFO("Starting filter_trusted: evaluating "
       << opts.ifile << ".");
  FILE *ifile = fopen(opts.ifile.c_str(), "r");
  FILE *ofile = fopen(opts.ofile.c_str(), "w");
  FILE *badfile = fopen(opts.badfile.c_str(), "w");
  char kmer[kMaxK];
  char format[20];
  float freq = -1;
  int count;
  float q_count;
  snprintf(format, sizeof(format), "%%%ds%%d%%f%%f", kMaxK);
  uint64_t read_number = 0;
  while (fscanf(ifile, format, kmer, &count, &q_count, &freq) != EOF) {
    ++read_number;
    if (read_number % kStep == 0) {
      INFO("Reading k-mer " << read_number << ".");
    }
    if (q_count > opts.threshold) {
      fprintf(ofile, "%s %d %f %f\n", kmer, count, q_count, freq);
    } else {
      fprintf(badfile, "%s %d %f %f\n", kmer, count, q_count, freq);
    }
  }
}
}


int main(int argc, char *argv[]) {
  Options opts = ParseOptions(argc, argv);
  if (!opts.valid) {
    PrintHelp(argv[0]);
    return 1;
  }
  return 0;
}

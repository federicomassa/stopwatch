#include "stopwatch.h"
#include <iostream>

using namespace Tazio;

int main() {

  Stopwatch sw;
  std::vector<std::string> labels;
  labels.push_back("p1");
  labels.push_back("p2");

  sw.set_labels(labels);
  sw.set_output_file("/home/nvidia/test.txt");

  for (int it = 0; it < 100; it++) {
	  sw.init();

	  double a = 0;
	  for (int i = 0; i < 1000000; i++) {
		a += 0.1;
	  }

	  sw.partial("p1");
	  
	  for (int i = 0; i < 1000000; i++) {
		a -= 0.1;
	  }

	  sw.partial("p2");

	  sw.end();
  }
  
  return 0;
}

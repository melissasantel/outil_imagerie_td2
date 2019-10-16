#include <iostream>
#include <cstdlib>

#include <opencv2/opencv.hpp>
#include <fstream>

using namespace cv;
using namespace std;

void
process(const char* imsname)
{
  (void) imsname;
}

void
usage (const char *s)
{
  std::cerr<<"Usage: "<<s<<" imsname imdname\n"<<std::endl;
  exit(EXIT_FAILURE);
}

#define param 2
int
main( int argc, char* argv[] )
{
  if(argc != (param+1))
    usage(argv[0]);
  process(NULL);
  return EXIT_SUCCESS;
}
;

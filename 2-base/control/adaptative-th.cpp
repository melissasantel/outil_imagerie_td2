#include <iostream>
#include <cstdlib>

#include <opencv2/opencv.hpp>
#include <fstream>

using namespace cv;
using namespace std;

void
process(const char* imsname, int radius, int cons)
{
  //Check the existence of the file, if the file doesn't exist the programme stop
  fstream infile(imsname);
  if (infile.good() == false){
    cerr<<"The file doesn't exist. Check the location of the file\n"<<endl;
    exit(EXIT_FAILURE);
  }
  cout<< "\n############### exercice : adaptative-th ##############\n"<<endl;
  //Read the image load
  Mat ims = imread(imsname, 0);

  Mat ims_th_ocv(ims.size(), CV_8UC1);
  adaptiveThreshold(ims, ims_th_ocv,255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, radius, cons);
  imwrite("th-ocv-mean.png",ims_th_ocv);

  Mat ims_th_gauss(ims.size(), CV_8UC1);
  adaptiveThreshold(ims, ims_th_gauss,255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, radius, cons);
  imwrite("th-ocv-gauss.png",ims_th_gauss);


}

void
usage (const char *s)
{
  std::cerr<<"Usage: "<<s<<" imsname imdname\n"<<std::endl;
  exit(EXIT_FAILURE);
}

#define param 3
int
main( int argc, char* argv[] )
{
  if(argc != (param+1))
    usage(argv[0]);
  process(argv[1],atoi(argv[2]),atoi(argv[3]));
  return EXIT_SUCCESS;
}
;

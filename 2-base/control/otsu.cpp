#include <iostream>
#include <cstdlib>

#include <opencv2/opencv.hpp>
#include <fstream>

using namespace cv;
using namespace std;

void
process(const char* imsname)
{
  //Check the existence of the file, if the file doesn't exist the programme stop
  fstream infile(imsname);
  if (infile.good() == false){
    cerr<<"The file doesn't exist. Check the location of the file\n"<<endl;
    exit(EXIT_FAILURE);
  }
  cout<< "\n############### exercice : hsv-modification ##############\n"<<endl;
  //Read the image load
  Mat ims = imread(imsname, 0);
  Size s = ims.size();

  Mat imd_otsu_ocv(s,CV_8UC1) ;
  double manual_T = threshold(ims, imd_otsu_ocv, 123, 255, THRESH_BINARY+THRESH_OTSU);
  cout << "manual_T " << manual_T<<endl;
  imshow("otsu-th-ocv.png",imd_otsu_ocv);
  waitKey(0);

}

void
usage (const char *s)
{
  std::cerr<<"Usage: "<<s<<" imsname imdname\n"<<std::endl;
  exit(EXIT_FAILURE);
}

#define param 1
int
main( int argc, char* argv[] )
{
  if(argc != (param+1))
    usage(argv[0]);
  process(argv[1]);
  return EXIT_SUCCESS;
}

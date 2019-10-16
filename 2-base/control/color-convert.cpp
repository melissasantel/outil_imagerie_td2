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
  cout<< "\n############### exercice : color-convert ##############\n"<<endl;
  //Read the image load
  Mat ims = imread(imsname, CV_LOAD_IMAGE_COLOR);

  //Split the loaded image to obtain the B,G,R composant
  Mat imsBGRSplit[3];
  split(ims,imsBGRSplit);
  //Show the R range
  imshow("R",imsBGRSplit[2]);
  waitKey(0);

  //Show the G range
  imshow("G",imsBGRSplit[1]);
  waitKey(0);

  //Show the B range
  imshow("B",imsBGRSplit[0]);
  waitKey(0);

  //Convert the loaded image to RGB to YCbCr image
  Mat imsYCbCr(ims.size(),CV_8UC3);
  cvtColor(ims,imsYCbCr,CV_BGR2YCrCb);
  Mat imsYCbCrSplit[3];
  split(imsYCbCr,imsYCbCrSplit);
  //Show the Y range
  imshow("Y", imsYCbCrSplit[0]);
  waitKey(0);
  //Show the Cb range
  imshow("Cb", imsYCbCrSplit[1]);
  waitKey(0);
  //Show the Cr range
  imshow("Cr", imsYCbCrSplit[2]);
  waitKey(0);

  //Convert the loaded image to RGB to gray image
  Mat imsGray(ims.size(),CV_8UC1);
  cvtColor(ims,imsGray,CV_BGR2GRAY);
  //Show the Gray image
  imshow("RGBtoGray",imsGray);
  waitKey(0);

  //Convert the YCbCr image previously created to a color
  Mat imsBGR(ims.size(),CV_8UC3);
  cvtColor(imsYCbCr,imsBGR,CV_YCrCb2BGR);
  //Show the BGR image
  imshow("RGB→YCbCr→RGB",imsBGR);
  waitKey(0);

  Mat diffGrayAndY = imsGray - imsYCbCrSplit[0];
  imshow("Difference image Gray et Y",diffGrayAndY);
  waitKey(0);

  Mat diffInitAndRGB = ims - imsBGR;
  imshow("Difference image initiale et reconvertit",diffInitAndRGB);
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

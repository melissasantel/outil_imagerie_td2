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
  Mat imsYCrCb(ims.size(),CV_8UC3);
  cvtColor(ims,imsYCrCb,CV_BGR2YCrCb);
  Mat imsYCrCbSplit[3];
  split(imsYCrCb,imsYCrCbSplit);
  //Show the Y range
  imshow("Y", imsYCrCbSplit[0]);
  waitKey(0);
  //Show the Cb range
  imshow("Cr", imsYCrCbSplit[1]);
  waitKey(0);
  //Show the Cr range
  imshow("Cb", imsYCrCbSplit[2]);
  waitKey(0);

  //Convert the loaded image to RGB to gray image
  Mat imsGray(ims.size(),CV_8UC1);
  cvtColor(ims,imsGray,CV_BGR2GRAY);
  //Show the Gray image
  imshow("RGBtoGray",imsGray);
  waitKey(0);

  //Convert the YCbCr image previously created to a color
  Mat imsBGR(ims.size(),CV_8UC3);
  cvtColor(imsYCrCb,imsBGR,CV_YCrCb2BGR);
  //Show the BGR image
  imshow("RGB→YCrCb→RGB",imsBGR);
  waitKey(0);

  //Make the difference between the image convert in Gray
  Mat diffGrayAndY = imsGray - imsYCrCbSplit[0];
  imshow("Difference image Gray et Y",diffGrayAndY);
  waitKey(0);

  //Make the difference between the first image and the image convert to YCbCr
  //and then reconvert to RGB
  Mat diffInitAndRGB = ims - imsBGR;
  imshow("Difference image RGB initiale et reconvertit",diffInitAndRGB);
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

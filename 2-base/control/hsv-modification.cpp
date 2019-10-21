#include <iostream>
#include <cstdlib>

#include <opencv2/opencv.hpp>
#include <fstream>

using namespace cv;
using namespace std;

void
process(int h, int s, int v, const char* imsname, const char* imdname)
{
  //Check the existence of the file, if the file doesn't exist the programme stop
  fstream infile(imsname);
  if (infile.good() == false){
    cerr<<"The file doesn't exist. Check the location of the file\n"<<endl;
    exit(EXIT_FAILURE);
  }
  cout<< "\n############### exercice : hsv-modification ##############\n"<<endl;
  //Read the image load
  Mat ims = imread(imsname, CV_LOAD_IMAGE_COLOR);

  //Create an image to make the conservation HSV
  Mat imsHSV(ims.size(), CV_8UC3);
  cvtColor(ims,imsHSV, CV_BGR2HSV);

  //Split the HSV image to display all its composants
  Mat imsHSV_split[3];
  split(imsHSV,imsHSV_split);
  //Show the H composant
  Mat imsHSV_H = imsHSV_split[0];
  imshow("H",imsHSV_split[0]);
  waitKey(0);
  //Show the S composant
  Mat imsHSV_S = imsHSV_split[1];
  imshow("S",imsHSV_split[1]);
  waitKey(0);
  //Show the V composant
  Mat imsHSV_V = imsHSV_split[2];
  imshow("V",imsHSV_split[2]);
  waitKey(0);

  //Changed the H,S,V channels of imsHSV image
  //With the arguments given h, s, v
  imsHSV_H +=h;
  imsHSV_S +=s;
  imsHSV_V +=v;

  Mat imd(ims.size(), CV_8UC3);

  //Merge the matrice to make an image on 3 chanels
  vector<Mat> hsv_channels;
  hsv_channels.push_back(imsHSV_H);
  hsv_channels.push_back(imsHSV_S);
  hsv_channels.push_back(imsHSV_V);
  merge(hsv_channels, imd);

  //Display the HSV image reconvert to RGB
  cvtColor(imd,imd, CV_HSV2BGR);
  //Save the produce image
  imwrite(imdname,imd);
}

void
usage (const char *s)
{
  std::cerr<<"Usage: "<<s<<" imsname imdname\n"<<std::endl;
  exit(EXIT_FAILURE);
}

#define param 5
int
main( int argc, char* argv[] )
{
  if(argc != (param+1))
    usage(argv[0]);
  process(atoi(argv[1]),atoi(argv[2]),atoi(argv[3]),argv[4],argv[5]);
  return EXIT_SUCCESS;
}

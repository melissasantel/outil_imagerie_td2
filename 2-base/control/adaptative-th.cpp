#include <iostream>
#include <cstdlib>

#include <opencv2/opencv.hpp>
#include <fstream>

using namespace cv;
using namespace std;

void
process(const char* imsname, int radius, int cst)
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
  Size s = ims.size();

  //Calcul the neighbor tall
  int block_size = 2*radius+1;

  //Sum of the neighbor pixels
  int pixels_neighbor_sum=0;
  int neighbor_nb =0 ;

  int value_FdeV;
  int mean_pixels;

  //Create the adaptative th image
  Mat ims_th(s,CV_8UC1);

  for(int i=0; i<s.height; i++){
    for(int j=0; j<s.width; j++){

      for(int l=-radius; l<=radius; l++){
        for(int m=-radius ;m<=radius; m++){
          if(i+l>=0 && j+m>=0 && i+l<s.height && j+m<s.width){
            pixels_neighbor_sum += ims.at<uchar>(i+l,j+m);
            neighbor_nb +=1;
          }
        }
      }

      mean_pixels = pixels_neighbor_sum/neighbor_nb;
      value_FdeV = mean_pixels-cst;

      if(ims.at<uchar>(i,j)<value_FdeV)
        ims_th.at<uchar>(i,j)=0;
      else
        ims_th.at<uchar>(i,j)=255;

      //Clear variables
      value_FdeV=0;
      mean_pixels=0;
      pixels_neighbor_sum =0;
      neighbor_nb =0;
    }
  }

  imwrite("th.png",ims_th);

  Mat ims_th_ocv(ims.size(), CV_8UC1);
  adaptiveThreshold(ims, ims_th_ocv,250, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, block_size, cst);
  imwrite("th-ocv-mean.png",ims_th_ocv);

  //Calcul the difference between the two methods
  Mat diff_th(ims.size(), CV_8UC1);
  diff_th = ims_th-ims_th_ocv;
  imwrite("diff.png",diff_th);

  Mat ims_th_gauss(ims.size(), CV_8UC1);
  adaptiveThreshold(ims, ims_th_gauss,255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, block_size, cst);
  imwrite("th-ocv-gauss.png",ims_th_gauss);

  waitKey(0);
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

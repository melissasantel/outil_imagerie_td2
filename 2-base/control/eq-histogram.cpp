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
  cout<< "\n############### exercice : eq-histogram ##############\n"<<endl;
  //Read the image load
  Mat ims = imread(imsname,0);
  Size s = ims.size();
  int M = s.height;
  int N = s.width;

  //Calcul the histogram
  vector<int> ims_histogram = vector<int>(256);
  int k;
  for(int i=0;i<M;i++){
    for(int j=0;j<N;j++){
      k = ims.at<uchar>(i,j);
      ims_histogram.at(k) +=1;
    }
  }
  //Calcul the histogram cumule
  vector<int> ims_histogram_cm = vector<int>(256);
  for(int k=0;k<(int)ims_histogram_cm.size();k++){
    for(int l=0;l<=k; l++){
      ims_histogram_cm.at(k) += ims_histogram.at(l);
    }
  }

  //Make test with 255 and it's work
  int Imax = 255;
  //Re calculate by hand the image
  //with the cumulate normalized histogram method
  int k_2;
  Mat imd_eq(s, CV_8UC1);
  for(int i=0;i<M;i++){
    for(int j=0;j<N;j++){
      k_2=ims.at<uchar>(i,j);
      imd_eq.at<uchar>(i,j)= Imax*ims_histogram_cm.at(k_2)/(M*N);
    }
  }
  imwrite("eq.png",imd_eq);

  //Get an modified image
  //with the cumulate normalized histogram method of opencv
  Mat imd_eq_ocv(ims.size(), CV_8UC1);
  equalizeHist(ims, imd_eq_ocv);
  imwrite("eq-ocv.png", imd_eq_ocv);

  //Make diffenrence between the 2 method
  Mat diff_eg_h;
  diff_eg_h = imd_eq_ocv-imd_eq;
  imwrite("diff.png",diff_eg_h);

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

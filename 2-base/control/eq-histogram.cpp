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
      k = ims.at<Vec3b>(i,j)[0];
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

  // int l_2=0;
  // while (l_2<256){
  //   cout<< l_2 << " : "<< ims_histogram.at(l_2)<<endl;
  //   cout<< l_2 << " : "<< ims_histogram_cm.at(l_2)<<endl;
  //   l_2++;
  // }

  //Make test with 255 and it's work
  int Imax = 255;
  //Re calculate by hand the image
  //with the cumulate normalized histogram method
  int k_2;
  Mat imd_eq(ims.size(), CV_8UC1);
  for(int i=0;i<M;i++){
    for(int j=0;j<N;j++){
      k_2=ims.at<Vec3b>(i,j)[0];
      imd_eq.at<Vec3b>(i,j)[0]= Imax*ims_histogram_cm.at(k_2)/(M*N);
    }
  }
  imshow("eq.png",imd_eq);
  waitKey(0);
  //Get an modified image
  //with the cumulate normalized histogram method of opencv
  Mat imd_eq_ocv(ims.size(), CV_8UC1);
  equalizeHist(ims, imd_eq_ocv);
  imshow("eq-ocv.png", imd_eq_ocv);
  waitKey(0);

  //Make diffenrence between the 2 method
  // Mat diff_eg_h;
  // diff_eg_h = imd_eq_ocv-imd_eq;
  // imshow("diff.png",diff_eg_h);
  // waitKey(0);

  cvtColor(imd_eq,imd_eq,CV_GRAY2BGR);
  imwrite("eq.png",imd_eq);
  cvtColor(imd_eq_ocv,imd_eq_ocv,CV_GRAY2BGR);
  imwrite("eq-ocv.png", imd_eq_ocv);


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

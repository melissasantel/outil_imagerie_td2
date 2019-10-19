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
  int img_size = s.height*s.width;

  float threshold_value,proba_1,proba_2,mean_1,sum_for_mean_1,sum_for_mean_2 = 0;
  float mean_2,inter_class_variance,variance_inter_max = 0;
  int max_value =255;

  //Calcul the histogram
  vector<int> ims_histogram = vector<int>(256);
  int k;
  for(int i=0;i<s.height;i++){
    for(int j=0;j<s.width;j++){
      k = ims.at<uchar>(i,j);
      ims_histogram.at(k) +=1;
    }
  }

  for(int p=0;p<max_value;p++){
    sum_for_mean_1 += p*ims_histogram[p];
  }
  cout<<"sum_1 "<<sum_for_mean_1<<endl;;

  for(int t=0;t<max_value;t++){
    proba_1 += ims_histogram.at(t);
    if(proba_1==0){
      continue;
    }
    proba_2 = img_size - proba_1;

    sum_for_mean_2 += t*ims_histogram[t];
    mean_1 = sum_for_mean_2/proba_1;
    mean_2 = (sum_for_mean_1-sum_for_mean_2)/proba_2;

    inter_class_variance = proba_1*proba_2*pow((mean_1-mean_2),2);

    if(inter_class_variance>variance_inter_max){
      threshold_value = t;
      variance_inter_max = inter_class_variance;
    }
  }

  Mat imd_otsu(s,CV_8UC1);
  for(int i=0; i<s.height;i++){
    for(int j=0; j<s.width; j++){
      if(ims.at<uchar>(i,j)>threshold_value)
        imd_otsu.at<uchar>(i,j) = 1;
      else
        imd_otsu.at<uchar>(i,j) = 0;
    }
  }
  cout << "manual otsu threshold t = " << threshold_value<<endl;
  // imshow("otsu-th.png",imd_otsu);
  imwrite("otsu-th.png",imd_otsu);

  Mat imd_otsu_ocv(s,CV_8UC1) ;
  double manual_T = threshold(ims, imd_otsu_ocv, 123, 255, THRESH_BINARY+THRESH_OTSU);
  cout << "ocv otsu threshold t = " << manual_T<<endl;
  // imshow("otsu-th-ocv.png",imd_otsu_ocv);
  imwrite("otsu-th-ocv.png",imd_otsu_ocv);

  Mat diff_otsu(s,CV_8UC1);
  diff_otsu = imd_otsu-imd_otsu_ocv;
  // imshow("diff-otsu-method",diff_otsu);

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

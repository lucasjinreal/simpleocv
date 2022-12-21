#include "simpleocv.h"

int main(int argc, char **argv) {

  std::string img_f = argv[1];

  cv::Mat a = cv::imread(img_f);
  cv::putText(a, "28.9 C from SimpleOCV", cv::Point(20, 45), 1, 0.5,
              cv::Scalar(255, 0, 255));
  cv::imwrite("a_gray.png", a);
}
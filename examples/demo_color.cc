#include "simpleocv.h"
#include <iostream>

int main(int argc, char **argv) {

  std::string img_f = argv[1];

  cv::Mat a = cv::imread(img_f);

  auto a_size = a.size();

  std::cout << a_size.width << "x" << a_size.height << std::endl;
  std::cout << a.cols << "x" << a.rows << std::endl;

  cv::putText(a, "28.9 C from SimpleOCV", cv::Point(20, 45), 1, 0.5,
              cv::Scalar(255, 0, 255));
  cv::imwrite("a_gray.png", a);
}
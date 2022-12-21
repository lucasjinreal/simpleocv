# Simple OpenCV

该仓库主要是在一些移动场景下，替代OpenCV的功能，例如一些图片的读写、预处理、resize、gui等被替换，并且大部分情况下你可以把SimpleOCV直接集成到你的项目中，甚至可以直接编译成wasm在浏览器运行。

SimpleOCV大部分实现均来自于ncnn，为了使得它更加模块化，单独拎出来作为一个独立库，方便任何项目进行集成。

理论上我们也可以使用opencv-mobile的版本，但这个东西是在原有的opencv源码上patch出来的，灵活性太差了，直接下载预先编译好的二进制也会出很多问题。

SimpleOCV尽可能秉承以下几个原则：

- 最小化：精简你今需要的几个接口；
- 跨平台：浏览器都能跑，还有什么不能跑；
- 方便集成：任何C++项目都能集成

目的就是：对于不是很复杂的项目，彻底抛弃OpenCV，但是对于已有的项目，SimpleOCV的include接口，保持和OpenCV一模一样。o


## 用法

你不需要opencv，只需要这样：

```c++
#include "simpleocv.h"

int main(int argc, char **argv) {

  std::string img_f = argv[1];

  cv::Mat a = cv::imread(img_f);
  cv::putText(a, "28.9 C from SimpleOCV", cv::Point(20, 45), 1, 0.5,
              cv::Scalar(255, 0, 255));
  cv::imwrite("a_gray.png", a);
}
```

然后，你就有了一个和opencv一模一样的能力。

上面你就可以看到这样一个可视化的图：

![](https://raw.githubusercontent.com/jinfagang/public_images/master/20221221165207.png)

请注意！这里面没有用到任何opencv的代码。


## 编译

```
mkdir build
cd build
cmake ..
make -j8

./examples/demo_color bus.jpg
```

## 后续

本项目旨在让你不依赖OpenCV，拥有基础的opencv能力，后续我会持续精简相关代码。并且尝试编译到其他平台，例如web，iOS等，这些平台你可以用SimploeOCV来做预处理，但是已经不需要opencv的依赖了。






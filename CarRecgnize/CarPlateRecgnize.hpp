//
//  CarPlateRecgnize.hpp
//  CarRecgnize
//
//  Created by liuyang on 2020/10/3.
//  Copyright © 2020 liuyang. All rights reserved.
//

#ifndef CarPlateRecgnize_hpp
#define CarPlateRecgnize_hpp
#include "CarSobelPlateLocation.hpp"
#include "CarColorPlateLocation.hpp"

#include <string>
using namespace std;

#include <opencv2/ml.hpp>
using namespace ml;
class  CarPlateRecgnize {
    
public:
        CarPlateRecgnize(const char* svm_model);
    
        ~CarPlateRecgnize();
    
         /**
          识别车牌返回给调用者 1.定位 2 识别
          */
    string plateRecgnize(Mat src);
    
private:
    void getHogFeatures(HOGDescriptor* svmHog,Mat src,Mat& out);
    void clearFixPoint(Mat& src);
    int verityCharSize(Mat src);
    int getCityIndex(vector<Rect> src);

    CarSobelPlateLocation *pateSobelLocation=0;
    CarColorPlateLocation *pateColorLocation=0;
    Ptr<SVM> svm = 0;
    HOGDescriptor *svmHog = 0;
};

#endif

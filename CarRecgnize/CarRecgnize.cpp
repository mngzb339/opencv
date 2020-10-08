//
//  CarRecgnize.cpp
//  CarRecgnize
//
//  Created by liuyang on 2020/10/3.
//  Copyright © 2020 liuyang. All rights reserved.
//

#include "CarPlateRecgnize.hpp"

int main()
{
    CarPlateRecgnize *p = new CarPlateRecgnize("/Users/liuyang/Desktop/opencv/HOG_SVM_DATA2.xml");
    Mat src= imread("/Users/liuyang/Desktop/opencv/test1.jpg");
    p->plateRecgnize(src);
    waitKey();
    return 0;
}

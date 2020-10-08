//
//  CarColorPlateLocation.hpp
//  CarRecgnize
//
//  Created by liuyang on 2020/10/6.
//  Copyright © 2020 liuyang. All rights reserved.
//

#ifndef CarColorPlateLocation_hpp
#define CarColorPlateLocation_hpp

#include "CarPlateLocation.hpp"

class CarColorPlateLocation :public CarPlateLocation{
public:
    CarColorPlateLocation();
    ~CarColorPlateLocation();

    // 1、要定位的图片 2、引用类型 作为定位结果
    void location(Mat src, vector<Mat>& dst);

};

#endif

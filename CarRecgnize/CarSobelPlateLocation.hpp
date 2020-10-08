//
//  CarPlateLocationRecognize.hpp
//  CarRecgnize
//
//  Created by liuyang on 2020/10/3.
//  Copyright © 2020 liuyang. All rights reserved.
//

#ifndef CarSobelPlateLocation_hpp
#define CarSobelPlateLocation_hpp

#include "CarPlateLocation.hpp"

class CarSobelPlateLocation :public CarPlateLocation{
public:
    CarSobelPlateLocation();
    ~CarSobelPlateLocation();
    /**
          1.输入源。2.输出地址
     */
    void location(Mat src,vector<Mat>& dst);
};
#endif /* CarPlateLocationRecognize_hpp */

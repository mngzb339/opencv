//
//  CarColorPlateLocation.cpp
//  CarRecgnize
//
//  Created by liuyang on 2020/10/6.
//  Copyright © 2020 liuyang. All rights reserved.
//

#include "CarColorPlateLocation.hpp"
CarColorPlateLocation::CarColorPlateLocation() {
 
}
CarColorPlateLocation::~CarColorPlateLocation() {
   
}
void CarColorPlateLocation::location(Mat src, vector<Mat>& dst) {
   //1、转成HSV
    Mat hsv;
    cvtColor(src,hsv,COLOR_BGR2HSV);
    
    
    //3通道
    int chanles = hsv.channels();
    //高
    int h = hsv.rows;
    //宽数据长度
    int w = hsv.cols * 3;
    //判断数据是否为一行存储的
    //内存足够的话 mat的数据是一块连续的内存进行存储
    if (hsv.isContinuous()) {
        w *= h;
        h = 1;
    }
    for (size_t i = 0; i < h; i++)
    {
        //第i行的数据 一行 hsv的数据 uchar = java byte
        uchar* p = hsv.ptr<uchar>(i);
        //处理3个数据 所以步长为3
        for (size_t j = 0; j < w; j+=3)
        {
            int h = int(p[j]);
            int s = int(p[j+1]);
            int v = int(p[j + 2]);
            //是否为蓝色像素点的标记
            bool blue = false;
            //蓝色
            if (h >= 100 && h <= 124 && s >= 43 && s <= 255 && v >= 46 && v <= 255) {
                blue = true;
            }
            //把蓝色像素 凸显出来 ，其他的区域全变成黑色
            
            if (blue) {
                p[j] = 0;
                p[j+1] = 0;
                p[j + 2] = 255;
            }
            else {
                //hsv 模型 h：0 红色 亮度和饱和度都是0 ，也就变成了黑色
                p[j] = 0;
                p[j + 1] = 0;
                p[j + 2] = 0;
            }
        }
    }
    //把亮度数据抽离出来
    // 把hsv 分离出来
    vector<Mat> hsv_split;
    split(hsv, hsv_split);
    
    //二值化
    Mat shold;
    threshold(hsv_split[2], shold, 0, 255, THRESH_OTSU + THRESH_BINARY);
    
    
    Mat close;
    Mat element = getStructuringElement(MORPH_RECT, Size(17, 3));
    morphologyEx(shold, close, MORPH_CLOSE, element);
    
    //6、查找轮廓
    //获得初步筛选车牌轮廓================================================================
    //轮廓检测
    vector< vector<Point> > contours;
    //查找轮廓 提取最外层的轮廓  将结果变成点序列放入 集合
    findContours(close, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    //遍历
    vector<RotatedRect> vec_color_roi;
    for (vector<Point> point : contours) {
        RotatedRect rotatedRect = minAreaRect(point);
        //rectangle(src, rotatedRect.boundingRect(), Scalar(255, 0, 255));
        //进行初步的筛选 把完全不符合的轮廓给排除掉 ( 比如：1x1，5x1000 )
        if (verifySizes(rotatedRect)) {
            vec_color_roi.push_back(rotatedRect);
        }
    }
    //矫正
    tortuosity(src, vec_color_roi, dst);
}

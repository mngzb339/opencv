
#include "CarSobelPlateLocation.hpp"

CarSobelPlateLocation::CarSobelPlateLocation() {
 
}
CarSobelPlateLocation::~CarSobelPlateLocation() {
   
}

void CarSobelPlateLocation::location(Mat src, vector<Mat>& dst) {
    //预处理 去除噪声 让车牌区域更加突出
    Mat blur;
    //1、高斯模糊（平滑） （1、为了后续操作 2、降噪 ）
    GaussianBlur(src,blur,Size(5,5),0);
    //imshow("高斯模糊", blur);
    
    Mat gray;
    //去掉颜色 灰度化 最终目的 降噪
    cvtColor(blur, gray, COLOR_BGRA2GRAY);

    Mat soble_16;
    // 采用soble边缘检测 16位主要为了精准 轮廓化 在后续操作 显示时需要转回8位

    Sobel(gray, soble_16, CV_16S, 1, 0);
    //转为8位，不然显示不出来
    Mat soble_8;
    convertScaleAbs(soble_16, soble_8);
    // 二值化 黑白 (大律法:最大类间算法)
    Mat shold;
    threshold(soble_8, shold, 0, 255, THRESH_OTSU+THRESH_BINARY);
    //闭操作 将相邻的白色区域扩大 连接一个整体
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
    vector<RotatedRect> vec_sobel_roi;
    for (vector<Point> point:contours) {
        RotatedRect rotatedRect = minAreaRect(point);
        //rectangle(src, rotatedRect.boundingRect(), Scalar(255, 0, 255));
        //进行初步的筛选 把完全不符合的轮廓给排除掉 ( 比如：1x1，5x1000 )
        if (verifySizes(rotatedRect)) {
            vec_sobel_roi.push_back(rotatedRect);
        }
    }
    //矫正 因为拍照的照片有可能是斜的
    //获得候选车牌
    // 整个图片+经过初步赛选的车牌 + 得到的候选车牌
    tortuosity(src, vec_sobel_roi, dst);
    //imshow("找到轮廓",src);
    blur.release();
    gray.release();
   //......
   // waitKey();
}

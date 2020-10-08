//
//  CarPlateRecgnize.cpp
//  CarRecgnize
//
//  Created by liuyang on 2020/10/3.
//  Copyright © 2020 liuyang. All rights reserved.
//

#include "CarPlateRecgnize.hpp"

CarPlateRecgnize::CarPlateRecgnize(const char* svm_model){
    pateSobelLocation = new CarSobelPlateLocation();
    pateColorLocation = new CarColorPlateLocation();
   svm = SVM::load(svm_model);
    //参数1的宽-参数2的宽 结果与参数3的余数为0  高也一样
    svmHog = new HOGDescriptor(Size(128,64),Size(16,16),Size(8,8),Size(8,8),3);
}

CarPlateRecgnize::~CarPlateRecgnize(){
    if(!pateSobelLocation){
           delete pateSobelLocation;
           pateSobelLocation=0;
    }
    if(!pateColorLocation){
           delete pateColorLocation;
           pateColorLocation=0;
    }
    //svm->clear();
    svm.release();
}

/**
 识别车牌返回给调用者 1.定位 2 识别
*/
string CarPlateRecgnize::plateRecgnize(Mat src){
    vector<Mat> sobel_plates;
    vector<Mat> color_plates;
    pateSobelLocation->location(src, sobel_plates);
    pateColorLocation->location(src, color_plates);
    vector<Mat> plates;
    //把sobel_plates的内容 全部加入plates向量
    plates.insert(plates.end(),sobel_plates.begin(), sobel_plates.end());
    plates.insert(plates.end(), color_plates.begin(), color_plates.end());
    int index = -1;
    float minScore = FLT_MAX; //float的最大值
    //使用 svm 进行 评测
    for (int i = 0;i< plates.size();++i)
    {
        Mat plate = plates[i];
        //抽取车牌特征 HOG
        Mat gray;
        cvtColor(plate, gray,COLOR_BGR2GRAY);
        //二值化 必须是以单通道进行
        Mat shold;
        threshold(gray, shold, 0, 255, THRESH_OTSU + THRESH_BINARY);
        //提取特征
        Mat features;
        getHogFeatures(svmHog,shold, features);
        //把特征置为一行
        Mat samples = features.reshape(1, 1);
    
        //原始模式
        // svm: 直接告诉你这个数据是属于什么类型.
        // RAW_OUTPUT：让svm 给出一个评分
        /*char name[100];
        sprintf(name,"候选车牌%d",i);
        imshow(name,plate);*/
        
        float score = svm->predict(samples,noArray(), StatModel::Flags::RAW_OUTPUT);
        //printf("评分：%f\n",score);
        if (score < minScore) {
            minScore = score;
            index = i;
        }
        gray.release();
        shold.release();
        features.release();
        samples.release();
    }
    Mat dst;
    if (index >= 0) {
       dst = plates[index].clone();
    }
    //释放
    for (Mat p : plates) {
       p.release();
    }
    //imshow("车牌", dst);
    //识别 ...... ann ：神经网络
    Mat plate_gray;
    cvtColor(dst, plate_gray,COLOR_BGR2GRAY);

    //二值化
    Mat plate_shold;
    threshold(plate_gray, plate_shold,0,255, THRESH_OTSU + THRESH_BINARY);
    //去除车牌上的两个点点 去除跳变次数比较小的干扰点
    clearFixPoint(plate_shold);
   
    vector< vector<Point> > contours;
    //查找轮廓 提取最外层的轮廓  将结果变成点序列放入 集合
    findContours(plate_shold, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    
    vector<Rect> charVec;
    for (vector<Point> point : contours) {
        Rect rect = boundingRect(point);
        //获得候选字符的图片
        Mat p  = plate_shold(rect);
        
        // 进行初步的筛选
        if (verityCharSize(p))
        {
            charVec.push_back(rect);
        }
    }
    //集合中仍然会存在 非字符矩形
    //对集合中的矩形按照x进行一下排序，保证它们是从左到右的顺序
    sort(charVec.begin(), charVec.end(), [] (const Rect& r1, const Rect& r2){
        return r1.x < r2.x;
    });
    
    //汉字比较特殊
    // 如何拿汉字的矩形：获取城市字符的轮廓所在集合的下标 比如湘A ，那么A就是城市字符 代表长沙
    int cityIndex = getCityIndex(charVec);
    plate_gray.release();
    plate_shold.release();
    return "";
}
//提取特征
void CarPlateRecgnize::getHogFeatures(HOGDescriptor* hog, Mat src, Mat& out) {
    //重新定义大小 缩放 提取特征的时候数据需要为  ：CV_32S 有符号的32位数据
    Mat trainImg = Mat(hog->winSize, CV_32S);
    resize(src, trainImg,hog->winSize);
    //计算特征 获得float集合
    vector<float> d;
    hog->compute(trainImg,d, Size(8, 8));

    Mat features(d);
    //特征矩阵
    features.copyTo(out);
    features.release();
    trainImg.release();
}
void CarPlateRecgnize::clearFixPoint(Mat& src){
    //最大改变次数是10
    int maxChange = 10;
    //一个集合统计每一行的跳变次数
    vector<int> c;
    for (size_t i = 0; i < src.rows; i++)
    {
        //记录这一行的改变次数
        int change = 0;
        for (size_t j = 0; j < src.cols - 1; j++)
        {
            //获得像素值
            char p = src.at<char>(i, j);
            //当前的像素点与下一个像素点值是否相同
            if (p != src.at<char>(i, j + 1)) {
                change++;
            }
        }
        c.push_back(change);
    }
    for (size_t i = 0; i < c.size(); i++)
    {
        //取出每一行的改变次数
        int change = c[i];
        //如果小与max ，则可能就是干扰点所在的行
        if (change <= maxChange) {
            //把这一行都抹黑
            for (size_t j = 0; j < src.cols; j++)
            {
                src.at<char>(i, j) = 0;
            }
        }
    }
}

int CarPlateRecgnize::verityCharSize(Mat src) {
    //最理想情况 车牌字符的标准宽高比
    float aspect = 45.0f / 90;
    // 当前获得矩形的真实宽高比
    float realAspect = (float)src.cols / (float)src.rows;
    //最小的字符高
    float minHeight = 10.0f;
    //最大的字符高
    float maxHeight = 35.0f;
    //1、判断高符合范围  2、宽、高比符合范围
    //最大宽、高比 最小宽高比
    float error = 0.7f;
    float maxAspect = aspect + aspect * error;
    float minAspect = aspect - aspect * error;

    if (realAspect  >= minAspect && realAspect <= maxAspect && src.rows >= minHeight && src.rows <= maxHeight)
    {
        return 1;
    }
    return 0;
}

int CarPlateRecgnize::getCityIndex(vector<Rect> src) {
    int cityIndex = 0;
    //循环集合
    for (size_t i = 0; i < src.size(); i++)
    {
        Rect rect = src[i];
        //获得矩形
        //把车牌区域划分为7个字符
        //如果当前获得的矩形 它的中心点 比 1/7 大，比2/7小，那么就是城市的轮廓
        int midX = rect.x + rect.width / 2;
        if (midX < 136 / 7 * 2 && midX > 136 / 7) {
            cityIndex = i;
            break;
        }
    }

    return cityIndex;
}

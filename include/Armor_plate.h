#pragma once
#include <opencv2/opencv.hpp>
#include <vector>


// 灯条描述类
class LightDescriptor {
public:
    float width, length, angle, area;
    cv::Point2f center;

public:
    LightDescriptor() = default;
    LightDescriptor(const cv::RotatedRect& light);
};

// 筛选灯条函数：从轮廓中筛选出符合条件的灯条
std::vector<LightDescriptor> filterLights(const std::vector<std::vector<cv::Point>>& contours);

// 匹配灯条函数：从筛选后的灯条中匹配成对的灯条并绘制
void matchLights(cv::Mat& frame, const std::vector<LightDescriptor>& lightInfos);
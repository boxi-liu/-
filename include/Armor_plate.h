#pragma once
#include <opencv2/opencv.hpp>
#include <vector>

// 灯条类：存储单个灯条的信息
class LightDescriptor {
private:
    cv::RotatedRect rect;  // 灯条旋转矩形
    float angle;           // 灯条角度
    cv::Point2f center;    // 灯条中心坐标

public:
    // 构造函数
    LightDescriptor(const cv::RotatedRect& r);

    // 获取灯条角度
    float getAngle() const;

    // 获取灯条旋转矩形（修改方法名以匹配主函数调用）
    cv::RotatedRect getRotatedRect() const;

    // 获取灯条中心坐标
    cv::Point2f getCenter() const;

    // 计算与另一个灯条的中心距离
    float distanceTo(const LightDescriptor& other) const;
};
    
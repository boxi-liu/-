#include "Armor_plate.h"
#include <cmath>

// 构造函数：初始化灯条属性
LightDescriptor::LightDescriptor(const cv::RotatedRect& r) : rect(r) {
    angle = r.angle;
    center = r.center;
}

// 获取灯条角度
float LightDescriptor::getAngle() const {
    return angle;
}

// 获取灯条旋转矩形（修改方法名以匹配主函数调用）
cv::RotatedRect LightDescriptor::getRotatedRect() const {
    return rect;
}

// 获取灯条中心坐标
cv::Point2f LightDescriptor::getCenter() const {
    return center;
}

// 计算与另一个灯条的欧氏距离
float LightDescriptor::distanceTo(const LightDescriptor& other) const {
    float dx = center.x - other.center.x;
    float dy = center.y - other.center.y;
    return std::sqrt(dx*dx + dy*dy);
}
    
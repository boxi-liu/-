#include<iostream>
#include "Armor_plate.h"
using namespace std;

int main(){
    //获取视频
    cv::VideoCapture video; 
    video.open("../R2.avi");

    //读取每帧
    cv::Mat frame;
    while (video.read(frame)) { 
        if (frame.empty()) break;
   
        //分离红色灯条
        cv::Mat hsv, red_mask;
        cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
        cv::Mat mask1, mask2;
        cv::inRange(hsv, cv::Scalar(0, 150, 100), cv::Scalar(10, 255, 255), mask1);  // 低范围红色
        cv::inRange(hsv, cv::Scalar(160, 150, 100), cv::Scalar(180, 255, 255), mask2); // 高范围红色
        cv::bitwise_or(mask1, mask2, red_mask);    //整合
        cv::GaussianBlur(red_mask, red_mask, cv::Size(3, 3), 0); //高斯模糊
        
        // 存储轮廓的向量（仅最外层）
        vector<vector<cv::Point>> contours;

        // 查找最外层轮廓（无需层级关系）
        cv::findContours(red_mask, contours, cv::noArray(), cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        vector<LightDescriptor> validLights;
        // 筛选符合条件的灯条
        for (auto& contour : contours) {
            if (cv::contourArea(contour) < 10) continue; 

            cv::RotatedRect rotRect = cv::minAreaRect(contour); 
            LightDescriptor lightDesc(rotRect);

            // 角度筛选
            if (abs(lightDesc.getAngle()) < 10 || abs(lightDesc.getAngle() - 90) < 10) { 
                validLights.push_back(lightDesc);
            }
        }

        // 寻找最近的两个灯条并绘制包围矩形
        if (validLights.size() >= 2) {
            int idx1 = 0, idx2 = 1;
            float minDist = cv::norm(validLights[0].getCenter() - validLights[1].getCenter());

            // 遍历所有可能的灯条对，找到距离最近的
            for (size_t i = 0; i < validLights.size(); ++i) {
                for (size_t j = i + 1; j < validLights.size(); ++j) {
                    float dist = cv::norm(validLights[i].getCenter() - validLights[j].getCenter());
                    if (dist < minDist) {
                        minDist = dist;
                        idx1 = i;
                        idx2 = j;
                    }
                }
            }

            // 获取两个灯条的旋转矩形
            cv::RotatedRect rect1 = validLights[idx1].getRotatedRect();
            cv::RotatedRect rect2 = validLights[idx2].getRotatedRect();
            
            // 计算两个灯条长度的中点（假设size.height为长度）
            cv::Point2f center1 = rect1.center;
            cv::Point2f center2 = rect2.center;
            
            // 计算大矩形的长度（两个中点之间的距离）
            float bigRectLength = cv::norm(center1 - center2);
            
            // 计算大矩形的宽度（两个灯条宽度的平均值，假设size.width为宽度）
            float bigRectWidth = (rect1.size.width + rect2.size.width) / 2.0f;
            
            // 计算大矩形的中心（两个灯条中心的中点）
            cv::Point2f bigRectCenter = (center1 + center2) / 2.0f;
            
            // 计算大矩形的旋转角度（与两个灯条中心连线一致）
            float angle = atan2(center2.y - center1.y, center2.x - center1.x) * 180 / CV_PI;
            
            // 创建大矩形
            cv::RotatedRect bigRect(bigRectCenter, cv::Size2f(bigRectLength, bigRectWidth), angle);
            
            // 绘制大矩形
            cv::Point2f vertices[4];
            bigRect.points(vertices);
            for (int j = 0; j < 4; ++j) {
                cv::line(frame, vertices[j], vertices[(j+1)%4], cv::Scalar(0, 0, 255), 2); // 红色大矩形
            }
        }

        
        cv::imshow("video", frame);
        cv::waitKey(16);
    }
 
    video.release();
    cv::destroyAllWindows();
    return 0;

}




#include <iostream>
#include "Armor_plate.h"

int main() {
    cv::VideoCapture video("../R2.avi");
    cv::Mat frame;
    while (video.read(frame)) {
        if (frame.empty()) break;

        // 红色灯条检测（HSV色彩空间）
        cv::Mat hsv, redMask;
        cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
        cv::Mat mask1, mask2;
        cv::inRange(hsv, cv::Scalar(0, 150, 100), cv::Scalar(10, 255, 255), mask1);    // 低范围红色
        cv::inRange(hsv, cv::Scalar(160, 150, 100), cv::Scalar(180, 255, 255), mask2); // 高范围红色
        cv::bitwise_or(mask1, mask2, redMask);
        cv::GaussianBlur(redMask, redMask, cv::Size(3, 3), 0);

        // 提取轮廓
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(redMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        // 筛选灯条
        auto lightInfos = filterLights(contours);

        // 匹配灯条并绘制
        matchLights(frame, lightInfos);

        
        cv::namedWindow("video", cv::WINDOW_FREERATIO);
        cv::imshow("video", frame);
        cv::waitKey(10) ;
    }

    video.release();
    cv::destroyAllWindows();
    return 0;
}
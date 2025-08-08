#include "Armor_plate.h"
// 灯条描述类构造函数
LightDescriptor::LightDescriptor(const cv::RotatedRect& light) {
    width = light.size.width;
    length = light.size.height;
    center = light.center;
    angle = light.angle;
    area = light.size.area();
}

// 筛选灯条实现
std::vector<LightDescriptor> filterLights(const std::vector<std::vector<cv::Point>>& contours) {
    std::vector<LightDescriptor> lightInfos;

    for (const auto& contour : contours) {
        // 筛选面积过小或点数量不足的轮廓
        double area = cv::contourArea(contour);
        if (area < 5 || contour.size() < 5) {
            continue;
        }

        // 椭圆拟合获取旋转矩形
        cv::RotatedRect lightRect = cv::fitEllipse(contour);

        // 筛选长宽比异常的灯条
        if (lightRect.size.width / lightRect.size.height > 4) {
            continue;
        }

        lightInfos.emplace_back(lightRect);
    }

    return lightInfos;
}

// 匹配灯条并绘制实现
void matchLights(cv::Mat& frame, const std::vector<LightDescriptor>& lightInfos) {
    for (size_t i = 0; i < lightInfos.size(); ++i) {
        for (size_t j = i + 1; j < lightInfos.size(); ++j) {
            const LightDescriptor& left = lightInfos[i];
            const LightDescriptor& right = lightInfos[j];

            // 计算匹配条件参数
            float angleGap = abs(left.angle - right.angle);
            float lenGapRatio = abs(left.length - right.length) / std::max(left.length, right.length);
            float distance = cv::norm(left.center - right.center);
            float meanLen = (left.length + right.length) / 2.0f;
            float lenGapRatio2 = abs(left.length - right.length) / meanLen;
            float yGap = abs(left.center.y - right.center.y);
            float yGapRatio = yGap / meanLen;
            float xGap = abs(left.center.x - right.center.x);
            float xGapRatio = xGap / meanLen;
            float ratio = distance / meanLen;

            // 匹配条件筛选
            if (angleGap > 10 ||
                lenGapRatio > 1.0 ||
                lenGapRatio2 > 0.8 ||
                yGapRatio > 0.7 ||
                xGapRatio > 2.2 ||
                xGapRatio < 0.8 ||
                ratio > 3 ||
                ratio < 1) {
                continue;
            }

            // 绘制匹配的灯条矩形
            cv::Point2f center((left.center.x + right.center.x) / 2,
                              (left.center.y + right.center.y) / 2);
            cv::RotatedRect rect(center, cv::Size(distance, meanLen),
                                (left.angle + right.angle) / 2);
            cv::Point2f vertices[4];
            rect.points(vertices);
            for (int k = 0; k < 4; ++k) {
                cv::line(frame, vertices[k], vertices[(k + 1) % 4], cv::Scalar(0, 0, 255), 2.2);
            }
        }
    }
}

前半部分先将图像转化为HSV，将低范围红色和高范围红色掩码合并后，再进行高斯模糊，获得二值化图像
```cpp
cv::Mat hsv, redMask;
        cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
        cv::Mat mask1, mask2;
        cv::inRange(hsv, cv::Scalar(0, 150, 100), cv::Scalar(10, 255, 255), mask1);    // 低范围红色
        cv::inRange(hsv, cv::Scalar(160, 150, 100), cv::Scalar(180, 255, 255), mask2); // 高范围红色
        cv::bitwise_or(mask1, mask2, redMask);
        cv::GaussianBlur(redMask, redMask, cv::Size(3, 3), 0);

```
后续主要操作通过头文件封装的三个函数实现：
1. `LightDescriptor` 构造函数
2. `filterLights` 函数(筛选灯条)
3. `matchLights` 函数（匹配灯条并绘制）
---
下面是封装的三个函数的详细介绍：

### 1. `LightDescriptor` 构造函数
```cpp
LightDescriptor::LightDescriptor(const cv::RotatedRect& light) {
    width = light.size.width;
    length = light.size.height;
    center = light.center;
    angle = light.angle;
    area = light.size.area();
}
```
**功能**：  
将 OpenCV 的旋转矩形(`RotatedRect`)转换为自定义的灯条描述对象，提取关键几何特征：
- `width`：灯条宽度（旋转矩形的短边）
- `length`：灯条高度（旋转矩形的长边）
- `center`：灯条中心点坐标
- `angle`：灯条旋转角度（OpenCV 的椭圆拟合角度体系）
- `area`：灯条区域面积



---

### 2. `filterLights` 函数
```cpp
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
```
**功能**：  
从原始轮廓中筛选符合条件的灯条：
1. **初级过滤**：
   - 剔除面积 < 5 的轮廓（噪声过滤）
   - 剔除点数 < 5 的轮廓（无法拟合椭圆）
2. **椭圆拟合**：用 `fitEllipse` 将轮廓拟合为旋转矩形
3. **几何过滤**：
   - 剔除长宽比 > 4 的轮廓（排除过扁平的形状）

**输出**：  
返回包含所有有效灯条的 `LightDescriptor` 向量

---

### 3. `matchLights` 函数
```cpp
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
```
**功能**：  
通过双层循环遍历所有灯条组合，执行装甲板匹配：
1. **几何约束条件**（核心匹配逻辑）：
   ```cpp
   angleGap > 10          // 角度差阈值
   lenGapRatio > 1.0      // 长度差异比 (绝对比例)
   lenGapRatio2 > 0.8     // 长度差异比 (相对平均长度)
   yGapRatio > 0.7        // 垂直位置偏差
   xGapRatio > 2.2 || < 0.8 // 水平间距异常
   ratio > 3 || < 1       // 灯条间距/平均长度比
   ```
2. **装甲板构建**：
   - 中心点：两灯条中心的均值
   - 宽度：两灯条中心点的距离
   - 高度：两灯条长度的平均值
   - 角度：两灯条角度的平均值
3. **可视化**：
   - 用红色线(0,0,255)绘制装甲板四边形
   - 线宽 2.2 像素


#include <opencv2/opencv.hpp>
#include <windows.h>
#include <highlevelmonitorconfigurationapi.h>
#include <iostream>
#include "AutoBrightness.h"

#pragma comment(lib, "Dxva2.lib")


// 获取图像平均亮度
double getAverageBrightness(const cv::Mat& frame) {
    cv::Mat grayFrame;
    cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);
    cv::Scalar meanScalar = mean(grayFrame);
    return meanScalar[0];
}


void SetBrightness(int brightness) {
    // 构建 PowerShell 命令字符串
    std::string command = "powershell.exe -Command \"(Get-WmiObject -Namespace root/wmi -Class WmiMonitorBrightnessMethods).WmiSetBrightness(1, ";
    command += std::to_string(brightness);
    command += ")\"";

    // SHELLEXECUTEINFO sei;
    // ZeroMemory(&sei, sizeof(sei));
    // sei.cbSize = sizeof(sei);
    // sei.lpVerb = L"open";
    // sei.lpFile = L"cmd.exe";
    // sei.lpParameters = command.c_str();
    // sei.nShow = SW_HIDE;

    // ShellExecuteEx(&sei);


    system(command.c_str());
}

void AutoBrightness() {
    cv::VideoCapture cap; // 打开默认摄像头

    // 禁用自动曝光和自动增益
    cap.set(cv::CAP_PROP_AUTO_EXPOSURE, 0.25); // 0.25表示手动模式
    cap.set(cv::CAP_PROP_EXPOSURE, -2); // 设置曝光值，具体值需要根据摄像头型号调整
    cap.set(cv::CAP_PROP_GAIN, 0); // 设置增益值
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 10);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 10);
    cap.set(cv::CAP_PROP_FPS, 5);

    while (true) {
        cap.open(0);
        if (!cap.isOpened()) {
            std::cerr << "Error: Could not open camera." << std::endl;
            return;
        }
        cv::Mat frame;
        cap >> frame; // 获取摄像头帧
        cap.release();
        if (frame.empty()) {
            std::cerr << "Error: No captured frame." << std::endl;
            break;
        }

        // 计算平均亮度并设置屏幕亮度
        double brightness = getAverageBrightness(frame);
        int screenBrightness = static_cast<int>(brightness * 200 / 255); // 将亮度转换为0-100范围
        SetBrightness(screenBrightness);

        //Sleep(60000);
    }

    cap.release();


    return;
}

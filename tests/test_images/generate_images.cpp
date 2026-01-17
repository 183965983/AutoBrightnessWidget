#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    // Create grayscale test images with different brightness levels
    
    // Dark image (brightness ~25)
    cv::Mat dark_img(480, 640, CV_8UC3, cv::Scalar(25, 25, 25));
    cv::imwrite("dark.jpg", dark_img);
    
    // Medium image (brightness ~128)
    cv::Mat medium_img(480, 640, CV_8UC3, cv::Scalar(128, 128, 128));
    cv::imwrite("medium.jpg", medium_img);
    
    // Bright image (brightness ~230)
    cv::Mat bright_img(480, 640, CV_8UC3, cv::Scalar(230, 230, 230));
    cv::imwrite("bright.jpg", bright_img);
    
    std::cout << "Created test images" << std::endl;
    
    return 0;
}

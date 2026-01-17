#ifndef MOCKCAMERA_H
#define MOCKCAMERA_H

#include <opencv2/opencv.hpp>
#include <string>

class MockCamera {
public:
    MockCamera();
    
    // Load a test image to simulate camera input
    bool loadTestImage(const std::string& imagePath);
    
    // Get the current frame (returns the loaded test image)
    cv::Mat getFrame();
    
    // Check if mock camera is initialized
    bool isOpened() const;
    
private:
    cv::Mat m_testImage;
    bool m_opened;
};

#endif // MOCKCAMERA_H

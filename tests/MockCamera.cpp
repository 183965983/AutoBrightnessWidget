#include "MockCamera.h"
#include <iostream>

MockCamera::MockCamera() : m_opened(false) {
}

bool MockCamera::loadTestImage(const std::string& imagePath) {
    m_testImage = cv::imread(imagePath, cv::IMREAD_COLOR);
    
    if (m_testImage.empty()) {
        std::cerr << "Failed to load test image: " << imagePath << std::endl;
        m_opened = false;
        return false;
    }
    
    m_opened = true;
    return true;
}

cv::Mat MockCamera::getFrame() {
    return m_testImage.clone();
}

bool MockCamera::isOpened() const {
    return m_opened;
}

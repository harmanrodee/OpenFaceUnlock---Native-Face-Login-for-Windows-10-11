#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <string>
#include <vector>

struct FaceDetection
{
    cv::Rect faceRect;
    cv::Point2f leftEye;
    cv::Point2f rightEye;
    cv::Point2f nose;
    cv::Point2f leftMouth;
    cv::Point2f rightMouth;
    float score;
};

bool loadYuNetModel(const std::string modelPath, cv::Size inputSize = cv::Size(640, 480));

std::vector<FaceDetection> detectFaces(const cv::Mat& frame);
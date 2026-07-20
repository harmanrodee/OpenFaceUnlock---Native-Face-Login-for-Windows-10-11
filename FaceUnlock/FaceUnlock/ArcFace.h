#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

bool loadArcFaceModel(const std::string& model_path);

void testONNX();

std::vector<float> getFaceEmbedding(const cv::Mat& face, cv::Size size = cv::Size(112, 112));
#include "YuNet.h"
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <string>
#include <vector>
#include <iostream>

cv::Ptr<cv::FaceDetectorYN> detector = nullptr;

bool loadYuNetModel(const std::string modelPath, cv::Size inputSize) {
	try {
		detector = cv::FaceDetectorYN::create(modelPath, "", inputSize);
		std::cout << "YuNet Model Loaded \n";
		return true;
	}
	catch (const cv::Exception& e) {
		std::cout << "YuNet load error : " << e.what() << std::endl;
		return false;
	}
}

std::vector<FaceDetection> detectFaces(const cv::Mat& frame) {
	// Downscale image to a fixed width of 480 for fast face detection
	const int targetWidth = 480;
	int targetHeight = (int)(frame.rows * ((double)targetWidth / frame.cols));
	
	cv::Mat smallFrame;
	cv::resize(frame, smallFrame, cv::Size(targetWidth, targetHeight));

	detector->setInputSize(smallFrame.size());
	cv::Mat detections;
	detector->detect(smallFrame, detections);

	// Calculate scaling ratios to map coordinates back to the original size
	double scaleX = (double)frame.cols / targetWidth;
	double scaleY = (double)frame.rows / targetHeight;

	std::vector<FaceDetection> faces;
	for (int i = 0; i < detections.rows; i++) {
		FaceDetection face;
		face.faceRect = cv::Rect(
			(int)(detections.at<float>(i, 0) * scaleX),
			(int)(detections.at<float>(i, 1) * scaleY),
			(int)(detections.at<float>(i, 2) * scaleX),
			(int)(detections.at<float>(i, 3) * scaleY)
		);
		face.rightEye = cv::Point2f(
			detections.at<float>(i, 4) * scaleX,
			detections.at<float>(i, 5) * scaleY
		);

		face.leftEye = cv::Point2f(
			detections.at<float>(i, 6) * scaleX,
			detections.at<float>(i, 7) * scaleY
		);

		face.nose = cv::Point2f(
			detections.at<float>(i, 8) * scaleX,
			detections.at<float>(i, 9) * scaleY
		);

		face.rightMouth = cv::Point2f(
			detections.at<float>(i, 10) * scaleX,
			detections.at<float>(i, 11) * scaleY
		);

		face.leftMouth = cv::Point2f(
			detections.at<float>(i, 12) * scaleX,
			detections.at<float>(i, 13) * scaleY
		);

		face.score = detections.at<float>(i, 14);

		faces.push_back(face);
	}
	return faces;
}

#include "ArcFace.h"
#include <iostream>
#include <windows.h>
#undef min
#undef max
#include <onnxruntime_cxx_api.h>
#include <thread>

Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "ArcFace");
Ort::Session* session = nullptr;
unsigned int threads = std::thread::hardware_concurrency();

void testONNX() {
	Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "test");
	std::cout << Ort::GetVersionString() << std::endl;
	std::cout << "Onnx Working" << std::endl;
}

bool FileExists(const std::wstring& path) {
    return GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES;
}

bool loadArcFaceModel(const std::string& model_path) {
	try {
		std::wstring wide_path(model_path.begin(), model_path.end());
		Ort::SessionOptions session_options;
        std::wstring opt_path = wide_path + L".opt.ort";
        if (threads != 0) {
		    session_options.SetIntraOpNumThreads(std::max(1, std::min((int)threads/2, 4)));
        }
        else {
            session_options.SetIntraOpNumThreads(1);
        }
        if (FileExists(opt_path)) {
            session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_DISABLE_ALL);
            session = new Ort::Session(env, opt_path.c_str(), session_options);
        }
        else {
		    session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
            session_options.SetOptimizedModelFilePath(opt_path.c_str());
		    session = new Ort::Session(env, wide_path.c_str(), session_options);
        }
		std::cout << "ArcFace Model Loaded" << std::endl;
		return true;
	}
	catch(const std::exception& e){
		std::cout << "Load error (ArcFace) : " << e.what() << std::endl;
		return false;
	}
}

static std::vector<float> preprocessFace(const cv::Mat& face, cv::Size size) {
    cv::Mat frame;
    face.copyTo(frame);
    cv::resize(frame, frame, size);
    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    frame.convertTo(frame, CV_32F, 1.0 / 255.0);
    std::vector<float> inputTensorValues(1 * 3 * 112 * 112);

    int idx = 0;
    for (int c = 0; c < 3; c++) {
        for (int y = 0; y < 112; y++) {
            for (int x = 0; x < 112; x++) {
                inputTensorValues[idx++] = frame.at<cv::Vec3f>(y, x)[c];
            }
        }
    }

    return inputTensorValues;
}

std::vector<float> getFaceEmbedding(const cv::Mat& face, cv::Size size) {
    std::vector<float> inputTensorValues = preprocessFace(face, size);
    std::vector<int64_t> inputShape = { 1, 3, 112, 112 };
    Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value inputTensor = Ort::Value::CreateTensor<float>(memoryInfo, inputTensorValues.data(), inputTensorValues.size(), inputShape.data(), inputShape.size());
    std::vector<float> ts;
    const char* inputNames[] = { "input.1" };
    const char* outputNames[] = { "516" };
    std::vector<Ort::Value> outputTensor = session->Run(
        Ort::RunOptions{ nullptr },
        inputNames,
        &inputTensor,
        1,
        outputNames,
        1
    );
    float* outputData = outputTensor[0].GetTensorMutableData<float>();
    std::vector<float> embedding(outputData, outputData + 512);
    
    return embedding;
}
#include "ArcFace.h"
#include "EmbeddingStorage.h"
#include "YuNet.h"
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <iostream>
#include <string>
#include <windows.h>
#undef min
#undef max
#include <fstream>
#include <ShlObj.h>
#include <chrono>
#include <thread>
#include <map>
#define MAX_CREDENTIAL_BLOB_SIZE 4096
#pragma comment(lib, "Shell32.lib")

static const float THRESHOLD_SPOOF = 0.90f;
static const float THRESHOLD_SIMILARITY = 0.51f;
static const int MIN_CONSEC_SIMILAR_FRAMES = 5;
static const int MIN_CONSEC_SIMILAR_FRAMES_REGISTER = 60;
static const std::string arcface_model_path("w600k_mbf.onnx");
static const std::string yunet_model_path("face_detection_yunet_2023mar.onnx");
static bool is_run_as_register = false;
static bool is_debug = false;
static int frame_count = 1;
static int max_frame_count = 150;
static std::string face_type = "main";
static std::vector <std::vector<float>> saved_embeddings;
static std::vector<BYTE> Credbytes;
static bool isWarmedup = false;

enum FaceUnlockState
{
	STATE_IDLE = 0,
	STATE_STARTING = 1,
	STATE_LOADING_MODELS = 2,
	STATE_OPENING_CAMERA = 3,
	STATE_DETECTING_FACE = 4,
	STATE_RECOGNIZING_FACE = 5,
	STATE_AUTH_SUCCESS = 6,
	STATE_AUTH_FAILED = 7,
	STATE_ERROR_CAMERA = 8,
	STATE_ERROR_FEED = 9,
	STATE_ERROR_MODEL = 10,
	STATE_ERROR_EMBEDDING = 11,
	STATE_ERROR_CRED = 12
};


struct AuthPacket {
	int state = 0;
	bool authenticated;
	DWORD credentialBlobSize;
	BYTE credentialBlob[MAX_CREDENTIAL_BLOB_SIZE];
};

static bool sendAuthPacket(int state = 0, bool authenticated = false)
{
	if (is_run_as_register) {
		return false;
	}
	std::vector<BYTE> blob;

	HANDLE hPipe = CreateFile(
		L"\\\\.\\pipe\\FaceUnlockAuth",
		GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
	);

	if (hPipe == INVALID_HANDLE_VALUE) {
		if (is_debug) std::cout << "Failed to connect to pipe\n";
		return false;
	}

	AuthPacket packet{};
	packet.state = state;
	packet.authenticated = authenticated;
	if (authenticated) {
		if (!ReadCredFile(blob)) {
			if (is_debug) std::cout << "Failed to load credentials.\n";
			packet.state = STATE_ERROR_CRED;
		}
		if (packet.state != STATE_ERROR_CRED) {
			packet.credentialBlobSize = (DWORD)blob.size();
			memcpy(packet.credentialBlob, blob.data(), blob.size());
		}
	}

	DWORD bytesWritten = 0;

	BOOL success = WriteFile(
		hPipe,
		&packet,
		sizeof(packet),
		&bytesWritten,
		NULL
	);

	if (!success)
	{
		if (is_debug) std::cout << "WriteFile Error = " << GetLastError() << "\n";
	}
	CloseHandle(hPipe);
	return success;
}

using Clock = std::chrono::high_resolution_clock;

static void PrintTime(const std::string& name, Clock::time_point start)
{
	auto end = Clock::now();

	if (is_debug) {
		std::cout << name << " : "
			<< std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
			<< " ms" << std::endl;
	}
}

static FaceDetection getBiggestFace(std::vector<FaceDetection> faces) {
	FaceDetection biggest_face = faces[0];
	for (int i = 0; i < faces.size(); i++) {
		if (biggest_face.faceRect.area() < faces[i].faceRect.area()) {
			biggest_face = faces[i];
		}
	}
	return biggest_face;
}

static cv::Mat rotateFrame(const cv::Mat& frame,const cv::Point& center, float angle) {
	cv::Mat rotatedFrame;
	cv::warpAffine(frame, rotatedFrame, cv::getRotationMatrix2D(center, angle, 1), frame.size());
	return rotatedFrame;
}

static std::vector<cv::Mat> alignFace(
	const cv::Mat& frame,
	const FaceDetection& face)
{
	std::vector<cv::Mat> processedFaces{};
	cv::Mat framePreviewFace = frame.clone();
	float dx = face.leftEye.x - face.rightEye.x;
	float dy = face.leftEye.y - face.rightEye.y;

	float faceAngle =
		atan2(dy, dx) * 180.0f / CV_PI;

	cv::Point2f eyeCenter(
		(face.leftEye.x + face.rightEye.x) * 0.5f,
		(face.leftEye.y + face.rightEye.y) * 0.5f
	);

	cv::Mat frameRotated = rotateFrame(
		frame,
		eyeCenter,
		faceAngle
	);

	int size = std::max(
		face.faceRect.width,
		face.faceRect.height
	);

	size = static_cast<int>(size * 1.35f);

	int centerX =
		face.faceRect.x + face.faceRect.width / 2;

	int centerY =
		face.faceRect.y + face.faceRect.height / 2;

	// Define raw rect for the aligned face (which is fed to ArcFace)
	cv::Rect rawFaceRect(
		centerX - size / 2,
		centerY - size / 2,
		size,
		size
	);
	rawFaceRect.x = std::max(0, rawFaceRect.x);
	rawFaceRect.y = std::max(0, rawFaceRect.y);
	rawFaceRect.width = std::min(rawFaceRect.width, frameRotated.cols - rawFaceRect.x);
	rawFaceRect.height = std::min(rawFaceRect.height, frameRotated.rows - rawFaceRect.y);

	// Define stabilized rect parameters
	static double smoothedCenterX = 0.0;
	static double smoothedCenterY = 0.0;
	static double smoothedSize = 0.0;
	static bool hasHistory = false;
	static int lastFrame = -1;

	// Reset history if face jumps too far (e.g. rapid movement/different person) or if frames were skipped
	double dist = std::sqrt((centerX - smoothedCenterX) * (centerX - smoothedCenterX) + 
	                        (centerY - smoothedCenterY) * (centerY - smoothedCenterY));
	if (!hasHistory || (frame_count - lastFrame) > 2 || dist > size * 0.5) {
		smoothedCenterX = centerX;
		smoothedCenterY = centerY;
		smoothedSize = size;
		hasHistory = true;
	} else {
		double alpha = 0.15; // Smoothing factor (lower is smoother/slower, higher is more responsive)
		smoothedCenterX = alpha * centerX + (1.0 - alpha) * smoothedCenterX;
		smoothedCenterY = alpha * centerY + (1.0 - alpha) * smoothedCenterY;
		smoothedSize = alpha * size + (1.0 - alpha) * smoothedSize;
	}
	lastFrame = frame_count;

	cv::Rect stableFaceRect(
		(int)(smoothedCenterX - smoothedSize / 2),
		(int)(smoothedCenterY - smoothedSize / 2),
		(int)smoothedSize,
		(int)smoothedSize
	);
	stableFaceRect.x = std::max(0, stableFaceRect.x);
	stableFaceRect.y = std::max(0, stableFaceRect.y);
	stableFaceRect.width = std::min(stableFaceRect.width, framePreviewFace.cols - stableFaceRect.x);
	stableFaceRect.height = std::min(stableFaceRect.height, framePreviewFace.rows - stableFaceRect.y);

	cv::Mat croppedFace =
		framePreviewFace(stableFaceRect).clone();
	processedFaces.push_back(croppedFace);

	cv::Mat alignedFace = frameRotated(rawFaceRect).clone();
	processedFaces.push_back(alignedFace);

	return processedFaces;
}

static bool CheckSimilarity(const std::vector<std::vector<float>>& reference_embeddings, const std::vector<float>& compare_embedding, int MINIMUM_CONSEC_FRAMES) {
	float score = 0;
	static int consecutiveSimilarFrames = 0;
	if (is_debug) std::cout << "refrence embeddings : " << reference_embeddings.size() << std::endl;
	//i transverse the vector of vector of float
	for (int i = 0; i < reference_embeddings.size(); i++) {

		float dot = 0.0f;
		float normA = 0.0f;
		float normB = 0.0f;

		//j transvserse the vector of float
		for (int j = 0; j < reference_embeddings[i].size(); j++) {
			dot += reference_embeddings[i][j] * compare_embedding[j];
			normA += reference_embeddings[i][j] * reference_embeddings[i][j];
			normB += compare_embedding[j] * compare_embedding[j];
		}

		float calculated_score = dot / (sqrt(normA) * sqrt(normB));
		if (is_debug) std::cout << "Score : " << calculated_score << std::endl;
		if (calculated_score > score) {
			score = calculated_score;
		}
	}
	if (score >= THRESHOLD_SIMILARITY) {
		consecutiveSimilarFrames++;
		if (is_debug) std::cout << "Consecutive Similarity found : " << consecutiveSimilarFrames << std::endl;
	}
	else {
		consecutiveSimilarFrames = 0;
	}

	if (consecutiveSimilarFrames >= MINIMUM_CONSEC_FRAMES)
	{
		if (is_debug) std::cout << "Face Recogonized !" << std::endl;
		return true;
	}

	return false;
}

static bool registerFace(cv::Mat face_mat, std::string path) {
	int input = cv::waitKey(1);

	static bool hasSavedEmbedding = false;

	std::vector<float> currentEmbedding = getFaceEmbedding(face_mat);

	if (!hasSavedEmbedding) {
		saved_embeddings.push_back(currentEmbedding);
		hasSavedEmbedding = true;
		return false;
	}

	bool isStable = CheckSimilarity(saved_embeddings, currentEmbedding, MIN_CONSEC_SIMILAR_FRAMES_REGISTER);
	saved_embeddings[0] = currentEmbedding;

	if (isStable) {
		if (!saveEmbedding(currentEmbedding, face_type)) {
			if (is_debug) std::cout << "Failed to save face embedding automatically" << std::endl;
			return false;
		}
		else {
			if (is_debug) std::cout << "Face Registered Automatically!" << std::endl;

			cv::destroyWindow("FaceUnlockRegister");

			hasSavedEmbedding = false;
			saved_embeddings.clear();

			return true;
		}
	}

	return false;
}

static bool checkArguments(int argc, char* argv[])
{
	std::map<std::string, std::string> args;

	for (int i = 1; i < argc; i += 2)
	{
		if (i + 1 < argc)
		{
			args[argv[i]] = argv[i + 1];
		}
	}

	if (args.count("--mode"))
	{
		if (args["--mode"] == "register")
		{
			if (args.count("--user") && args.count("--pass") && args.count("--type")) {
				std::string str_user = args["--user"];
				std::string str_pass = args["--pass"];
				if (args["--type"] == "main") {
					face_type = "main";
				}
				else if (args["--type"] == "alt") {
					face_type = "alt";
				}
				else {
					return false;
				}

				std::wstring wstr_user(str_user.begin(), str_user.end());
				std::wstring wstr_pass(str_pass.begin(), str_pass.end());
				
				HANDLE tokenHandle = nullptr;
				BOOL logonSuccess = LogonUserW(
					wstr_user.c_str(),
					NULL,
					wstr_pass.c_str(),
					LOGON32_LOGON_NETWORK,
					LOGON32_PROVIDER_DEFAULT,
					&tokenHandle
				);
				if (!logonSuccess) {
					if (is_debug) std::cout << "Logon failed. Error: "
						<< GetLastError()
						<< std::endl;
					return false;
				}
				CloseHandle(tokenHandle);
				is_run_as_register = true;
				max_frame_count = 300;
				return true;
			}
			else {
				if (is_debug) std::cout << "No user and pass arguments"
					<< std::endl;
				return false;
			}
		}else if (args["--mode"] == "login")
		{
			is_run_as_register = false;
			max_frame_count = 150;
			return true;
		}else if (args["--mode"] == "warmup")
		{
			bool arcfaceOk = false;
			bool yunetOk = false;
			//This mode helps the windows preload models and dll files from disk to ram for faster opening during startup
			std::thread arcFaceThread([&]() { arcfaceOk = loadArcFaceModel(arcface_model_path); });
			std::thread yunetThread([&]() { yunetOk = loadYuNetModel(yunet_model_path); });

			arcFaceThread.join();
			yunetThread.join();
			if (!arcfaceOk || !yunetOk) {
				return false;
			}
			isWarmedup = true;
			is_run_as_register = false;
			max_frame_count = 150;
			/*exit(0);*/
			return true;
		}
		else {
			return false;
		}
	}
}

int main(int argc, char *argv[]) {
	// --- BATTERY OPTIMIZATION: Disable Process Power Throttling ---
	PROCESS_POWER_THROTTLING_STATE PowerThrottling{};
	PowerThrottling.Version = PROCESS_POWER_THROTTLING_CURRENT_VERSION;
	PowerThrottling.ControlMask = PROCESS_POWER_THROTTLING_EXECUTION_SPEED;
	PowerThrottling.StateMask = 0; // 0 = Turn OFF throttling, grant full CPU speed
	SetProcessInformation(GetCurrentProcess(), ProcessPowerThrottling, &PowerThrottling, sizeof(PowerThrottling));
	// --------------------------------------------------------------

	auto timer = Clock::now();
	//checking arguments
	if (!checkArguments(argc, argv)) {
		std::cout << "checkargsfail\n";
		return 1;
	}
	sendAuthPacket(STATE_STARTING);
	sendAuthPacket(STATE_OPENING_CAMERA);

	timer = Clock::now();
	cv::VideoCapture cap;
	bool cameraOk = false;
	bool arcfaceOk = false;
	bool yunetOk = false;
	bool minifasnetOk = false;

	//Multithreading Loading Tasks
	std::thread cameraThread([&]() {
		for (int i = 0; i < 10; i++) {
			cap = cv::VideoCapture(0);
			if (cap.isOpened()) {
				cv::Mat testframe;
				cap >> testframe;
				if (testframe.empty()) {
					if (is_debug) std::cout << "Unable to Access Frame" << std::endl;
					return 1;
				}
				cameraOk = true;
				break;
			}
			Sleep(50);
		}
	});

	sendAuthPacket(STATE_LOADING_MODELS);


	std::thread arcFaceThread([&]() {
		arcfaceOk = loadArcFaceModel(arcface_model_path);
		});

	std::thread yunetThread([&]() {
		yunetOk = loadYuNetModel(yunet_model_path);
		});
	arcFaceThread.join();
	yunetThread.join();
	cameraThread.join();

	//load models
	if (!arcfaceOk || !yunetOk) {
		sendAuthPacket(STATE_ERROR_MODEL);
		return 1;
	}

	
	if (!cameraOk) {
		if (is_debug) std::cout << "Unable to open camera" << std::endl;
		sendAuthPacket(STATE_ERROR_CAMERA);
		return 1;
	}
	cap.set(cv::CAP_PROP_FPS, 30);

	PrintTime("Camera Setup", timer);


	//load faceEmbedding
	if (!is_run_as_register) {
		
		if (!loadEmbedding(saved_embeddings)) {
			sendAuthPacket(STATE_ERROR_EMBEDDING);
			return 1;
		}
	}

	//initializing variables
	cv::Mat frame;
	cv::Mat frame_ml;
	cv::Mat frame_face;
	cv::Mat frameRegisterPreview;
	std::vector<float> foundEmbedding;
	bool isRecogonized = false;
	sendAuthPacket(STATE_RECOGNIZING_FACE);

	while (frame_count <= max_frame_count) {
		if (is_debug) std::cout << "Frame : " << frame_count << std::endl;

		// Check if parent process wrote a cancellation command to standard input
		HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
		DWORD bytesAvailable = 0;
		if (PeekNamedPipe(hStdin, NULL, 0, NULL, &bytesAvailable, NULL) && bytesAvailable > 0) {
			std::string cmd;
			std::getline(std::cin, cmd);
			if (cmd == "exit") {
				cap.release();
				cv::destroyAllWindows();
				return 0;
			}
		}

		timer = Clock::now();
		cap >> frame;

		if (frame.empty()) {
			if (is_debug) std::cout << "Unable to Access Frame" << std::endl;
			return 1;
		}

		PrintTime("Frame cap time : ", timer);
		cv::flip(frame, frame, 1);
		
		// Center-crop the rectangular frame into a square to match the square preview window aspect ratio
		int width = frame.cols;
		int height = frame.rows;
		int size = std::min(width, height);
		int x = (width - size) / 2;
		int y = (height - size) / 2;
		cv::Rect centerRect(x, y, size, size);
		frameRegisterPreview = frame(centerRect).clone();

		std::vector<FaceDetection> facesYuNet = detectFaces(frame);
		if (is_debug) std::cout << "Faces by YuNet " << facesYuNet.size() << std::endl;
		frame_ml = frame.clone();
		if (is_debug) {
			for (int i = 0; i < facesYuNet.size(); i++)
			{
				cv::rectangle(
					frame,
					facesYuNet[i].faceRect,
					cv::Scalar(0, 255, 0),
					2
				);

				cv::circle(frame, facesYuNet[i].leftEye, 3, cv::Scalar(255, 0, 0), -1);
				cv::circle(frame, facesYuNet[i].rightEye, 3, cv::Scalar(255, 0, 0), -1);

				cv::circle(frame, facesYuNet[i].nose, 3, cv::Scalar(0, 0, 255), -1);

				cv::circle(frame, facesYuNet[i].leftMouth, 3, cv::Scalar(0, 255, 255), -1);
				cv::circle(frame, facesYuNet[i].rightMouth, 3, cv::Scalar(0, 255, 255), -1);
			}
		}

		//performing actual face recogonization and drawing face landmarks on the frame
		if (facesYuNet.size() > 0) {
			FaceDetection face = getBiggestFace(facesYuNet);

			std::vector<cv::Mat> processedFaceFrames = alignFace(frame_ml, face);
			frameRegisterPreview = processedFaceFrames[0];
			frame_face = processedFaceFrames[1];

			if (is_run_as_register) {
				bool registered = registerFace(frame_face, "face.dat");

				if (registered) {
					cap.release();
					cv::destroyAllWindows();
					return 100;
				}
			}
			else {
				foundEmbedding = getFaceEmbedding(frame_face);
				isRecogonized = CheckSimilarity(saved_embeddings, foundEmbedding, MIN_CONSEC_SIMILAR_FRAMES);
				if (isRecogonized) {
					if (sendAuthPacket(STATE_AUTH_SUCCESS, isRecogonized)) {
						if (is_debug) std::cout << "Auth packet sent\n";
					}
					break;
				}

				if (is_debug) {
					cv::imshow("Face", frame_face);
				}
			}
		}

		if (is_debug) {
			cv::imshow("Feed", frame);
		}
		if (is_debug || is_run_as_register) {
			cv::namedWindow("FaceUnlockRegister", cv::WINDOW_NORMAL);
			cv::imshow("FaceUnlockRegister", frameRegisterPreview);
		}
		cv::waitKey(1);
		frame_count++;
	}
	
	if (!isRecogonized && !is_run_as_register) {
		if (is_debug) std::cout << "Face Not Recogonized ! \n";
		sendAuthPacket(STATE_AUTH_FAILED);
	}
	cap.release();
	cv::destroyAllWindows();

	return 0;
}
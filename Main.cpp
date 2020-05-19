#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <SFML/System.hpp>
#include <SFML/Main.hpp>
#include <SFML/Window.hpp>



template<typename T>void print(const T& text)
{
	std::cout << text << std::endl;
}

void get_video(const char* filename, cv::VideoCapture& video)
{
	if (!video.open(filename))
	{
		std::cout << "Error opening " << filename << "." << std::endl;
		exit(0);
	}
}

int get_number_of_frames( cv::VideoCapture video)
{
	int frames = 0;
	while (true)
	{
		cv::Mat frame;
		bool grab = video.retrieve(frame);
		if (grab) frames++;
		else break;
	}
	return frames;
}

void display_video(int& wait_for, int frames, cv::VideoCapture input_video)
{
	if (!input_video.grab()) std::cout << "Error with display_video." << std::endl;
	while (0 < frames--)
	{
		cv::Mat frame;		
		input_video.read(frame);
		cv::imshow("display_video", frame);
		cv::waitKey(wait_for);
	}
	cv::destroyAllWindows();
}

void display_frames(int& wait_for, const std::vector<cv::Mat>& input_frames)
{
	for (size_t t = 0; t < input_frames.size(); t++)
	{
		std::cout << "frame number = " << t << std::endl;
		cv::imshow("display_frames", input_frames[t]);
		cv::waitKey(wait_for);
	}
	cv::destroyAllWindows();
}

void frame_video(std::vector<cv::Mat>& video_frames, cv::VideoCapture input)
{
	while (true)
	{
		cv::Mat frame;
		bool end_of_input = input.read(frame);
		if (!end_of_input) break;
		video_frames.emplace_back(frame);
	}
}

void convert_to_gray(const std::vector<cv::Mat>& input, std::vector<cv::Mat>& output)
{
	for (size_t i = 0; i < input.size(); i++)
	{
		cv::Mat dst;
		cv::cvtColor(input[0], dst, cv::COLOR_BGR2GRAY);
		output.emplace_back(dst);
	}
}

void convert_to_gray(const cv::Mat& input, cv::Mat& output)
{
	cv::cvtColor(input, output, cv::COLOR_BGR2GRAY);
}


int main()
{
	

	return 0;

	cv::VideoCapture video;

	get_video("video_1.mp4", video); //loading video from file
	int frames_number = get_number_of_frames(video); // number of frames in video

	std::vector<cv::Mat> video_frames; // vector holding frames
	video_frames.reserve(frames_number);

	frame_video(video_frames, video); // cunstruct video with frames

	int img_width = video_frames[0].size().width, img_height = video_frames[0].size().height; // getting img size
	int roi_width = 150, roi_height = 150; // setting roi size (must be input from mouse)

	int Ax = img_width/2 - roi_width/2, Ay = img_height/2 - roi_height/2, 
		Bx = img_width / 2 + roi_width / 2, By = img_height / 2 + roi_height / 2; // this might be useless if got input from mouse



	cv::Mat roi = video_frames[0](cv::Rect(Ax, Ay, roi_width, roi_height)); // gets ROI from img frame

	cv::Mat roi_gray;
	convert_to_gray(roi, roi_gray); // sets roi img to grey scale

	std::vector<cv::Mat> gray_frames;
	gray_frames.reserve(video_frames.size()); // converts video frames to grey scale

	convert_to_gray(video_frames, gray_frames);
	
}
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>




template<typename T>void print(const T& text)
{
	std::cout << text << std::endl;
}

static bool mouse_pressed = false;
static cv::Point A;
static cv::Point B;

int roi_width, roi_height;

cv::Scalar red(255, 0, 0);
cv::Scalar green(0, 255, 0);
cv::Scalar blue(0, 0, 255);
cv::Scalar black(0, 0, 0);


void get_video(std::string& filename, cv::VideoCapture& video)
{
	filename = filename + ".mp4";
	if (!video.open(filename))
	{
		std::cout << "Error opening " << filename << "." << std::endl;
		exit(0);
	}
}

int get_number_of_frames(cv::VideoCapture video)
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

void display_frames(int wait_for, const std::vector<cv::Mat>& input_frames)
{	
	for (size_t t = 0; t < input_frames.size(); t++)
	{		
		cv::imshow("Display frames", input_frames[t]);
		cv::waitKey(wait_for);
	}
	cv::waitKey(0);
	cv::destroyAllWindows();
}

void frame_video(std::vector<cv::Mat>& video_frames, cv::VideoCapture input)
{
	bool img_to_big = false;
	cv::Mat frame;
	input.read(frame);
	cv::Size frame_size((int)(frame.cols / 3), (int)(frame.rows / 3));
	if (frame.cols > 960 || frame.rows > 540) img_to_big = true;
	while (true)
	{
		
		bool end_of_input = input.read(frame);		
		if (!end_of_input) break;
		if (img_to_big) cv::resize(frame, frame, frame_size);
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

void convert_frames_to_gray(const std::vector<cv::Mat>& src, std::vector<cv::Mat>& dst)
{
	for (size_t i = 0; i < src.size(); i++)
	{
		cv::Mat gray;
		cv::cvtColor(src[i], gray, cv::COLOR_BGR2GRAY);
		dst.emplace_back(gray);
	}
}

static void onMouse(int event, int x, int y, int, void*)
{	
	if (event != cv::EVENT_LBUTTONDOWN && event != cv::EVENT_LBUTTONUP && !mouse_pressed) return;

	if (event == cv::EVENT_LBUTTONDOWN)
	{
		A = cv::Point(x, y);		
		mouse_pressed = true;	
	}
	
	if (event == cv::EVENT_MOUSEMOVE && mouse_pressed) B = cv::Point(x, y);

	if (event == cv::EVENT_LBUTTONUP) mouse_pressed = false;
}

void click_to_crop(cv::Mat& image)
{
	cv::Mat img;
	img = image.clone();
	bool roi_set = false;



	cv::imshow("Click_to_crop", img);
	cv::Rect roi;
	while (true)
	{
		char finish = cv::waitKey(50);
		cv::setMouseCallback("Click_to_crop", onMouse, nullptr);
		cv::imshow("Click_to_crop", img);

		if (mouse_pressed)
		{
			img = image.clone();
			roi = cv::Rect(A, B);
			cv::rectangle(img, roi, green, 1, 8, 0);
			roi_set = true;
		}

		if (!mouse_pressed)
		{
			if (roi_set)
			{
				roi_set = false;
				cv::rectangle(img, roi, green, 1, 8, 0);
			}
			if (finish == 'r') img = image.clone();
		}

		if (finish == '\r') break;

	}
	cv::destroyWindow("Click_to_crop");
}

static int threshold = 50, max_threshold = 250;
int method = cv::THRESH_BINARY_INV;

static void on_trackbar(int thresh, void*)
{
	threshold = thresh;
}

void threshold_image(cv::Mat& src, cv::Mat& dst)
{
	cv::Mat thresh_img = src.clone();
	
	cv::threshold(src, thresh_img, threshold, 255, method);
	char finish;
	cv::namedWindow("Trackbar", cv::WINDOW_NORMAL);
	while (true)
	{
		cv::imshow("Set threshold", thresh_img);
		cv::createTrackbar("Threshold", "Trackbar", &threshold, max_threshold, on_trackbar);
		cv::threshold(src, thresh_img, threshold, 255, method);
		finish = cv::waitKey(300);
		if (finish == '\r') break;
	}
	dst = thresh_img;
}

void threshold_frames(std::vector<cv::Mat>& src_frames, std::vector<cv::Mat>& dst_frames)
{
	for (size_t i = 0; i < src_frames.size(); i++)
	{
		cv::Mat threshed_img;
		cv::threshold(src_frames[i], threshed_img, threshold, 255, method);
		dst_frames.emplace_back(threshed_img);
	}
}

void find_picture_center(const cv::Mat& roi_gray, std::vector<cv::Mat>& video_frames, std::vector<cv::Point>& center_points)
{
	print("\nFinding centers in progress...");
	for (size_t i = 0; i < video_frames.size(); i++)
	{
		cv::Point maxLoc(0,0);
		cv::Mat eval_frame;
		cv::matchTemplate(video_frames[i], roi_gray, eval_frame, cv::TM_CCOEFF_NORMED);
		cv::minMaxLoc(eval_frame, nullptr, nullptr, nullptr, &maxLoc);
		center_points.emplace_back(maxLoc);
	}
	
}

static void calculate_offset(cv::Point img_center , std::vector<cv::Point>& center_points, std::vector<cv::Point>& offset)
{
	for (size_t i = 0; i < center_points.size(); i++)
	{
		cv::Point new_point;
		new_point.y = (img_center.y - center_points[i].y);
		new_point.x = (img_center.x - center_points[i].x);
		offset.emplace_back(new_point);
	}
}

cv::Mat offsetImageWithPadding(cv::Mat& originalImage, int offsetX, int offsetY) 
{

	auto padded = cv::Mat(originalImage.rows + 2*abs(offsetX), originalImage.cols + 2 * abs(offsetY), CV_8UC3, black);

	originalImage.copyTo(padded(cv::Rect(abs(offsetX), abs(offsetY), originalImage.cols, originalImage.rows)));

	return cv::Mat(padded, cv::Rect(abs(offsetX) + offsetX, abs(offsetY) + offsetY, originalImage.cols, originalImage.rows));
}

cv::Mat translateImg(cv::Mat& img, int offsetx, int offsety) 
{
	cv::Mat trans_mat = (cv::Mat_<double>(2, 3) << 1, 0, offsetx, 0, 1, offsety);
	cv::warpAffine(img, img, trans_mat, img.size());
	return img;
}


void center_pictures(std::vector<cv::Mat>& src_frames, std::vector<cv::Mat>& dst_frames, std::vector<cv::Point>& center_points)
{
	print("\nCentering images in progress...");
	std::vector<cv::Point> offsets;
	offsets.reserve(center_points.size());
	calculate_offset(cv::Point((int)(src_frames[0].cols / 2), (int)(src_frames[0].rows / 2)), center_points, offsets);

	auto img_center = cv::Point((int)(src_frames[0].cols / 2), (int)(src_frames[0].rows / 2));
	cv::Mat new_frame(src_frames[0].rows, src_frames[0].cols, CV_8UC3, black);
	cv::Point base(0, 0);

	for (size_t i = 0; i < src_frames.size(); i++)
	{
		//new_frame = offsetImageWithPadding(src_frames[i], offsets[i].x, offsets[i].y);
		new_frame = translateImg(src_frames[i], offsets[i].x, offsets[i].y);
		dst_frames.emplace_back(new_frame);
	}
}

void save_video(std::vector<cv::Mat>& src, std::string& filename)
{
	filename = filename + " centered.avi";
	auto writer = cv::VideoWriter(filename, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 24, src[0].size());
	std::for_each(src.begin(), src.end(), [&](auto& img) { writer.write(img); });
}

int main()
{
	cv::VideoCapture video;
	std::string filename = "video_3";
	get_video(filename, video); //loading video from file
	int frames_number = get_number_of_frames(video); // number of frames in video

	std::vector<cv::Mat> video_frames; // vector holding frames
	video_frames.reserve(frames_number);

	frame_video(video_frames, video); // chops video to frames

	cv::Mat temp(video_frames[0].rows, video_frames[0].cols, CV_8UC3, black);
	
	std::vector<cv::Mat> gray_frames;
	gray_frames.reserve(video_frames.size()); // converts video frames to grey scale
	convert_frames_to_gray(video_frames, gray_frames);
	
	click_to_crop(video_frames[0]); // setting object for ROI

	cv::Mat roi = video_frames[0](cv::Rect(A,B));

	cv::Mat roi_gray;
	cv::cvtColor(roi, roi_gray, cv::COLOR_BGR2GRAY);

	cv::destroyAllWindows();

	cv::Mat roi_thresh;
	threshold_image(roi_gray, roi_thresh);

	// every frame threshold
	std::vector<cv::Mat> thresh_frames;
	thresh_frames.reserve(frames_number);
	threshold_frames(gray_frames, thresh_frames);
	
	std::vector<cv::Point> center_points;
	center_points.reserve(frames_number);
	find_picture_center(roi_thresh, thresh_frames, center_points);
	
	cv::destroyAllWindows();
	std::vector<cv::Mat> centered_frames;
	centered_frames.reserve(video_frames.size());
	center_pictures(video_frames, centered_frames, center_points);
	//display_frames(30, centered_frames);
	save_video(centered_frames, filename);
}
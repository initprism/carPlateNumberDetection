#include "../header/OpenCv.h"
#define REFNUM 10

int main()
{
	Mat img_src = imread("img/car1.jpg", IMREAD_GRAYSCALE);
	imshow("imgsrcbb", img_src);
	blur(img_src, img_src, Size(3, 3));
	imshow("imgsrc", img_src);
	
	Mat img_canny;
	Canny(img_src, img_canny, 100, 300, 3);
	imshow("canny", img_canny);

	Mat img_contour(img_src.size(), CV_8UC3);
	double ratio;
	int cnt = 0;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(img_canny, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point());
	vector<vector<Point> > contours_poly(contours.size());
	vector<Rect> boundRect(contours.size());
	vector<Rect> boundRect2(contours.size());

	for (int i = 0; i< contours.size(); i++) {
		approxPolyDP(Mat(contours[i]), contours_poly[i], 1, true);
		boundRect[i] = boundingRect(Mat(contours_poly[i]));
	}
	for (int i = 0; i< contours.size(); i++) {
		
		ratio = (double)boundRect[i].height / boundRect[i].width;
		if ((ratio <= 2.5) && (ratio >= 0.5) && (boundRect[i].area() <= 700) && (boundRect[i].area() >= 100)) {
			drawContours(img_contour, contours, i, Scalar(0, 255, 255), 1, 8, hierarchy, 0, Point());
			rectangle(img_contour, boundRect[i].tl(), boundRect[i].br(), Scalar(255, 0, 0), 1, 8, 0);
			boundRect2[cnt] = boundRect[i];
			cnt += 1;
		}
	}
	boundRect2.resize(cnt);
	imshow("contour", img_contour);
	Rect rect, temp_rect;
	Mat img_clearContour(img_src.size(), CV_8UC3);
	int select, plate_width, check, friend_count = 0;
	double delta_x, delta_y, gradient;

	for (int i = 0; i<boundRect2.size(); i++) {
		for (int j = 0; j<(boundRect2.size() - i); j++) {
			if (boundRect2[j].tl().x > boundRect2[j + 1].tl().x) {
				temp_rect = boundRect2[j];
				boundRect2[j] = boundRect2[j + 1];
				boundRect2[j + 1] = temp_rect;
			}
		}
	}
	for (int i = 0; i< boundRect2.size(); i++) {
		rectangle(img_clearContour, boundRect2[i].tl(), boundRect2[i].br(), Scalar(0, 255, 0), 1, 8, 0);
		check = 0;
		for (int j = i + 1; j<boundRect2.size(); j++) {
			delta_x = abs(boundRect2[j].tl().x - boundRect2[i].tl().x);
			if (delta_x > 150)  
				break;
			delta_y = abs(boundRect2[j].tl().y - boundRect2[i].tl().y);
			if (delta_x == 0) {
				delta_x = 1;
			}
			if (delta_y == 0) {
				delta_y = 1;
			}
			gradient = delta_y / delta_x;
			//cout << gradient << endl;
			if (gradient < 0.25) {  
				check += 1;
			}
		}
		if (check > friend_count) {
			select = i; 
			friend_count = check;
			rectangle(img_clearContour, boundRect2[select].tl(), boundRect2[select].br(), Scalar(255, 0, 0), 1, 8, 0);
			plate_width = delta_x;
		}                         
	}
	rectangle(img_clearContour, boundRect2[select].tl(), boundRect2[select].br(), Scalar(0, 0, 255), 2, 8, 0);
	line(img_clearContour, boundRect2[select].tl(), Point(boundRect2[select].tl().x + plate_width, boundRect2[select].tl().y), Scalar(0, 0, 255), 1, 8, 0);
	imshow("img_clearContour", img_clearContour);

	Mat img_pre;
	img_pre = img_src(Rect(boundRect2[select].tl().x - 20, boundRect2[select].tl().y - 20, plate_width, plate_width*0.3));
	blur(img_pre, img_pre, Size(3, 3));
	
	Mat img_threshold;
	threshold(img_pre, img_threshold, 60, 255, CV_THRESH_BINARY_INV);
	imshow("img_threshold", img_threshold);
	Mat img_contours;
	img_threshold.copyTo(img_contours);
	contours.clear();
	findContours(img_contours,contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	for (int i = 0; i< contours.size(); i++) {
		approxPolyDP(Mat(contours[i]), contours_poly[i], 1, true);
		boundRect[i] = boundingRect(Mat(contours_poly[i]));
	}
	Mat img_test = img_threshold.clone();
	cnt = 0;
	for (int i = 0; i< contours.size(); i++) {
		ratio = (double)boundRect[i].height / boundRect[i].width;
		if ((ratio <= 2.5) && (ratio >= 0.5) && (boundRect[i].area() <= 700) && (boundRect[i].area() >= 100)) {
			rectangle(img_test, boundRect[i].tl(), boundRect[i].br(), Scalar(255, 0, 0), 1, 8, 0);
			boundRect2[cnt] = boundRect[i];
			cnt += 1;
		}
	}
	boundRect2.resize(cnt);
	imshow("img_test", img_test);
	Mat roi = img_threshold.clone();
	char name[20];
	int detectNum[7];
	for (int i = 0; i < boundRect2.size(); i++) {
		float minK = 100.0;
		Mat ROI = roi(boundRect2[i]);
		imshow(to_string(i), ROI);
		int top = 0;

		for (int j = 0; j < REFNUM; j++) {
			sprintf(name, "img/number/%d.jpg", j);
			Mat ref_src = imread(name, IMREAD_GRAYSCALE);
			resize(ref_src, ref_src, Size(ROI.cols, ROI.rows));
			threshold(ref_src, ref_src, 60, 255, THRESH_BINARY_INV);
			int cnt = 0;
			for (int k = 0; k < ref_src.rows; k++) {
				for (int p = 0; p < ref_src.cols; p++) {
					if (ref_src.at<uchar>(k, p) != 0 && ROI.at<uchar>(k, p) != 0)
						cnt++;
				}
			}
			if (cnt > top) {
				printf("%d\n", cnt);
				top = cnt;
				detectNum[i] = j;
			}
			imshow(name, ref_src);
			waitKey(0);
		}
	}

	cout << "Number : ";
	for (int i = 6; i >= 0; i--) {
		if(i != 4)
			cout << detectNum[i]; 
	}

	waitKey(0);
	destroyAllWindows();
	return 0;
}
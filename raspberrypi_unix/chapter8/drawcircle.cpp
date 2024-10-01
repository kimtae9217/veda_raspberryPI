#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

int main()
{
    Mat image = Mat::zeros(300, 400, CV_8UC3);
    image.setTo(Scalar(255, 255, 255));
    Scalar color(0, 255, 0);
    Scalar color2(255, 0, 0);
    Scalar color3(0, 0, 255);
    Point p1(10, 10), p2(100, 100), p3(220, 10), p4(220, 100);
    Size size(100, 100);
    Size size2(50, 40);
    Rect rect1(110, 10, 100, 100);
    Rect rect2(p3, size);
    rectangle(image, p1, p2, color, 2);
    rectangle(image, rect1, color, 2);
    rectangle(image, rect2, color, 2);
    line(image, p1, p2, color2, 5);
    circle(image, p2, 50, color3, -1);
    ellipse(image, p4, size2, 45, 0, 270, color3, -1);

    imshow("Draw Line & Rect & Circle", image);
    waitKey(0);
    return 0;
}

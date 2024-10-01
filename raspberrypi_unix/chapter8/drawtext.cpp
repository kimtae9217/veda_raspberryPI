#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

int main()
{
    Mat image = Mat::zeros(300, 400, CV_8UC3);
    image.setTo(Scalar(0, 0, 0));
    Scalar color(0, 128, 255);
    float scale = 0.8;


    putText(image, "HELLO TAEWON", Point(10, 30),
            FONT_HERSHEY_SIMPLEX, scale, color, 1);
    putText(image, "HELLO TAEWON", Point(10, 60),
            FONT_HERSHEY_PLAIN, scale, color, 1);
    putText(image, "HELLO TAEWON", Point(10, 90),
            FONT_HERSHEY_DUPLEX, scale, color, 1);
    putText(image, "HELLO TAEWON", Point(10, 120),
            FONT_HERSHEY_COMPLEX, scale, color, 1);
    putText(image, "HELLO TAEWON", Point(10, 150),
            FONT_HERSHEY_TRIPLEX, scale, color, 1);
    putText(image, "HELLO TAEWON", Point(10, 180),
            FONT_HERSHEY_COMPLEX_SMALL, scale, color, 1);
    putText(image, "HELLO TAEWON", Point(10, 210),
            FONT_HERSHEY_SCRIPT_SIMPLEX, scale, color, 1);
    putText(image, "HELLO TAEWON", Point(10, 240),
            FONT_HERSHEY_SCRIPT_COMPLEX, scale, color, 1);
    putText(image, "HELLO TAEWON", Point(10, 270),
            FONT_HERSHEY_PLAIN | FONT_ITALIC, scale, color, 1);

    imshow("Draw Polygon", image);
    waitKey(0);
    return 0;

}

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

int main()
{
    int histSize = 255;
    Mat image1 = imread("mandrill.jpg", IMREAD_COLOR);
    Mat image2 = Mat::zeros(image1.size(), image1.type());
    Mat histImg(histSize, histSize, CV_8U, Scalar(histSize));

    image1 += 50;
    cvtColor(image1, image2, COLOR_BGR2YCrCb);
    std::vector<Mat> channels;
    split(image2, channels);
    equalizeHist(channels[0], channels[0]);
    merge(channels, image2);
    cvtColor(image2, histImg, COLOR_YCrCb2BGR);

    imshow("Image", image1);
    imshow("equalize", histImg);

    waitKey(0);

    return 0;
}

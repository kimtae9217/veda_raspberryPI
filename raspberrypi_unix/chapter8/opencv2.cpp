#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

int main(int argc, char **argv)
{
    Mat image1 = imread("mandrill.jpg", IMREAD_COLOR);
    Mat image2 = imread("lena.bmp", IMREAD_COLOR);
    Mat image_add = Mat::zeros(image1.size(), image1.type());
    Mat image_sub = Mat::zeros(image1.size(), image1.type());
    Mat image_mul = Mat::zeros(image1.size(), image1.type());
    Mat image_div = Mat::zeros(image1.size(), image1.type());
    Mat image_gray1 = Mat::zeros(image1.size(), CV_8UC1);
    Mat image_gray2 = Mat::zeros(image1.size(), CV_8UC1);
    Mat image_white = Mat::zeros(image1.size(), CV_8UC1);
    Mat image_gray_sub = Mat::zeros(image1.size(), CV_8UC1);

    add(image1, image2, image_add);
    subtract(image1, image2, image_sub);
    multiply(image1, image2, image_mul);
    divide(image1, image2, image_div);
    cvtColor(image1, image_gray1, COLOR_BGR2GRAY);
    cvtColor(image2, image_gray2, COLOR_BGR2GRAY);
    absdiff(image_gray1, image_gray2, image_gray_sub);
    threshold(image_gray_sub, image_white, 100, 255, THRESH_BINARY);

    imshow("IMAGE_1", image1);
    imshow("IMAGE_2", image2);
    imshow("IMAGE_ADDITION", image_add);
    imshow("IMAGE_SUBTRACTION", image_sub);
    imshow("IMAGE_MULTIPLICATION", image_mul);
    imshow("IMAGE_DIVISION", image_div);
    imshow("IMAGE_GRAY1", image_gray1);
    imshow("IMAGE_GRAY2", image_gray2);
    imshow("IMAGE_WHITE", image_white);

    waitKey(0);
    

    destroyAllWindows();

    return 0;
}

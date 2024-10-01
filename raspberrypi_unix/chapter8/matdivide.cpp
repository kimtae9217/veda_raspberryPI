#include <opencv2/highgui/highgui.hpp>

using namespace cv;

int main()
{
    Mat image = imread("mandrill.jpg", IMREAD_COLOR);
    image /= 2;

    imshow("Mat : Divide", image);
    waitKey(0);

    return 0;
}

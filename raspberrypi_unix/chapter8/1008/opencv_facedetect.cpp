#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>

using namespace cv;

#define FBDEV        "/dev/fb0"      /* 프레임 버퍼를 위한 디바이스 파일 */
#define CAMERA_COUNT 300
#define CAM_WIDTH    800
#define CAM_HEIGHT       600                

typedef unsigned char ubyte;

const static char* cascade_name = "/usr/share/opencv4/haarcascades/haarcascade_frontalface_alt.xml";

int main(int argc, char **argv)
{
    int fbfd;              /* 프레임버퍼의 파일 디스크립터 */
    struct fb_var_screeninfo vinfo; // 프레임버퍼 정보 처리를 위한 구조체 
    ubyte *buffer;
    unsigned short *pfbmap;
    unsigned int x, y, i, j, screensize;
    ubyte r, g, b = 255;

    VideoCapture vc(0);
    CascadeClassifier cascade;
    Mat frame(CAM_WIDTH, CAM_HEIGHT, CV_8UC3, Scalar(255));
    Point pt1, pt2;

    if(!cascade.load(cascade_name)){
        perror("load()");
        return EXIT_FAILURE;
    }

    vc.set(CAP_PROP_FRAME_WIDTH, CAM_WIDTH);
    vc.set(CAP_PROP_FRAME_HEIGHT, CAM_HEIGHT);

    /* 프레임버퍼 열기 */
    fbfd = open(FBDEV, O_RDWR);
    if(fbfd == -1) {
        perror("open( ) : framebuffer device");
        return EXIT_FAILURE;
    }

    /* 프레임버퍼의 정보 가져오기 */
    if(ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        perror("Error reading variable information.");
        return EXIT_FAILURE;
    }

    
    // mmap( ) : 프레임버퍼를 위한 메모리 공간 확보 
    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel/8.;
    pfbmap = (unsigned short *)mmap(NULL, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if(pfbmap == (unsigned short *)-1) {
        perror("mmap() : framebuffer device to memory");
        return EXIT_FAILURE;
    }
     
    memset(pfbmap, 0, screensize);

    for(i = 0; i< CAMERA_COUNT; i++){
        int colors = vinfo.bits_per_pixel/8;
        long location = 0;
        
        vc >> frame;

        Mat image(CAM_WIDTH, CAM_HEIGHT, CV_8UC1, Scalar(255));
        cvtColor(frame, image, COLOR_BGR2GRAY);
        equalizeHist(image, image);

        std::vector<Rect> faces;
        cascade.detectMultiScale(image, faces, 1.1, 2, 0|CASCADE_SCALE_IMAGE, Size(30,30));

        for(j = 0; j < faces.size(); j++){
            pt1.x = faces[j].x; pt2.x = (faces[j].x + faces[j].width);
            pt1.y = faces[j].y; pt2.y = (faces[j].y + faces[j].height);

            rectangle(frame, pt1, pt2, Scalar(255,0,0), 3, 8);
        }

        buffer = (uchar*)frame.data;

        for(y = 0, location = 0; y < frame.rows; y++){
            for(x = 0; x < vinfo.xres; x++){
                if(x >= frame.cols){
                    location++;
                    continue;
                }
                
                ubyte b = *(buffer + (y * frame.cols + x)*3 + 0);
                ubyte g = *(buffer + (y * frame.cols + x)*3 + 1);
                ubyte r = *(buffer + (y * frame.cols + x)*3 + 2);      

                // convert 16bit(5:6:5)
                pfbmap[location++] = ((r>>3)<<11)|((g>>2)<<5)|(b>>3);

            }
        } 
    }


    munmap(pfbmap, screensize);

    close(fbfd);
    return 0;;


}

#include <fcntl.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

#define FBDEV        "/dev/fb0"      /* 프레임 버퍼를 위한 디바이스 파일 */
#define CAMERA_COUNT 100
#define CAM_WIDTH    640
#define CAM_HEIGHT       480                



int main(int argc, char **argv)
{
    int fbfd;              /* 프레임버퍼의 파일 디스크립터 */
    struct fb_var_screeninfo vinfo; // 프레임버퍼 정보 처리를 위한 구조체 
    unsigned char *buffer, *pfbmap;
    unsigned int x, y, i, screensize;

    VideoCapture vc(0);
    Mat image(CAM_WIDTH, CAM_HEIGHT, CV_8UC3, Scalar(255));
    if(!vc.isOpened()){
        perror("OpenCV : open WebCam\n");
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
    pfbmap = (unsigned char *)mmap(NULL, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if(pfbmap == (unsigned char*)-1) {
        perror("mmap() : framebuffer device to memory");
        return EXIT_FAILURE;
    }
    
    
    memset(pfbmap, 0, screensize);

    for(i = 0; i< CAMERA_COUNT; i++){
        int colors = vinfo.bits_per_pixel/8;
        long location = 0;
        int istride = image.cols*colors;
        vc >> image;
        buffer = (uchar*)image.data;

        for(y = 0; y < image.rows && vinfo.yres; y++){
            for(x = 0; x < image.cols && vinfo.xres; x++){
                long location = (y * vinfo.xres + x)* colors;
                
                uchar b = buffer[(y * image.cols + x)*3 + 0];
                uchar g = buffer[(y * image.cols + x)*3 + 1];
                uchar r = buffer[(y * image.cols + x)*3 + 2];      

                // convert 16bit(5:6:5)
                uint16_t pixel = ((r>>3)<<11)|((g>>2)<<5)|(b>>3);

                // frameBuffer capture (Little Endian)
                pfbmap[location + 0] = pixel & 0xFF;
                pfbmap[location + 1]  = (pixel >> 8) & 0xFF;


                /* //32 bit
                pfbmap[location++] = *(buffer+(y*image.cols+x)*3+0);
                pfbmap[location++] = *(buffer+(y*image.cols+x)*3+1);
                pfbmap[location++] = *(buffer+(y*image.cols+x)*3+2);
                pfbmap[location++] = 0xFF;
                */
            }
        } 
    }


    munmap(pfbmap, screensize);

    close(fbfd);
    return 0;;


}

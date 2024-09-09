#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdlib.h>

#define FBDEVICE "/dev/fb0"

typedef unsigned char ubyte;
struct fb_var_screeninfo vinfo;

// 32bit -> 16bit
unsigned short makepixel(unsigned char r, unsigned char g, unsigned char b){
    return (unsigned short)(((r>>3)<<11)|((g>>2)<<5)|(b>>3));
}


/* 프랑스 국기 만들기 */
static void drawfacemmap(int fd, int start_x, int start_y, int end_x, int end_y, ubyte r, ubyte g, ubyte b)
{
    unsigned short *pfb; 
    ubyte a = 0xFF;
    int color = vinfo.bits_per_pixel/8.;
   
    if(end_x == 0) end_x = vinfo.xres;
    if(end_y == 0) end_y = vinfo.yres;
       
    pfb = (unsigned short*)mmap(0, vinfo.xres * vinfo.yres * color,  PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
       
    for(int x = start_x; x < end_x; x++){
        for(int y = start_y; y < end_y; y++){
            unsigned short pixel = makepixel(r, g, b);
            int xline = vinfo.xres * y;
            *(pfb + x + xline) = pixel;

           // *(pfb + (x+0) + y*vinfo.xres*color) = b;
           // *(pfb + (x+1) + y*vinfo.xres*color) = g;
           // *(pfb + (x+2) + y*vinfo.xres*color) = r;
           // *(pfb + (x+3) + y*vinfo.xres*color) = a;
        }   
    }


    munmap(pfb, vinfo.xres * vinfo.yres * color);
}   


int main(int argc, char **argv)
{
    int fbfd, status, offset;

    fbfd = open(FBDEVICE, O_RDWR);
    if(fbfd < 0){
        perror("Error: cannot open framebuffer device");
        return -1;
    }

    if(ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) < 0){
        perror("Error reading fixed information");
        return -1;
    }
    
    int xresW = vinfo.xres / 3;
    //xresW*0, xresW*1
    //xresW*1, xresW*2
    //xresW*2, xresW*3
    drawfacemmap(fbfd, xresW*0, 0, xresW*1, 0, 0, 0, 255); 
    drawfacemmap(fbfd, xresW*1, 0, xresW*2, 0, 255, 255, 255); 
    drawfacemmap(fbfd, xresW*2, 0, xresW*3, 0, 255, 0, 0); 

    close(fbfd);

    return 0;

}

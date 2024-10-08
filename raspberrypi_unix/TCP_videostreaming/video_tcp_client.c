#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>

#define FBDEV "/dev/fb0"
#define WIDTH 800
#define HEIGHT 600
#define SERVER_IP "192.168.0.40"
#define SERVER_PORT 5100
#define FRAME_SIZE (WIDTH * HEIGHT * 2)

static short *fbp = NULL;
static struct fb_var_screeninfo vinfo;

static void error_exit(const char *s)
{
    fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
    exit(EXIT_FAILURE);
}

static int clip(int value, int min, int max)
{
    return (value > max ? max : value < min ? min : value);
}

static void process_image(const void *p)
{
    unsigned char *in = (unsigned char *)p;
    int istride = WIDTH * 2;
    int x, y, j;
    int y0, u, y1, v, r, g, b;
    unsigned short pixel;
    long location = 0;

    for (y = 0; y < HEIGHT; ++y) {
        for (j = 0, x = 0; j < vinfo.xres * 2; j += 4, x += 2) {
            if(j >= WIDTH*2){
                location++; location++;
                continue;
            }

            y0 = in[j];
            u = in[j + 1] - 128;
            y1 = in[j + 2];
            v = in[j + 3] - 128;

            r = clip((298 * y0 + 409 * v + 128) >> 8, 0, 255);
            g = clip((298 * y0 - 100 * u - 208 * v + 128) >> 8, 0, 255);
            b = clip((298 * y0 + 516 * u + 128) >> 8, 0, 255);
            pixel = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
            fbp[location++] = pixel;

            r = clip((298 * y1 + 409 * v + 128) >> 8, 0, 255);
            g = clip((298 * y1 - 100 * u - 208 * v + 128) >> 8, 0, 255);
            b = clip((298 * y1 + 516 * u + 128) >> 8, 0, 255);
            pixel = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
            fbp[location++] = pixel;
        }
        in += istride;
    }
}

int receive_frame(int sock, char *buffer)
{
    int total_received = 0;
    while (total_received < FRAME_SIZE) {
        int to_receive = FRAME_SIZE - total_received;
        
        int received = recv(sock, buffer + total_received, to_receive, 0);
        if (received <= 0) {
            if (received == 0) {
                printf("Server disconnected\n");
            } else {
                perror("recv failed");
            }
            return -1;
        }
        total_received += received;
    }
    printf("Received frame size: %d\n", total_received); // 프레임 확인 
    return 0;
}

int main()
{
    int fbfd, sock;
    struct sockaddr_in server_addr;
    long screensize;
    char buffer[FRAME_SIZE];

    fbfd = open(FBDEV, O_RDWR);
    if (fbfd == -1) {
        error_exit("Error opening framebuffer device");
    }

    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        error_exit("Error reading variable information");
    }

    printf("Framebuffer resolution: %dx%d\n", vinfo.xres, vinfo.yres);
    if (vinfo.xres != WIDTH || vinfo.yres != HEIGHT) {
        fprintf(stderr, "Framebuffer resolution does not match camera resolution\n");
    }

    screensize = vinfo.xres * vinfo.yres * 2;
    fbp = (short *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if (fbp == MAP_FAILED) {
        error_exit("Error: failed to map framebuffer");
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        error_exit("Error opening socket");
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        error_exit("Invalid address/ Address not supported");
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        error_exit("Connection Failed");
    }

    printf("Connected to server. Video streaming...\n");

    while (1) {
        memset(buffer, 0, FRAME_SIZE);  
        if (receive_frame(sock, buffer) < 0) {
            break;
        }
        process_image(buffer);
    }

    close(sock);
    munmap(fbp, screensize);
    close(fbfd);

    return 0;
}

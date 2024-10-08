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
#include <linux/videodev2.h>

#define VIDEODEV "/dev/video0"
#define WIDTH 800
#define HEIGHT 600
#define TCP_PORT 5100
#define FRAME_SIZE (WIDTH * HEIGHT * 2)  // YUYV format

struct buffer {
    void *start;
    size_t length;
};

static struct buffer *buffers = NULL;
static unsigned int n_buffers = 0;

static void error_exit(const char *s)
{
    fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
    exit(EXIT_FAILURE);
}

static int xioctl(int fd, int request, void *arg)
{
    int r;
    do r = ioctl(fd, request, arg);
    while (-1 == r && EINTR == errno);
    return r;
}

static void init_mmap(int fd)
{
    struct v4l2_requestbuffers req = {0};
    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
        error_exit("VIDIOC_REQBUFS");
    }

    buffers = calloc(req.count, sizeof(*buffers));

    for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
        struct v4l2_buffer buf = {0};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffers;

        if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
            error_exit("VIDIOC_QUERYBUF");

        buffers[n_buffers].length = buf.length;
        buffers[n_buffers].start = mmap(NULL, buf.length,
                                        PROT_READ | PROT_WRITE,
                                        MAP_SHARED,
                                        fd, buf.m.offset);

        if (MAP_FAILED == buffers[n_buffers].start)
            error_exit("mmap");
    }
}

static void init_device(int fd)
{
    struct v4l2_format fmt = {0};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = WIDTH;
    fmt.fmt.pix.height = HEIGHT;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
        error_exit("VIDIOC_S_FMT");

    init_mmap(fd);
}

static void start_capturing(int fd)
{
    enum v4l2_buf_type type;
    for (unsigned int i = 0; i < n_buffers; ++i) {
        struct v4l2_buffer buf = {0};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
            error_exit("VIDIOC_QBUF");
    }
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
        error_exit("VIDIOC_STREAMON");
}

static int send_frame(const void *p, int size, int client_socket)
{
    int total_sent = 0;
    while (total_sent < size) {
        int sent = send(client_socket, p + total_sent, size - total_sent, 0);
        if (sent < 0) {
            if (errno == EINTR) continue;
            perror("send");
            return -1;
        }
        total_sent += sent;
    }
    return 0;
}

static int read_frame(int fd, int client_socket)
{
    struct v4l2_buffer buf = {0};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
        switch (errno) {
            case EAGAIN:
                return 0;
            case EIO:
            default:
                error_exit("VIDIOC_DQBUF");
        }
    }

    printf("frame size: %d\n", buf.bytesused);  // 프레임 확인

    if (send_frame(buffers[buf.index].start, buf.bytesused, client_socket) < 0) {
        return -1;
    }

    if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
        error_exit("VIDIOC_QBUF");

    return 1;
}

int main(int argc, char **argv)
{
    int server_socket, client_socket, camera_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    camera_fd = open(VIDEODEV, O_RDWR);
    if (camera_fd == -1)
        error_exit("Opening video device");

    init_device(camera_fd);
    start_capturing(camera_fd);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
        error_exit("socket");

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(TCP_PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        error_exit("bind");

    if (listen(server_socket, 5) < 0)
        error_exit("listen");

    printf("Server listening on port %d\n", TCP_PORT);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0)
            error_exit("accept");

        printf("Client connected\n");

        while (1) {
            fd_set fds;
            struct timeval tv;
            int r;

            FD_ZERO(&fds);
            FD_SET(camera_fd, &fds);
            FD_SET(client_socket, &fds);

            tv.tv_sec = 2;
            tv.tv_usec = 0;

            r = select(((camera_fd > client_socket) ? camera_fd : client_socket) + 1, &fds, NULL, NULL, &tv);

            if (-1 == r) {
                if (EINTR == errno)
                    continue;
                error_exit("select");
            }

            if (0 == r) {
                fprintf(stderr, "select timeout\n");
                continue;
            }

            if (FD_ISSET(client_socket, &fds)) {
                char buffer[1];
                if (recv(client_socket, buffer, 1, MSG_PEEK | MSG_DONTWAIT) == 0) {
                    printf("Client disconnected\n");
                    break;
                }
            }

            
            if (FD_ISSET(camera_fd, &fds)) {
                if (read_frame(camera_fd, client_socket) < 0) {
                    printf("Error sending frame, client disconnected\n");
                    break;
                }
            }
            
        }

        close(client_socket);
    }

    close(server_socket);
    close(camera_fd);

    for (unsigned int i = 0; i < n_buffers; ++i)
        if (-1 == munmap(buffers[i].start, buffers[i].length))
            error_exit("munmap");
    free(buffers);

    return 0;
}

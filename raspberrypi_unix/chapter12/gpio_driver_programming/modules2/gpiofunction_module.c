#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/gpio.h> // GPIO 관련 함수 사용


MODULE_LICENSE("GPL");
MODULE_AUTHOR("YoungJin Suh");
MODULE_DESCRIPTION("Raspberry Pi GPIO LED Device Module");

#define GPIO_MAJOR		200
#define GPIO_MINOR		0
#define GPIO_DEVICE		"gpioled"
#define GPIO_LED		  589 // cat /sys/kernel/debug/gpio로 확인 하여 gpio 18번이 589인 것을 확인 

static char msg[BLOCK_SIZE] = {0};

static int gpio_open(struct inode *, struct file *);
static ssize_t gpio_read(struct file *, char *, size_t, loff_t *);
static ssize_t gpio_write(struct file *, const char *, size_t, loff_t *);
static int gpio_close(struct inode *, struct file *);

static struct file_operations gpio_fops = {
  .owner = THIS_MODULE,
  .read  = gpio_read,
  .write = gpio_write,
  .open  = gpio_open,
  .release = gpio_close,
};

struct cdev gpio_cdev;

// GPIO 레지스터를 표현하는 구조체. GPIO의 상태와 제어를 담당하는 레지스터 포함.
typedef struct {
  uint32_t status;  // GPIO 상태 레지스터
  uint32_t ctrl;    // GPIO 제어 레지스터
} GPIOregs;

// GPIO 레지스터를 쉽게 접근하기 위해 정의한 매크로
#define GPIO ((GPIOregs*)GPIOBase)

// RIO(입출력) 레지스터를 위한 구조체 정의. GPIO 핀의 입출력 제어와 관련된 레지스터 포함.
typedef struct {
  uint32_t Out;     // 출력 레지스터
  uint32_t OE;      // 출력 enable 레지스터
  uint32_t In;      // 입력 레지스터
  uint32_t InSync;  // 동기화된 입력 레지스터
} rioregs;


// RIO 레지스터에 접근하기 위한 매크로 정의
#define rio ((rioregs *)RIOBase)
#define rioXOR ((rioregs *)(RIOBase + 0x1000 / 4))  // XOR 연산을 위한 레지스터 블록
#define rioSET ((rioregs *)(RIOBase + 0x2000 / 4))  // GPIO 핀을 설정하는 레지스터 블록
#define rioCLR ((rioregs *)(RIOBase + 0x3000 / 4))  // GPIO 핀을 클리어하는 레지스터 블록

int init_module(void)
{
  dev_t devno;
  unsigned int count;
//  static void *map;
  int err;

  printk(KERN_INFO "Hello module!\n");

  try_module_get(THIS_MODULE);

  devno = MKDEV(GPIO_MAJOR, GPIO_MINOR);
  register_chrdev_region(devno, 1, GPIO_DEVICE);

  cdev_init(&gpio_cdev, &gpio_fops);

  gpio_cdev.owner = THIS_MODULE;

  count = 1;

  err = cdev_add(&gpio_cdev, devno, count);
  if(err<0) {
    printk("Error : Device Add\n");
    return -1;
  }

  printk(" 'mknod /dev/%s c %d 0'\n", GPIO_DEVICE,GPIO_MAJOR);
  printk(" 'chmod 666 /dev/%s'\n", GPIO_DEVICE);


  // GPIO 사용을 요청 
  gpio_request(GPIO_LED, "LED");
  gpio_direction_output(GPIO_LED, 0);


  return 0;
}

void cleanup_module(void)
{

  dev_t devno = MKDEV(GPIO_MAJOR, GPIO_MINOR);
  unregister_chrdev_region(devno, 1);

  cdev_del(&gpio_cdev);

  // 더 이상 사용이 필요없는 경우 관련 자원 해제 
  gpio_free(GPIO_LED);
  gpio_direction_output(GPIO_LED, 0);
  
  module_put(THIS_MODULE);
  printk(KERN_INFO "Good-bye module!\n");
}


static int gpio_open(struct inode * inode, struct file * fil)
{
  printk("GPIO DEvice opened(%d/%d)\n",imajor(inode), iminor(inode));
  return 0;
}

static int gpio_close(struct inode * inode, struct file *fil)
{
  printk("GPIO DEvice closed(%d)\n",MAJOR(fil->f_path.dentry->d_inode->i_rdev));
  return 0;
}

static ssize_t gpio_read(struct file *inode , char * buff, size_t len, loff_t *off)
{
  int count;

  strcat(msg, " from Kernel");
  count = copy_to_user(buff, msg, strlen(msg)+1);

  printk("GPIO Device(%d) read : %s(%d)\n",MAJOR(inode->f_path.dentry->d_inode->i_rdev),msg, count);

  return count;
}

static ssize_t gpio_write(struct file * inode , const char * buff, size_t len, loff_t *off)
{
  short count;

  memset(msg,0,BLOCK_SIZE);	// BLOCK_SIZE

  count = copy_from_user(msg,buff, len);

  // LED 설정
  gpio_set_value(GPIO_LED, (!strcmp(msg, "0"))?0:1);

  printk("GPIO Device(%d) write : %s(%ld)\n",MAJOR(inode->f_path.dentry->d_inode->i_rdev),msg, len);

  return count;
}

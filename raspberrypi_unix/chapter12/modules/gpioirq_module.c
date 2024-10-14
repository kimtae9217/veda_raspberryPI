#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YoungJin Suh");
MODULE_DESCRIPTION("Raspberry Pi GPIO LED Device Module");

#define GPIO_MAJOR		200
#define GPIO_MINOR		0
#define GPIO_DEVICE		"gpioled"
#define GPIO_LED		  589 // cat /sys/kernel/debug/gpio로 확인 하여 gpio 18번이 589인 것을 확인 
#define GPIO_SW       595

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
static int switch_irq;


static irqreturn_t isr_func(int irq, void *data)
{
    if(irq == switch_irq && !gpio_get_value(GPIO_LED)) {
        printk("Switch Off");
        gpio_set_value(GPIO_LED, 1);
    } else if(irq == switch_irq && gpio_get_value(GPIO_LED)) {
        printk("Switch On");
        gpio_set_value(GPIO_LED, 0);
    }

    return IRQ_HANDLED;
}

int init_module(void)
{
    dev_t devno;
    unsigned int count;
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
	printk("'mknod /dev/%s c %d 0'\n", GPIO_DEVICE, GPIO_MAJOR);
	printk("'chmod 666 /dev/%s'\n", GPIO_DEVICE);
	
	gpio_request(GPIO_LED, "LED");
	gpio_direction_output(GPIO_LED, 0);
	gpio_request(GPIO_SW, "SWITCH");
    gpio_direction_input(GPIO_SW);
    switch_irq = gpio_to_irq(GPIO_SW);
	err = request_irq(switch_irq, isr_func, IRQF_TRIGGER_RISING, "switch", NULL);
	return 0;
}

void cleanup_module(void)
{
	dev_t devno = MKDEV(GPIO_MAJOR, GPIO_MINOR);
	unregister_chrdev_region(devno, 1);
	
	cdev_del(&gpio_cdev);
	free_irq(switch_irq, NULL);
	gpio_free(GPIO_LED);
	gpio_free(GPIO_SW);

	module_put(THIS_MODULE);
	printk("Good-Bye module!\n");
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

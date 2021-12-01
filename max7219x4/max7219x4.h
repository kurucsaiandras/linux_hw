
#include <linux/module.h>
#include <linux/init.h>
#include <linux/spi/spi.h>
#include <linux/uaccess.h>
#include <linux/fs.h>



/* Character device file operation functions */
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);


/* Some meta information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kardos Martin");
MODULE_DESCRIPTION("4 cascaded MAX7219 SPI Driver for the raspberry pi on SPI bus 0");


//device related constants
#define DEVICE_NAME "max7219x4"
#define BUF_LEN 200
#define BUS_NUM 0

static int majorNumber;
static int is_device_open = 0;
static char msg[BUF_LEN];
static char *msg_ptr;

dev_t min_maj;  // majorNumber and Minor device numbers combined into 32 bits
struct class *pClass;  // class_create will set this
static struct file_operations fops = {
  .read = device_read,
  .write = device_write,
  .open = device_open,
  .release = device_release
};

//typedef static struct spi_device spi_device;
static struct spi_device *max7219;

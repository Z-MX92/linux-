#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/string.h>

//LED1 -- GPIO0_D0 -- 0*32 + 3*8 + 0 = 24
//LED2 -- GPIO1_D5 -- 1*32 + 3*8 + 5 = 61
//BEEP的引脚:GPIO1_A4 -- 32*1 + 4 -- 36

dev_t dev_led1;
dev_t dev_led2;
dev_t dev_beep;

struct cdev mycdev;//linux2.6的核心结构体
struct file_operations myfops;//内核层的操作集合结构体
struct class * myclass;//类结构体


static int myopen(struct inode *inode, struct file *file)
{
	const char *dev_name = file->f_path.dentry->d_name.name;//通过设备文件名判断
	//inode->i_rdev可以获取设备号，通过判断设备号区分设备
	printk("%s打开\n", dev_name);
	if (strcmp(dev_name, "LED1") == 0)
	{
		gpio_set_value(24, 1);
	}
	if (strcmp(dev_name, "LED2") == 0)
	{
		gpio_set_value(61, 1);
	}
	if (strcmp(dev_name, "BEEP") == 0)
	{
		gpio_set_value(36, 1);
	}
	return 0;
}

static int myclose(struct inode *inode, struct file *file)
{
	const char *dev_name = file->f_path.dentry->d_name.name;
	printk("%s关闭\n", dev_name);
	if (strcmp(dev_name, "LED1") == 0)
	{
		gpio_set_value(24, 0);
	}
	if (strcmp(dev_name, "LED2") == 0)
	{
		gpio_set_value(61, 0);
	}
	if (strcmp(dev_name, "BEEP") == 0)
	{
		gpio_set_value(36, 0);
	}
	return 0;
}

static int __init APP_init(void)
{
	gpio_request(24, "LED1");
	gpio_request(61, "LED2");
	gpio_request(36, "BEEP");
	gpio_direction_output(24, 0);
	gpio_direction_output(61, 0);
	gpio_direction_output(36, 0);

	//申请一个字符设备的设备号
	alloc_chrdev_region(&dev_led1, 0, 1, "led1");
	alloc_chrdev_region(&dev_led2, 0, 1, "led2");
	alloc_chrdev_region(&dev_beep, 0, 1, "beep");

	//初始化设备操作集
	myfops.owner = THIS_MODULE;
	myfops.open = myopen;
	myfops.release = myclose;
	
	//初始化字符设备结构体
	cdev_init(&mycdev, &myfops);
	
	//在内核注册设备
	cdev_add(&mycdev, dev_led1, 1);
	cdev_add(&mycdev, dev_led2, 1);
	cdev_add(&mycdev, dev_beep, 1);

	//创建类结构体
	myclass = class_create(THIS_MODULE, "myclass");
	
	//创建设备文件
	device_create(myclass, NULL, dev_led1, NULL, "LED1");
	device_create(myclass, NULL, dev_led2, NULL, "LED2");
	device_create(myclass, NULL, dev_beep, NULL, "BEEP");
	
	return 0;
}

static void __exit APP_exit(void)
{
	//移除设备文件
	device_destroy(myclass, dev_led1);
	device_destroy(myclass, dev_led2);
	device_destroy(myclass, dev_beep);
	//删除类结构体
	class_destroy(myclass);
	//注销设备
	cdev_del(&mycdev);
}

module_init(APP_init);
module_exit(APP_exit);
MODULE_LICENSE("GPL");



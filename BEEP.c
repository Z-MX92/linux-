#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>

//BEEP的引脚:GPIO1_A4 -- 32*1 + 4 -- 36
struct miscdevice mymisc;
struct file_operations myfops;

static int myopen(struct inode *inode, struct file *file)
{
	gpio_set_value(36, 1);
	printk("蜂鸣器开\n");
	return 0;
}

static int myclose(struct inode *inode, struct file *file)
{
	gpio_set_value(36, 0);
	printk("蜂鸣器关\n");
	return 0;
}

static int __init BEEP_init(void)
{
	gpio_request(36, "BEEP");
	gpio_direction_output(36, 0);

	myfops.owner = THIS_MODULE;
	myfops.open = myopen;
	myfops.release = myclose;

	mymisc.minor = 255;
	mymisc.name = "MyBEEP";
	mymisc.fops = &myfops;
	misc_register(&mymisc);
	return 0;
}

static void __exit BEEP_exit(void)
{
	misc_deregister(&mymisc);
}

module_init(BEEP_init);
module_exit(BEEP_exit);
MODULE_LICENSE("GPL");


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>

struct miscdevice mymisc;
struct file_operations myfops;

static int myopen(struct inode *inode, struct file *file)
{
	gpio_set_value(21, 1);
	gpio_set_value(22, 1);
	printk("开灯\n");
	return 0;
}

static int myclose(struct inode *inode, struct file *file)
{
	gpio_set_value(21, 0);
	gpio_set_value(22, 0);
	printk("关灯\n");
	return 0;
}


static int __init LED_init(void)
{
	gpio_request(21, "LED21");
	gpio_request(22, "LED22");
	gpio_direction_output(21, 0);
	gpio_direction_output(22, 0);

	myfops.owner = THIS_MODULE;
	myfops.open = myopen;
	myfops.release = myclose;

	mymisc.minor = 255;
	mymisc.name = "MyLED";
	mymisc.fops = &myfops;
	misc_register(&mymisc);
	return 0;
}

static void __exit LED_exit(void)
{
	misc_deregister(&mymisc);
}

module_init(LED_init);
module_exit(LED_exit);
MODULE_LICENSE("GPL");






#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/timer.h>

//KEY1的引脚:GPIO1_A7 -- 32*1 + 7 -- 39
//KEY2的引脚:GPIO1_B1 -- 32*1 + 1*8 +1 -- 41
DECLARE_WAIT_QUEUE_HEAD(myqueue);//等待队列头

struct of_device_id myinfo = {.compatible = "mykey",};
struct timer_list mytimer;

struct platform_driver mydrv;
int key_gpio_num[2], key_value[2];
int irq_num[2];
int cond = 0;//阻塞条件

//linux2.6设备注册相关参数
dev_t mydev[2];
struct cdev mycdev;
struct file_operations myfops;
struct class *myclass;

irqreturn_t myirq_handler(int num, void *arg)
{
	if (num == irq_num[0])
	{
		mod_timer(&mytimer, jiffies + msecs_to_jiffies(15));
	}
	else if (num == irq_num[1])
	{
		mod_timer(&mytimer, jiffies + msecs_to_jiffies(15)); 
	}
	return 0;
}

static int myopen(struct inode *inode, struct file *file)
{
	
	return 0;
}

static int myclose(struct inode *inode, struct file *file)
{
	
	return 0;
}

ssize_t myread(struct file *file, char __user *buf, size_t size, loff_t *lof)
{
	int res = 0;
	wait_event_interruptible(myqueue, cond);//阻塞
	key_value[0] = gpio_get_value(key_gpio_num[0]);
	key_value[1] = gpio_get_value(key_gpio_num[1]);
	res = copy_to_user(buf, key_value, sizeof(key_value));
	cond = 0;
	return 0;
}


int myprobe(struct platform_device *dev)
{
	int i = 0, res = 0;
	for (i = 0; i < 2; i++)
	{
		//获取gpio引脚编号
		key_gpio_num[i] = of_get_named_gpio(dev->dev.of_node, "key_gpios", i);
		//获取中断编号
		irq_num[i] = platform_get_irq(dev, i);
		printk("KEY%d:引脚编号 %d 中断编号 %d\n", i + 1, key_gpio_num[i], irq_num[i]);
		//开启中断，由于内核的一个进程已经给开启了，所以不用开
		//enable_irq(irq_num[i]);
		//向内核注册中断
		res = request_irq(irq_num[i], myirq_handler, IRQF_TRIGGER_FALLING, "mykey", NULL);

		gpio_request(key_gpio_num[i], "mykey");
		gpio_direction_input(key_gpio_num[i]);
	}

	//linux2.6
	alloc_chrdev_region(mydev, 0, 2, "mykey");
	mydev[1] = mydev[0] + 1;

	myfops.owner = THIS_MODULE;
	myfops.open = myopen;
	myfops.release = myclose;
	myfops.read = myread;
	cdev_init(&mycdev, &myfops);
	
	for (i = 0; i < 2; i++)
	{
		cdev_add(&mycdev, mydev[i], 1);
	}

	myclass = class_create(THIS_MODULE, "myclass");
	for (i = 0; i < 2; i++)
	{
		device_create(myclass, NULL, mydev[i], NULL, "mykey%d", i + 1);
	}
	printk("匹配成功!\n");
	return 0;
}

int myremove(struct platform_device *dev)
{
	return 0;
}

void Mytimer_func(struct timer_list *list)
{
	key_value[0] = gpio_get_value(key_gpio_num[0]);
	key_value[1] = gpio_get_value(key_gpio_num[1]);

	printk("key1=%d\n", key_value[0]);
	printk("key2=%d\n", key_value[1]);
	if (key_value[0] == 0)
	{	
		printk("按键1被真正按下\n");
		cond = 1;
		wake_up_interruptible(&myqueue);
	}
	else if (key_value[1] == 0)
	{	
		printk("按键2被真正按下\n");
		cond = 1;
		wake_up_interruptible(&myqueue);
	}
	else
	{
		printk("这是一个抖动\n");
	}
}

static int __init MyKEY_Init(void)
{
	//软件定时器初始化
	mytimer.expires = jiffies + 5 * HZ;
	mytimer.function = Mytimer_func;
	mytimer.flags = 0;
	timer_setup(&mytimer, Mytimer_func, 0);

	//向内核注册驱动端结构体
	mydrv.driver.name = "hello";
	mydrv.driver.of_match_table = &myinfo;
	mydrv.probe = myprobe;
	mydrv.remove = myremove;
	platform_driver_register(&mydrv);
	return 0;
}

static void __exit MyKEY_Exit(void)
{
	device_destroy(myclass, mydev[0]);//删除设备
	device_destroy(myclass, mydev[1]);//删除设备
	class_destroy(myclass);
	cdev_del(&mycdev);
	platform_driver_unregister(&mydrv);
	printk("卸载成功!\n");
}


module_init(MyKEY_Init);
module_exit(MyKEY_Exit);
MODULE_LICENSE("GPL");



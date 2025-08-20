#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/string.h>

#define device_num 4//设备个数

struct platform_driver mydriver;
struct of_device_id mydev_id = {.compatible = "myled",};

int gpio_num[device_num], act_level[device_num];
enum of_gpio_flags myflags;

//linux2.6
dev_t mydev[device_num];
struct cdev mycdev;
struct file_operations myfops;
struct class *myclass;

int myopen(struct inode *inode, struct file *file)
{
	int i = 0;
	for (i = 0; i < device_num; i++)
	{
		if (mydev[i] == inode->i_rdev) break;
	}
	switch(i)
	{
		case 0:gpio_set_value(gpio_num[0], act_level[0]);printk("led1_on\n");break;//LED1
		case 1:gpio_set_value(gpio_num[1], act_level[1]);printk("led2_on\n");break;//LED2
		case 2:gpio_set_value(gpio_num[2], act_level[2]);printk("led21_on\n");break;//LED21绿灯
		case 3:gpio_set_value(gpio_num[3], act_level[3]);printk("led22_on\n");break;//LED22

	}
	return 0;
}

int myclose(struct inode *inode, struct file *file)
{
	int i = 0;
	for (i = 0; i < device_num; i++)
	{
		if (mydev[i] == inode->i_rdev) break;
	}
	switch(i)
	{
		case 0:gpio_set_value(gpio_num[0], !act_level[0]);printk("led1_off\n");break;//LED1
		case 1:gpio_set_value(gpio_num[1], !act_level[1]);printk("led2_off\n");break;//LED2
		case 2:gpio_set_value(gpio_num[2], !act_level[2]);printk("led21_off\n");break;//LED21
		case 3:gpio_set_value(gpio_num[3], !act_level[3]);printk("led22_off\n");break;//LED22
	}

	return 0;
}

int myprobe(struct platform_device *dev)
{
	int i = 0;
	printk("dev->dev.of_node->name = %s\n", (dev->dev.of_node->name));
	//获取节点信息
	for (i = 0; i < device_num; i++)
	{
		gpio_num[i] = of_get_named_gpio_flags(dev->dev.of_node, "led_gpios", i, &myflags);
		if (myflags == OF_GPIO_ACTIVE_LOW) act_level[i] = 0;
		else act_level[i] = 1;
		printk("设备%d:引脚编号 %d 有效电平 %d\n", i + 1, gpio_num[i], act_level[i]);
	}
	//请求gpio
	for (i = 0; i < device_num; i++)
	{
		gpio_request(gpio_num[i], "mydev");
		gpio_direction_output(gpio_num[i], !act_level[i] );
	}
	//linux2.6
	//1. 申请设备号
	alloc_chrdev_region(mydev, 0, device_num, "mydev");
	for (i = 1; i < device_num; i++)
	{
		mydev[i] = mydev[i - 1] + 1;
	}
	//2. 初始化linux2.6核心结构体
	myfops.owner = THIS_MODULE;
	myfops.open = myopen;
	myfops.release = myclose;
	cdev_init(&mycdev, &myfops);
	//3. 注册linux2.6结构体
	for (i = 0; i < device_num; i++)
	{
		cdev_add(&mycdev, mydev[i], 1);
	}
	//4. 创建类结构体
	myclass = class_create(THIS_MODULE, "myclass");
	//5. 创建设备文件
	for (i = 0; i < device_num; i++)
	{
		device_create(myclass, NULL, mydev[i], NULL, "my_dev%d", i + 1);
	}
	
	printk("匹配成功!\n");
	return 0;
}

int myremove(struct platform_device *dev)
{
	printk("驱动端卸载!\n");
	return 0;
}

static int __init Driver_Init(void)
{
	//向内核注册平台设备总线驱动端的信息结构体
	mydriver.probe = myprobe;
	mydriver.remove = myremove;
	mydriver.driver.name = "led";
	//const struct of_device_id	*of_match_table;常量指针，指向的值不可改变
	mydriver.driver.of_match_table = &mydev_id;
	platform_driver_register(&mydriver);
	
	return 0;
}

static void __exit Driver_Exit(void)
{
	int i = 0;
	//移除设备文件
	for (i = 0; i < device_num; i++)
	{
		device_destroy(myclass, mydev[i]);
	}
	//删除类结构体
	class_destroy(myclass);
	//注销设备
	cdev_del(&mycdev);
	
	
	platform_driver_unregister(&mydriver);
}

module_init(Driver_Init);
module_exit(Driver_Exit);
MODULE_LICENSE("GPL");


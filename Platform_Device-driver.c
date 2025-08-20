#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/cdev.h>

int gpio_num[2], act_level[2], i;
dev_t mydev[2];//设备号

struct platform_driver mydriver;
struct resource *myresource;//资源结构体

struct cdev mycdev;
struct file_operations myfops;
struct class * myclass;//类结构体

int myopen(struct inode *inode, struct file *file)
{
	for(i = 0;i < 2;i++)
    {
        if(inode->i_rdev == mydev[i]) break;
        else continue;
    }
	switch(i)
	{
		case 0:gpio_set_value(gpio_num[0], act_level[0]);printk("led21_on\n");break;//LED21
		case 1:gpio_set_value(gpio_num[1], act_level[1]);printk("led22_on\n");break;//LED22
	}
	return 0;
}

int myclose(struct inode *inode, struct file *file)
{
	for(i = 0;i < 2;i++)
    {
        if(inode->i_rdev == mydev[i]) break;
        else continue;
    }
	switch(i)
	{
		case 0:gpio_set_value(gpio_num[0], !act_level[0]);printk("led21_off\n");break;//LED21
		case 1:gpio_set_value(gpio_num[1], !act_level[1]);printk("led22_off\n");break;//LED22
		
	}	
	return 0;
}


int myprobe(struct platform_device *dev)
{
	//获取设备端的资源信息
	myresource = platform_get_resource(dev, IORESOURCE_MEM, 0);
	act_level[0] = !myresource->start;//led21
	act_level[1] = myresource->end;//led22
	myresource = platform_get_resource(dev, IORESOURCE_MEM, 1);
	gpio_num[0] = myresource->start;//led21
	gpio_num[1] = myresource->end;//led22
	printk("匹配成功!\n设备1:有效电平:%d\n引脚编号:%d\n设备2:有效电平:%d\n引脚编号:%d\n", gpio_num[0], act_level[0], gpio_num[1], act_level[1]);

	gpio_request(gpio_num[0], "led21");
	gpio_request(gpio_num[1], "led22");
	gpio_direction_output(gpio_num[0], !act_level[0]);
	gpio_direction_output(gpio_num[1], !act_level[1]);

	//linux2.6设备注册
	//1. 申请设备号
	alloc_chrdev_region(mydev, 0, 2, "led");
	for (i = 1; i < 2; i++)
	{
		mydev[i] = mydev[i - 1] + 1;
	}
	//2. 初始化linux2.6核心结构体
	myfops.owner = THIS_MODULE;
	myfops.open = myopen;
	myfops.release = myclose;
	cdev_init(&mycdev, &myfops);
	//3. 向内核注册linux2.6核心结构体
	for (i = 0; i < 2; i++)
	{
		cdev_add(&mycdev, mydev[i], 1);
	}
	//4. 创建类结构体
	myclass = class_create(THIS_MODULE, "myclass");
	//5.创建设备文件
	for (i = 0; i < 2; i++)
	{
		device_create(myclass, NULL, mydev[i], NULL, "my_led2%d", i + 1);
	}
	return 0;
}

int myremove(struct platform_device *dev)
{
	printk("设备端或驱动端卸载!");
	return 0;
}

static int __init LED_Driver_Init(void)
{
	//向内核注册平台设备总线驱动端的信息结构体
	mydriver.probe = myprobe;//匹配成功的回调函数
	mydriver.remove = myremove;//有一端卸载的回调函数
	mydriver.driver.name = "myled";
	platform_driver_register(&mydriver);
	
	return 0;
}

static void __exit LED_Driver_exit(void)
{
	//移除设备文件
	for (i = 0; i < 2; i++)
	{
		device_destroy(myclass, mydev[i]);
	}
	//删除类结构体
	class_destroy(myclass);
	//注销设备
	cdev_del(&mycdev);
	platform_driver_unregister(&mydriver);
}

module_init(LED_Driver_Init);
module_exit(LED_Driver_exit);
MODULE_LICENSE("GPL");


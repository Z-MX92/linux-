#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_gpio.h>

struct device_node *mynode[2];//保存节点信息
enum of_gpio_flags myflags[5];//保存gpio的标志

dev_t mydev[5];
struct cdev mycdev;//linux2.6的核心结构体
struct file_operations myfops;//内核层的操作集合结构体
struct class * myclass;//类结构体

int i = 0;
int j = 0;
int gpio_pin_dev[5];//led1,led2,led21,led22,beep的gpio引脚编号
int gpio_act_dev[5];//led1,led2,led21,led22,beep的有效电平active

int Node_Info_Load(void);

int myopen(struct inode *inode, struct file *file)
{
	for(i = 0;i < 5;i++)
    {
        if(inode->i_rdev == mydev[i])
        break;
        else
        continue;
    }
	switch(i)
	{
		case 0:gpio_set_value(gpio_pin_dev[0], gpio_act_dev[0]);printk("led1_on\n");break;//LED1
		case 1:gpio_set_value(gpio_pin_dev[1], gpio_act_dev[1]);printk("led2_on\n");break;//LED2
		case 2:gpio_set_value(gpio_pin_dev[2], gpio_act_dev[2]);printk("led21_on\n");break;//LED21
		case 3:gpio_set_value(gpio_pin_dev[3], gpio_act_dev[3]);printk("led22_on\n");break;//LED22
		case 4:gpio_set_value(gpio_pin_dev[4], gpio_act_dev[4]);printk("beep_on\n");break;//BEEP
	}	
	return 0;
}

int myclose(struct inode *inode, struct file *file)
{
	for(i = 0;i < 5;i++)
	{
		if(inode->i_rdev == mydev[i])
		break;
		else
		continue;
	}
	switch(i)
	{
		case 0:gpio_set_value(gpio_pin_dev[0], !gpio_act_dev[0]);printk("led1_off\n");break;//LED1
		case 1:gpio_set_value(gpio_pin_dev[1], !gpio_act_dev[1]);printk("led2_off\n");break;//LED2
		case 2:gpio_set_value(gpio_pin_dev[2], !gpio_act_dev[2]);printk("led21_off\n");break;//LED21
		case 3:gpio_set_value(gpio_pin_dev[3], !gpio_act_dev[3]);printk("led22_off\n");break;//LED22
		case 4:gpio_set_value(gpio_pin_dev[4], !gpio_act_dev[4]);printk("beep_off\n");break;//BEEP
	}	
	return 0;
}

static int __init Mydev_init(void)
{
	
	//申请gpio
	gpio_request(gpio_pin_dev[0], "LED1");
	gpio_request(gpio_pin_dev[1], "LED2");
	gpio_request(gpio_pin_dev[2], "LED21");
	gpio_request(gpio_pin_dev[3], "LED22");
	gpio_request(gpio_pin_dev[4], "BEEP");
	for (i = 0; i < 5; i++)
	{
		gpio_direction_output(gpio_pin_dev[i], !gpio_act_dev[i]);
	}
	Node_Info_Load();//获取设备节点信息
	//使用linux2.6设备注册
	//1.向内核申请设备号
	alloc_chrdev_region(mydev, 0, 5, "mydev");
	for (i = 1; i < 5; i++)
	{
		mydev[i] = mydev[i - 1] + 1;
	}
	//2.初始化linux2.6核心结构体
	myfops.owner = THIS_MODULE;
	myfops.open = myopen;
	myfops.release = myclose;
	cdev_init(&mycdev, &myfops);
	//3.向内核注册核心结构体
	for (i = 0; i < 5; i++)
	{
		cdev_add(&mycdev, mydev[i], 1);
	}
	//4.创建类结构体
	myclass = class_create(THIS_MODULE, "myclass");
	//5.创建设备文件
	for (i = 0; i < 5; i++)
	{
		device_create(myclass, NULL, mydev[i], NULL, "my_dev%d", i);
	}
	
	return 0;
}

static void __exit Mydev_exit(void)
{
	//移除设备文件
	for (i = 0; i < 5; i++)
	{
		device_destroy(myclass, mydev[i]);
	}
	//删除类结构体
	class_destroy(myclass);
	//注销设备
	cdev_del(&mycdev);
}

int Node_Info_Load(void)
{
	int dev_index = 0;
	//查找设备节点
//	mynode = of_find_node_by_name(NULL, "my_led");//通过节点名字查找节点
//	mynode = of_find_compatible_node(NULL, NULL, "myled");//通过compatible查找节点	

	mynode[0] = of_find_node_by_path("/my_led");//通过节点的路径查找节点
	mynode[1] = of_find_node_by_path("/my_beep");
	if (mynode[0] || mynode[1] == NULL)
	{
		printk("设备节点不存在!\n");
		return -1;
	}
	//获取设备的gpio引脚编号、有效电平
	for (i = 0; i < 2; i++)//两个节点
	{
		for (j = 0; j < 4; j++)//led节点下有四个灯
		{
			if (dev_index > 4) continue;
			gpio_pin_dev[dev_index] = of_get_named_gpio_flags(mynode[i], "gpios", j, &myflags[dev_index]);
			if (myflags[dev_index] == OF_GPIO_ACTIVE_LOW)
			{
				gpio_act_dev[dev_index] = 0;
			}
			else
			{
				gpio_act_dev[dev_index] = 1;
			}
			printk("\n设备[%d]_pin = %d\n", dev_index, gpio_pin_dev[dev_index]);
			printk("有效电平 = %d\n", gpio_act_dev[dev_index]);
			dev_index ++;
		}
		

	}
	
	
	return 0;
}



module_init(Mydev_init);
module_exit(Mydev_exit);
MODULE_LICENSE("GPL");



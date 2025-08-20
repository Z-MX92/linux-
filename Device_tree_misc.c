#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/miscdevice.h>


struct device_node *mynode;//保存节点信息
enum of_gpio_flags myflags;//保存gpio的标志

struct file_operations myfops;
struct miscdevice mymisc;

int gpio_pin_led1;//led1的gpio引脚编号
int gpio_act_led1;//led1的有效电平active

int myopen(struct inode *inode, struct file *file)
{
	gpio_set_value(gpio_pin_led1, gpio_act_led1);
	printk("led1_on\n");
	return 0;
}

int myclose(struct inode *inode, struct file *file)
{
	gpio_set_value(gpio_pin_led1, !gpio_act_led1);
	printk("led1_off\n");
	return 0;
}

static int __init mydev_init(void)
{
	//查找设备节点
	mynode = of_find_node_by_path("/my_led");//通过节点的路径查找节点
//	mynode = of_find_node_by_name(NULL, "my_led");//通过节点名字查找节点
//	mynode = of_find_compatible_node(NULL, NULL, "myled");//通过compatible查找节点
	//获取设备的gpio引脚编号、有效电平
//	gpio_pin_led1 = of_get_named_gpio(mynode, "gpios", 0);//获取引脚编号
	gpio_pin_led1 = of_get_named_gpio_flags(mynode, "gpios", 0, &myflags);
	if (myflags == OF_GPIO_ACTIVE_LOW)
	{
		gpio_act_led1 = 0;
	}
	else
	{
		gpio_act_led1 = 1;
	}
	printk("gpio_pin_led1 = %d\n", gpio_pin_led1);
	printk("gpio_act_led1 = %d\n", gpio_act_led1);
	
	//申请gpio
	gpio_request(gpio_pin_led1, "LED1");
	gpio_direction_output(gpio_pin_led1, !gpio_act_led1);

	//使用杂项设备注册
	myfops.owner = THIS_MODULE;
	myfops.open = myopen;
	myfops.release = myclose;
	
	mymisc.minor = 255;
	mymisc.name = "LED1";
	mymisc.fops = &myfops;
	misc_register(&mymisc);
	return 0;
}

static void __exit mydev_exit(void)
{
	misc_deregister(&mymisc);
}

module_init(mydev_init);
module_exit(mydev_exit);
MODULE_LICENSE("GPL");



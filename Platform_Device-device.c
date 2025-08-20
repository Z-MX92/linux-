#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>

struct platform_device mydevice;
struct resource myresource[2];//资源结构体

void myrelease(struct device *dev){}

static int __init LED_Device_Init(void)
{
	//向内核注册平台设备总线设备端的资源信息
	myresource[0].start = 0;//led21的无效电平
	myresource[0].end = 1;//led22的有效电平
	myresource[0].name = "act_level";
	myresource[0].flags = IORESOURCE_MEM;
	
	myresource[1].start = 21;//led21的引脚编号
	myresource[1].end = 22;//led22的引脚编号
	myresource[1].name = "gpio_num";
	myresource[1].flags = IORESOURCE_MEM;

	mydevice.name = "myled";//用来匹配的name
	mydevice.dev.release = myrelease;//必须有实现
	mydevice.num_resources = 2;
	mydevice.resource = myresource;
	platform_device_register(&mydevice);

	return 0;
}

static void __exit LED_Device_exit(void)
{
	platform_device_unregister(&mydevice);
}

module_init(LED_Device_Init);
module_exit(LED_Device_exit);
MODULE_LICENSE("GPL");



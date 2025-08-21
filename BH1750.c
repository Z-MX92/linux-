#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>

struct i2c_driver myi2c_drv;
struct of_device_id myinfo = {.compatible = "mybh1750", };
struct i2c_client *myclient = NULL;

//linux2.6
dev_t mydev;
struct cdev mycdev;
struct file_operations myfops;
struct class *myclass;

int myopen(struct inode *inode, struct file *file)
{
	struct i2c_msg msg;
	u8 data = 0x10;//高频连续采集
	msg.addr = myclient->addr;
	msg.flags = 0;//0 -- 发送
	msg.buf = &data;
	msg.len = 1;//1个字节
	i2c_transfer(myclient->adapter, &msg, 1);//1个信息结构体
	return 0;
}

int myclose(struct inode *inode, struct file *file)
{
	return 0;
}

ssize_t myread(struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	int res = 0;
	struct i2c_msg msg;
	u8 data[2] = {0};
	msg.addr = myclient->addr;
	msg.flags = 1;
	msg.buf = data;
	msg.len = 2;//2个字节
	i2c_transfer(myclient->adapter, &msg, 1);//1个信息结构体

	res = copy_to_user(buf, data, sizeof(data));
	return 0;
}

int myprobe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int res = 0;
	printk("BH1750 加载中...\n");
	myclient = client;
	res = alloc_chrdev_region(&mydev, 0, 1, "mybh1750");

	myfops.owner = THIS_MODULE;
	myfops.open = myopen;
	myfops.release = myclose;
	myfops.read = myread;
	cdev_init(&mycdev, &myfops);

	res = cdev_add(&mycdev, mydev, 1);

	myclass = class_create(THIS_MODULE, "myclass");

	device_create(myclass, NULL, mydev, NULL, "mydev_bh1750");
	
	printk("BH1750 加载完成！\n");
	return 0;
}

int myremove(struct i2c_client *client)
{
	printk("BH1750 卸载中...\n");
	device_destroy(myclass, mydev);
	class_destroy(myclass);
	cdev_del(&mycdev);
	printk("BH1750 卸载完成！\n");
	return 0;
}

static int __init BH1750_Init(void)
{
	myi2c_drv.driver.name = "hello";
	myi2c_drv.driver.of_match_table = &myinfo;
	myi2c_drv.probe = myprobe;
	myi2c_drv.remove = myremove;
	i2c_add_driver(&myi2c_drv);
	return 0;
}

static void __exit BH1750_Exit(void)
{
	i2c_del_driver(&myi2c_drv);
}

module_init(BH1750_Init);
module_exit(BH1750_Exit);
MODULE_LICENSE("GPL");

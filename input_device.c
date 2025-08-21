#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

struct input_dev *myinput_dev = NULL;
struct platform_driver mydrv;
struct of_device_id myinfo = {.compatible = "mykey",};

int key_gpio_num, key_value, key_irq_num;

irqreturn_t KEY_IRQHandler(int num, void *arg)
{
	key_value = gpio_get_value(key_gpio_num);
	if (key_value == 0)//按键被按下
	{
		input_event(myinput_dev, EV_KEY, KEY_1, 1);
		//input_report_key(myinput_dev, KEY_1, 1);//专门用来上报按键
	}
	else if(key_value == 1)//按键释放
	{
		input_event(myinput_dev, EV_KEY, KEY_1, 0);
	}
	input_sync(myinput_dev);
	return 0;
}


int myprobe(struct platform_device *dev)
{
	int res = 0;
	key_gpio_num = of_get_named_gpio(dev->dev.of_node, "key_gpios", 0);

	gpio_request(key_gpio_num, "mykey");
	gpio_direction_input(key_gpio_num);

	key_irq_num = platform_get_irq(dev, 0);

	//enable_irq(key_irq_num);
	res = request_irq(key_irq_num, KEY_IRQHandler, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "key_irq", NULL);
	return 0;
}

int myremove(struct platform_device *dev)
{
	free_irq(key_irq_num, NULL);
	gpio_free(key_gpio_num);
	return 0;
}

static int __init Myinput_Init(void)
{
	int res = 0;
	myinput_dev = input_allocate_device();

	myinput_dev->name = "myinput";
	set_bit(EV_KEY, myinput_dev->evbit);
	set_bit(KEY_1, myinput_dev->keybit);
	res = input_register_device(myinput_dev);

	mydrv.driver.name = "myinput";
	mydrv.driver.of_match_table = &myinfo;
	mydrv.probe = myprobe;
	mydrv.remove = myremove;
	platform_driver_register(&mydrv);
	return 0;
}

static void __exit Myinput_Exit(void)
{
	platform_driver_unregister(&mydrv);
	input_unregister_device(myinput_dev);
	
}

module_init(Myinput_Init);
module_exit(Myinput_Exit);
MODULE_LICENSE("GPL");
	

/***************************************************************************//**
*  \file       driver_client.c
*
*  \details    Simple I2C driver explanation (SSD_1306 OLED Display Interface)
*
*  \author     Thundersoft
*
*  \Tested with Linux raspberrypi 5.4.51-v7l+
*
* *******************************************************************************/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/kernel.h>
 
#define I2C_BUS_AVAILABLE  (1)              // I2C Bus available in our Raspberry Pi
#define SLAVE_DEVICE_NAME  ("WM8960")              // Device and Driver Name
#define WM8960_SLAVE_ADDR  (0x34 )              // wm8960 Slave Address
 
static struct i2c_adapter *my_i2c_adapter     = NULL;  // I2C Adapter Structure
static struct i2c_client  *my_i2c_client_wm8960 = NULL;  // I2C Cient Structure

/*
** This function getting called when the slave has been found
** Note : This will be called only once when we load the driver.
*/
static int my_wm8960_probe(struct i2c_client *client,
                         const struct i2c_device_id *id)
{
    //TODO: Implement init of codec here
    pr_info("Wm8960 Probed!!!\n");
    
    return 0;
}
 
/*
** This function getting called when the slave has been removed
** Note : This will be called only once when we unload the driver.
*/
static int my_wm8960_remove(struct i2c_client *client)
{   
    //TODO: Implement deint of codec here
    pr_info("wm8960 Removed!!!\n");
    return 0;
}
 
/*
** Structure that has slave device id
*/
static const struct i2c_device_id my_wm8960_id[] = {
        { SLAVE_DEVICE_NAME, WM8960_SLAVE_ADDR },
        { }
};
MODULE_DEVICE_TABLE(i2c, my_wm8960_id);
 
/*
** I2C driver Structure that has to be added to linux
*/
static struct i2c_driver my_wm8960_codec_driver = {
		.class          = I2C_CLASS_DEPRECATED,
        .driver = {
            .name   = SLAVE_DEVICE_NAME,
            .owner  = THIS_MODULE,
        },
        .probe          = my_wm8960_probe,
        .remove         = my_wm8960_remove,
        .id_table       = my_wm8960_id,
};
 
/*
** I2C Board Info strucutre
*/
static struct i2c_board_info wm8960_i2c_board_info = {
        I2C_BOARD_INFO(SLAVE_DEVICE_NAME, WM8960_SLAVE_ADDR)
    };
 
/*
** Module Init function
*/
static int __init my_driver_init(void)
{
    int ret = -1;
    my_i2c_adapter = i2c_get_adapter(I2C_BUS_AVAILABLE);
    
    if( my_i2c_adapter != NULL )
    {
        my_i2c_client_wm8960 = i2c_new_client_device(my_i2c_adapter, &wm8960_i2c_board_info);
        
        if( my_i2c_client_wm8960 != NULL )
        {
            ret = i2c_add_driver(&my_wm8960_codec_driver);
            if(ret) {
				pr_info("Error: i2c_add_driver failed");				
			}
        }
		else
		{
			pr_info("Error: Device not added");
		}
        
        i2c_put_adapter(my_i2c_adapter);
		pr_info("Driver Added!!! ret %d\n",ret);
    }
	else
	{
		pr_info("Error: adapter not created");
	}
    
    return ret;
}
 
/*
** Module Exit function
*/
static void __exit my_driver_exit(void)
{
    i2c_unregister_device(my_i2c_client_wm8960);
    i2c_del_driver(&my_wm8960_codec_driver);
    pr_info("Driver Removed!!!\n");
}
 
module_init(my_driver_init);
module_exit(my_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("kisan yadav");
MODULE_DESCRIPTION("Simple I2C driver explanation (wm8960 codec)");
#include "max7219x4.h"

/* Initialization of the module */
static int __init ModuleInit(void) {

  //interface to SPI master controller
	struct spi_master *master;

  //device structure
	struct device *pDev;

  //register the character device
	majorNumber = register_chrdev(0, DEVICE_NAME, &fops);

  //check if registration is successful
  if (majorNumber < 0) {
    printk(KERN_ALERT "Error device registration failed with: %d\n", majorNumber);
    return majorNumber;
  }

  //minor and major number combination with the MKDEV macro
	min_maj = MKDEV(majorNumber, 0);  

	 // Create /sys/class/led_matrix_dev
  pClass = class_create(THIS_MODULE, "led_matrix_device");
  if (IS_ERR(pClass)) {
    printk(KERN_WARNING "\ncan't create class");
    unregister_chrdev_region(min_maj, 1);
    return -1;
  }

	// create /dev/max7219x4 file
  if (IS_ERR(pDev = device_create(pClass, NULL, min_maj, NULL, DEVICE_NAME))) {
    printk(KERN_WARNING "devko.ko can't create device /dev/kmem\n");
    class_destroy(pClass);
    unregister_chrdev_region(min_maj, 1);
    return -1;
  }

//display initialization hex codes for normal mode
u8 init_cmds[13][8] = {
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  {0x09, 0x00, 0x09, 0x00, 0x09, 0x00, 0x09, 0x00},
  {0x0a, 0x01,0x0a, 0x01,0x0a, 0x01, 0x0a, 0x01},
  {0x0b, 0x07, 0x0b, 0x07,0x0b, 0x07, 0x0b, 0x07},
  {0x0c, 0x01, 0x0c, 0x01,0x0c, 0x01, 0x0c, 0x01},
  {0x01, 0x00, 0x01, 0x00,0x01, 0x00, 0x01, 0x00},
  {0x02, 0x00, 0x02, 0x00 ,0x02, 0x00, 0x02, 0x00},
  {0x03, 0x00, 0x03, 0x00,0x03, 0x00, 0x03, 0x00},
  {0x04, 0x00, 0x04, 0x00,0x04, 0x00, 0x04, 0x00},
  {0x05, 0x00,0x05, 0x00 ,0x05, 0x00, 0x05, 0x00},
  {0x06, 0x00, 0x06, 0x00,0x06, 0x00, 0x06, 0x00},
  {0x07, 0x00, 0x07, 0x00,0x07, 0x00, 0x07, 0x00},
  {0x08, 0x00, 0x08, 0x00,0x08, 0x00, 0x08, 0x00}
};

	//spi device parameters 
	struct spi_board_info spi_device_info = {
		.modalias = "MAX7219",
		.max_speed_hz = 100000,
		.bus_num = BUS_NUM,
		.chip_select = 0,
		.mode = 0
	};

	//access spi bus
	master = spi_busnum_to_master(BUS_NUM);

	//can we access master?
	if(!master) {
		printk("Cannot acces no. %d bus\n", BUS_NUM);
		return -1;
	}

	//create a new spi device for max7219
	max7219 = spi_new_device(master, &spi_device_info);
	if(!max7219) {
		printk("Could not create device!\n");
		return -1;
	}

  //every word are 8 bits
	max7219 -> bits_per_word = 8;
	

	//bus setup for device
	if(spi_setup(max7219) != 0){
		printk("Could not change bus setup!\n");
		spi_unregister_device(max7219);
		return -1;
	}
	

  //send out the initialization hex codes
	int i_init;
  for(i_init = 0; i_init < 13; i_init++){
    spi_write(max7219, init_cmds[i_init] , sizeof(init_cmds[i_init]));
  }

	return 0;
}




//function being called when opening the device
static int device_open(struct inode *inode, struct file *filp)
{
  //check if device is opened
  if (is_device_open){
    return -EBUSY;
  }

  //set the device state to opened
  is_device_open++;

  //message on reading the file
  sprintf(msg, "\nThis device contains four max7219 led matrix panels. To use the driver send eight bytes to file /dev/max7219x4 in order. See the datasheet for more details.\n");
  msg_ptr = msg;

  try_module_get(THIS_MODULE);
  return 0;
}



//function being called when closing the device
static int device_release(struct inode *inode, struct file *filp)
{
  //device is not opened anymore
  is_device_open--;

  //module cannot be unloaded
  module_put(THIS_MODULE);

  return 0;
}


//function being called when reading from the device
static ssize_t device_read(struct file *filp, char *buffer,size_t length,loff_t *offset)
{
  //number of bytes written to the buffer
  int bytes_read = 0;

 //when end of message is reached
  if (*msg_ptr == 0)
    return 0;

  //put data into buffer
  while (length && *msg_ptr) {

    //copy from kernel data segment to user data segment
    put_user(*(msg_ptr++), buffer++);

    length--;
    bytes_read++;
  }

  //return number of bytes put into the buffer
  return bytes_read;
}

//function being called when writing to the device file
static ssize_t device_write(struct file *filp, const char *buf, size_t len, loff_t *off)
{


  


	//count the failed and the succeeded
    size_t datalen = 64, failed_to_copy;
    uint8_t data_buff[datalen];


    if (len < datalen) {
        datalen = len;
    }
    if(len > 64){
      printk(KERN_INFO"MAX7219 got too many bytes.");
    return -EFAULT;
    }
    
    //copy from user space to kernel space
    failed_to_copy = copy_from_user(data_buff, buf, datalen);
    data_buff[datalen] = 0;
    
    //if something failed to copy return with error
	if (failed_to_copy == 0) {

    //if not exactly 64 bytes return error
    if(datalen != 64){
      printk(KERN_INFO"MAX7219 driver got less bytes than expected, ignoring command.");
      return 	-EINVAL;
    }
  }else{
    //if some bytes failed to copy
    printk(KERN_INFO"MAX7219 driver lost some bytes, ignoring the command.");
    return -EFAULT;
  }
  
	
  //2d array for command bytes
  u8 cmd_bytes[8][8];

  //iteration variables
  int cmd_index_y;
  int cmd_index_x;
  int byte_index = 0;

  //placing bytes in right x and y values
  for (cmd_index_y = 0; cmd_index_y < 8; cmd_index_y++){
    for (cmd_index_x = 0; cmd_index_x < 8; cmd_index_x++){
      //command bytes masking and placing in the right position
        cmd_bytes[cmd_index_y][cmd_index_x % 8] = 0xFF & data_buff[byte_index];
        byte_index++;
    }

  }

  //send the bytes to the max7219s
  int cmd_index;
  for (cmd_index = 0; cmd_index < 8; cmd_index++){
    spi_write(max7219, cmd_bytes[cmd_index], sizeof(cmd_bytes[cmd_index]));
  }
	


  //return number of read from user
  return len;
}


//function being called on removing the module
static void __exit ModuleExit(void) {

	if(max7219){

    //turn off device and unregister if exists
    u8 uninit[] = {0x0C, 0x00, 0x0C, 0x00,0x0C, 0x00, 0x0C, 0x00};
    spi_write(max7219, uninit, sizeof(uninit));
		spi_unregister_device(max7219);
  }

  //remove /dev/max7219x4
  device_destroy(pClass, min_maj);  
  //remove /sys/class/led_matrix_device
  class_destroy(pClass);

  //unregister character device
  unregister_chrdev(majorNumber, DEVICE_NAME);  // Unregister the device

	printk("Device max7219 over SPI unregistered.\n");
}



//define init and exit functions
module_init(ModuleInit);
module_exit(ModuleExit);

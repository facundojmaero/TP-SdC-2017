#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/device.h>
#include <linux/cdev.h>

#define DEVICE_NAME "my_timer1"			//El dispositivo aparecera en /dev/my_timer1
#define CLASS_NAME  "my_timer_class"
#define	MSG_LEN 100

MODULE_LICENSE("GPL");

//Prototipos de funciones de fops y otras
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static void my_timer_startup(long timer_value);

static struct timer_list my_timer;
static struct class*  myCharClass  = NULL;
static struct device* myCharDevice = NULL;

static struct file_operations my_fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

static struct cdev *my_cdev;
static int major, timer_done;
static char msg[MSG_LEN];
static char *msg_Ptr;
static dev_t my_dev_t;

void my_timer_callback( unsigned long data )
{
	printk( "my_timer_callback called (%ld).\n", jiffies );
	timer_done = 1;
}

void my_timer_startup(long timer_value){
	int ret;
	printk( "Starting timer to fire in %lums (%ld)\n", timer_value, jiffies );
	ret = mod_timer( &my_timer, jiffies + msecs_to_jiffies(timer_value) );
	if (ret) printk("Error in mod_timer\n");
	timer_done = 0;
}

static int __init initialization_function(void)
{

  //obtengo major y minor para el driver
	int result;

	my_cdev = cdev_alloc();
	my_cdev->ops = &my_fops;
	my_cdev->owner = THIS_MODULE;

	major = alloc_chrdev_region(&my_dev_t, 0, 1, "my_timer1");

	printk("major -> %d, minor -> %d \n", MAJOR(my_dev_t), MINOR(my_dev_t));

	result = cdev_add(my_cdev, my_dev_t, 1);
	if(result < 0){
		printk("Error in cdev_add");
		unregister_chrdev_region(my_dev_t, 1);
		cdev_del(my_cdev);
		return result;
	}

  	// Registro la device class
	myCharClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(myCharClass)){                // Checkeo errores
		unregister_chrdev_region(my_dev_t, 1);
		cdev_del(my_cdev);
		printk(KERN_ALERT "Fallo al registrar la device class\n");
		return PTR_ERR(myCharClass);
	}

	// Registro el device driver
	myCharDevice = device_create(myCharClass, NULL, my_dev_t, NULL, DEVICE_NAME);
	if (IS_ERR(myCharDevice)){               // Checkeo errores
		unregister_chrdev_region(my_dev_t, 1);
		cdev_del(my_cdev);
		class_unregister(myCharClass);
		class_destroy(myCharClass);
		printk(KERN_ALERT "Fallo al crear el device\n");
		return PTR_ERR(myCharDevice);
	}

	setup_timer( &my_timer, my_timer_callback, 0 );

	printk(KERN_INFO "my_timer1: driver inicializado correctamente\n");

	return 0;
}
module_init(initialization_function);


static void __exit cleanup_function(void)
{
	int ret;

	ret = del_timer( &my_timer );
	if (ret) printk("The timer is still in use...\n");

	unregister_chrdev_region(my_dev_t, 1);
	cdev_del(my_cdev);
	device_unregister(myCharDevice);
	device_destroy(myCharClass, major);
	class_unregister(myCharClass);
	class_destroy(myCharClass);

	printk(KERN_INFO "my_timer1: modulo removido.\n");

	return;
}
module_exit(cleanup_function);


static int device_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t device_read(struct file *filp,
						   char *buffer,	// buffer a llenar con datos
						   size_t length,	// longitud del buffer
						   loff_t * offset){
	int error_count = 0;
   	// copy_to_user has the format ( * to, *from, size) and returns 0 on success

	printk("device read! timer done -> %d\n", timer_done);
	if(timer_done == 1){
		error_count = copy_to_user(buffer, msg, length);
		return error_count;	
	}
	else{
		char not_yet[20] = "Not yet...";
		error_count = copy_to_user(buffer, not_yet, 20);
		return error_count;		
	}
}

static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	int res;
	long timer_value;
	copy_from_user (msg, buff, len);
    //Hago que el puntero apunte al mensaje para cuando lo lea
	msg_Ptr = msg;
	printk(KERN_ALERT "Mensaje recibido -> %s\n", msg);

	res = kstrtol(msg, 10, &timer_value);
	if(res){
		printk("my_timer1 -> parsing error\n");
		return res;
	}
	
	my_timer_startup(timer_value);	

	printk("Conversion -> %lu\n", timer_value);

	return 0;
}

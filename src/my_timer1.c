/** @file my_timer.c
 *  @brief Main file that initiates the linux driver module. 
 *  It starts a timer after the user enters a given number. When it finishes it generates 
 *	an interrupt to notify the user. It also contains the function that uninstalls the module (module_exit).
 *
 *  @author Facundo Maero
 *  @author Agustin Colazo
 *	@author Gustavo Gonzalez
 *
 *  @bug No known bugs.
 */


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/string.h>

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
static void my_timer_callback( unsigned long data );

static struct timer_list my_timer;
static struct class*  myCharClass  = NULL;
static struct device* myCharDevice = NULL;

static struct file_operations my_fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};
/**< Esta estructura define las funciones que va a realizar el driver. */

static struct cdev *my_cdev;
static int major, timer_done, first_operation, interrupt_mode;
static char msg[MSG_LEN];
static char *msg_Ptr;
static dev_t my_dev_t;

//wait queue para alojar procesos bloqueados
static DECLARE_WAIT_QUEUE_HEAD(wq);

 /** @brief Funcion de inicializacion del modulo.
     *
     * Esta funcion es la llamada cuando se instala el modulo. 
     * La función de inicialización creará todas las estructuras de control necesarias para que el módulo funcione, 
     * se registrará con el Sistema Operativo para ser utilizable por el usuario, iniciará el módulo de Timer y avisará 
     * de su estado en los logs del Kernel.
     */
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

	//creo el timer
	setup_timer( &my_timer, my_timer_callback, 0 );

	printk(KERN_INFO "my_timer1: driver inicializado correctamente\n");

	return 0;
}
module_init(initialization_function);

 /** @brief Desinstala el modulo.
     *
     * Para limpiar el driver de nuestro sistema, es decir realizar una desinstalación correcta, es necesario deshacer todos los pasos
     * que se siguieron al instalarlo. Esto engloba desalojar los major y minor solicitados, eliminar la estructura cdev, desregistrar 
     * el device y su clase, y borrar las estructuras correspondientes:
     */
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
	first_operation = 0;
	return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
	first_operation = 0;
	return 0;
}

 /** @brief Funcion read del driver.
     *
     *
     *	Esta funcion envia un mensaje al espacio de usuario avisando si el timer termino o no.
     *
	 *
	 * @param filp Puntero a una estructura file.
	 * @param buff Buffer que se usa para pasar el mensaje.
	 * @param len Longitud del buffer
	 * @param off No se utiliza
     */
static ssize_t device_read(struct file *filp,
						   char *buffer,	// buffer a llenar con datos
						   size_t length,	// longitud del buffer
						   loff_t * offset){
	int error_count = 0;
   	// copy_to_user has the format ( * to, *from, size) and returns 0 on success

	if(interrupt_mode){
	   	printk(KERN_DEBUG "process %i (%s) going to sleep\n", current->pid, current->comm);
		wait_event_interruptible(wq, timer_done != 0);
		printk(KERN_DEBUG "awoken %i (%s)\n", current->pid, current->comm);
	}

	printk("device read! timer done -> %d\n", timer_done);
	if(timer_done == 1){
		char done[13] = "Timer ready!";
		error_count = copy_to_user(buffer, done, 13);
		return error_count;	
	}
	else{
		char not_yet[20] = "Not yet...";
		error_count = copy_to_user(buffer, not_yet, 20);
		return error_count;		
	}
}


 /** @brief Funcion write del driver.
     *
     * Cuando el usuario escribe en el driver, le pasa el tiempo en milisegundos para el timer. 
	 * Por lo tanto, se copia el contenido del buffer de usuario buff en el string msg en espacio de Kernel. 
	 * Luego se apunta msg_Ptr, un char pointer, al mensaje, para luego devolverlo al usuario en un read().
	 * La variable long timer_value guarda el tiempo a pasar al timer. Se extrae esta cantidad con la función simple_strtol(), 
	 * similar a atoi() en espacio de usuario. Finalmente se inicia el timer con la función wrapper my_timer_startup(), 
	 * que simplemente llama a mod_timer().
	 *
	 * @param filp Puntero a una estructura file.
	 * @param buff Buffer que contiene el numero que se le va a asignar al timer.
	 * @param len Longitud del buffer
	 * @param off No se utiliza
     */
static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	long mode;
	long timer_value;
	char* temp;

	copy_from_user (msg, buff, len);
    //Hago que el puntero apunte al mensaje para cuando lo lea

	if(first_operation == 0){
		first_operation = 1;

		mode = simple_strtol(msg, &temp , 10);
		if(mode == 3){
			interrupt_mode = 1;
			printk("Interrupt mode activated\n");
		}
		else{
			interrupt_mode = 0;
			printk("Interrupt mode deactivated\n");
		}

		return 0;
	}

	msg_Ptr = msg;
	printk(KERN_ALERT "Mensaje recibido -> %s", msg);

	timer_value = simple_strtol(msg, &temp , 10);

	my_timer_startup(timer_value);	
	return 0;
}

/** @brief Se ejecutara al finalizar la cuenta del timer.
	 *
     * Se imprime en el log del kernel que la cuenta termino.
     * Si un proceso estaba dormido esperando por el timer, se lo despierta.
	 *
*/
void my_timer_callback( unsigned long data )
{
	printk( "my_timer_callback called (%ld).\n", jiffies );
	timer_done = 1;
	
	if(interrupt_mode){
		printk(KERN_DEBUG "process %i (%s) awakening the readers...\n",	current->pid, current->comm);
		wake_up_interruptible(&wq);
	}
}

 /** @brief Se pasa el valor que se desea que cuente el timer.
	 *
     * 	Esto se hace con la función mod_timer y un instante en el futuro, dado por el valor 
     *	actual de jiffies más el tiempo deseado timer_value.
     * 
     * @timer_value Tiempo a setear el en el timer, en milisegundos
*/
void my_timer_startup(long timer_value){
	int ret;
	printk( "Starting timer to fire in %lums (%ld)\n", timer_value, jiffies );
	ret = mod_timer( &my_timer, jiffies + msecs_to_jiffies(timer_value) );
	if (ret) printk("Error in mod_timer\n");
	timer_done = 0;
}

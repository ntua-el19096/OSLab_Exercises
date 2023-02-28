/*
* lunix-chrdev.c
*
* Implementation of character devices
* for Lunix:TNG
*
*Despoina Tomkou el19181
*Stefanis Panagiotis el19096
*
*/

#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/cdev.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mmzone.h>
#include <linux/vmalloc.h>
#include <linux/spinlock.h>

#include "lunix.h"
#include "lunix-chrdev.h"
#include "lunix-lookup.h"

/*
* Global data
*/
struct cdev lunix_chrdev_cdev;

/*
* Just a quick [unlocked] check to see if the cached
* chrdev state needs to be updated from sensor measurements.
*/
/*
* Declare a prototype so we can define the "unused" attribute and keep
* the compiler happy. This function is not yet used, because this helpcode
* is a stub.
*/
//static int __attribute__((unused)) lunix_chrdev_state_needs_refresh(struct lunix_chrdev_state_struct *);
static int lunix_chrdev_state_needs_refresh(struct lunix_chrdev_state_struct *state) {

	struct lunix_sensor_struct *sensor;
	uint32_t previous_update, current_update;

	WARN_ON ( !(sensor = state->sensor));

	/* ? */
    previous_update = state->buf_timestamp;							//value contained in state struct (the buffered one)
    current_update = sensor->msr_data[state->type]->last_update;	//value contained in sensor struct (the last we got fron protocol)

    if (previous_update >= current_update) {                        //check operator for debugging
      debug("no new measurments");
      return 0;
    }
    else {
      debug("found some new measurments");
      return 1;
    }
}

/*
* Updates the cached state of a character device
* based on sensor data. Must be called with the
* character device state lock held.
 */
static int lunix_chrdev_state_update(struct lunix_chrdev_state_struct *state) {
	struct lunix_sensor_struct __attribute__((unused)) *sensor;
	uint32_t temp;
	long looked_up_val;

	debug("leaving\n");

	//spinlock
	spin_lock(&state->sensor->lock);

	//check for new available data
  	if(!(lunix_chrdev_state_needs_refresh(state))){
	   //release spinlock and return     semaphore is read's job
       spin_unlock(&state->sensor->lock);
       return -EAGAIN;
    }

    //hold new sensor measurements
	state->buf_timestamp = state->sensor->msr_data[state->type]->last_update;   //renew timestamp
	temp = state->sensor->msr_data[state->type]->values[0];                     // hold new measurments in a temp to convert them later

    //we dont need spinlock anymore
    spin_unlock(&state->sensor->lock);

    //Theloyme to temp meta thn metatroph na mpei sto buf_data[] poy xwraei 20 dyte
    switch(state->type) {
        case BATT:
        	looked_up_val = lookup_voltage[temp];
        	break;
        case TEMP:
        	looked_up_val = lookup_temperature[temp];
        	break;
        case LIGHT:
        	looked_up_val = lookup_light[temp];
        	break;
		default:
			return -EINVAL;
			break;
    }
  	state->buf_lim = snprintf(state->buf_data, LUNIX_CHRDEV_BUFSZ, "%ld.%03ld  ", looked_up_val/1000,
	(((looked_up_val%1000)<0) ? -(looked_up_val%1000): (looked_up_val%1000)));

	debug("leaving\n");
	return 0;
}

/*************************************
 * Implementation of file operations
 * for the Lunix character device
 *************************************/

static int lunix_chrdev_open(struct inode *inode, struct file *filp) {
	int ret;
	unsigned int file_minor;
	struct lunix_chrdev_state_struct *state;
    state = kzalloc(sizeof(*state), GFP_KERNEL);

	//check the allocation happened
	if(!state) {
		printk(KERN_ERR "Failed to allocate memory for measurement node\n");
		ret = -ENOMEM;
		goto out;
	}

	debug("entering\n");
	ret = -ENODEV;
	if ((ret = nonseekable_open(inode, filp)) < 0)
		goto out;

	/*
	 * Associate this open file with the relevant sensor based on
	 * the minor number of the device node [/dev/sensor<NO>-<TYPE>]
	 */

  	file_minor = iminor(inode);
  	state->sensor = &(lunix_sensors[file_minor/8]);
	switch(file_minor%8) {
      case 0:
        state->type = BATT;
        break;
      case 1:
        state->type = TEMP;
        break;
      case 2:
        state->type= LIGHT;
        break;
    }
  	sema_init(&(state->lock),1);
  	state->buf_lim = 0;
  	state->buf_timestamp = 0;
	/* Allocate a new Lunix character device private state structure */
	filp->private_data = state;

  	return 0;
out:
	debug("leaving, with ret = %d\n", ret);
	return ret;
}

static int lunix_chrdev_release(struct inode *inode, struct file *filp) {
	kfree(filp->private_data);
 	printk(KERN_INFO "Lunix chatacter device closed successfully\n");
	return 0;
}

static long lunix_chrdev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
	return -EINVAL;
}

static ssize_t lunix_chrdev_read(struct file *filp, char __user *usrbuf, size_t cnt, loff_t *f_pos) {
	ssize_t ret = 0 ;

	struct lunix_sensor_struct *sensor;
	struct lunix_chrdev_state_struct *state;

	state = filp->private_data;
	WARN_ON(!state);

	sensor = state->sensor;
	WARN_ON(!sensor);

	/* Lock? */
  	if(down_interruptible(&state->lock)){
		return -ERESTARTSYS;
	}

	/*
	 * If the cached character device state needs to be
	 * updated by actual sensor data (i.e. we need to report
	 * on a "fresh" measurement, do so
	 */
	if (*f_pos == 0) {
		while (lunix_chrdev_state_update(state) == -EAGAIN) {
            //unlock the sem
          	up(&state->lock);

            /* The process needs to sleep */
         	debug("Going to sleep!\n");
       	  	if (wait_event_interruptible(state->sensor->wq, lunix_chrdev_state_needs_refresh(state))) {
      	  		return -ERESTARTSYS;
     	  	}

            //reaquire the lock
    	  	if(down_interruptible(&state->lock)){
    			  return -ERESTARTSYS;
   	  	  	}
        }
    }

	/* End of file */
	if(*f_pos >= state->buf_lim){
      goto out;
    }

	/* Determine the number of cached bytes to copy to userspace */
	if(*f_pos + cnt >  state->buf_lim) {
      cnt =  state->buf_lim - *f_pos;
    }

    if (copy_to_user(usrbuf, state->buf_data + *f_pos, cnt)) {
      ret = -EFAULT;
      goto out;
    }

    *f_pos +=  cnt;

    /* Auto-rewind on EOF mode? */
	if (*f_pos==state->buf_lim){
       *f_pos=0;
    }

    ret = cnt;

out:
	/* Unlock? */
	debug("read successfully\n");
    up(&state->lock);
	return ret;
}

static int lunix_chrdev_mmap(struct file *filp, struct vm_area_struct *vma) {
	return -EINVAL;
}

static struct file_operations lunix_chrdev_fops = {
    .owner          = THIS_MODULE,
	.open           = lunix_chrdev_open,
	.release        = lunix_chrdev_release,
	.read           = lunix_chrdev_read,
	.unlocked_ioctl = lunix_chrdev_ioctl,
	.mmap           = lunix_chrdev_mmap
};

int lunix_chrdev_init(void) {
	/*
	 * Register the character device with the kernel, asking for
	 * a range of minor numbers (number of sensors * 8 measurements / sensor)
	 * beginning with LINUX_CHRDEV_MAJOR:0
	 */
	int ret;
	dev_t dev_no;
	unsigned int lunix_minor_cnt = lunix_sensor_cnt << 3;

	debug("initializing character device\n");
	cdev_init(&lunix_chrdev_cdev, &lunix_chrdev_fops);        //"Kernel I am here and these are my fops"
	lunix_chrdev_cdev.owner = THIS_MODULE;

	dev_no = MKDEV(LUNIX_CHRDEV_MAJOR, 0);
	/* register_chrdev_region */
	ret = register_chrdev_region(dev_no, lunix_minor_cnt, "lunix");   //registers device numbers
	if (ret < 0) {
		debug("failed to register region, ret = %d\n", ret);
		goto out;
	}

  	ret = cdev_add(&lunix_chrdev_cdev, dev_no, lunix_minor_cnt);
	if (ret < 0) {
		debug("failed to add character device\n");
		goto out_with_chrdev_region;
	}
	debug("completed successfully\n");
	return 0;

out_with_chrdev_region:
	unregister_chrdev_region(dev_no, lunix_minor_cnt);
out:
	return ret;
}

void lunix_chrdev_destroy(void) {
	dev_t dev_no;
	unsigned int lunix_minor_cnt = lunix_sensor_cnt << 3;

	debug("entering\n");
	dev_no = MKDEV(LUNIX_CHRDEV_MAJOR, 0);
	cdev_del(&lunix_chrdev_cdev);
	unregister_chrdev_region(dev_no, lunix_minor_cnt);
	debug("leaving\n");
}

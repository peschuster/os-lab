#include <linux/module.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");

#define DEVICE_NAME "fifo"
#define BUF_LEN 80

static int fifo_major; // Major device number

typedef struct {
  char msg[BUF_LEN]; // Message buffer - i.e. "fifo"
  int pread;   // Read pointer
  int pwrite;  // Write ponter
  wait_queue_head_t read_wait;  // Wait queue for reading
  wait_queue_head_t write_wait; // Wait queue for writing
  int creaders;
  int cwriters;
} fifo_proc_t;

static fifo_proc_t fp[2];

// this method is executed when reading from the module
static ssize_t fifo_module_read( struct file *file, char *buf, size_t count,
						 loff_t *ppos ) {
  int fpi, minor, max_read, read_count, i;
  
  if (count <= 0)
    return 0;
					 
  minor = MINOR(file->f_dentry->d_inode->i_rdev);
  printk("Opened %d for reading\n", minor);
  if (minor % 2 == 0)
    return 0; // device is only for writing

  fpi = minor / 2;
  
  if (fp[fpi].cwriters == 0 && fp[fpi].pread == fp[fpi].pwrite) {
    return 0;    // EOF
  }
 
  wait_event_interruptible(fp[fpi].read_wait, fp[fpi].pread != fp[fpi].pwrite);
  
  max_read = fp[fpi].pwrite > fp[fpi].pread ? (fp[fpi].pwrite - fp[fpi].pread) : (fp[fpi].pwrite + BUF_LEN - fp[fpi].pread);
  read_count = (max_read <= count) ? max_read : count;
  
  for (i = 0; i < read_count; i++) {
    put_user(fp[fpi].msg[fp[fpi].pread], buf + i);
    fp[fpi].pread++;
    
    if (fp[fpi].pread == BUF_LEN)
      fp[fpi].pread = 0;
  }
  
  wake_up(&fp[fpi].write_wait);
  return read_count;
}

// this method is executed when writing to the module
static ssize_t fifo_module_write( struct file *file, const char *buf, size_t count,
						 loff_t *ppos ) {
  int fpi, minor, i;
  
  minor = MINOR(file->f_dentry->d_inode->i_rdev);
  printk("Opened %d for writing\n", minor);
  if (minor % 2 == 1) {
    printk("Closing...  %d is only for reading\n", minor);
    return count; // device is only for reading
  }

  fpi = minor / 2;
  
  for (i = 0; i < count; i++) {

    if ((fp[fpi].pwrite+1) == fp[fpi].pread || (fp[fpi].pread == 0 && fp[fpi].pwrite == (BUF_LEN - 1))) {
      wait_event_interruptible(fp[fpi].write_wait, (fp[fpi].pwrite+1) == fp[fpi].pread || (fp[fpi].pread == 0 && fp[fpi].pwrite == (BUF_LEN - 1)));
    }
    
    get_user(fp[fpi].msg[fp[fpi].pwrite], (buf + i));
    fp[fpi].pwrite++;
    
    if (fp[fpi].pwrite == BUF_LEN)
      fp[fpi].pwrite = 0;
  }
  
  wake_up(&fp[fpi].read_wait);
  return count;
}

// this method is called whenever the module is being used
// e.g. for both read and write operations
static int fifo_module_open(struct inode * inode, struct file * file) {
  int fpi, minor;
  
  minor = MINOR(file->f_dentry->d_inode->i_rdev);
  fpi = minor / 2;
  
  if (minor % 2 == 1) {
    // reader
    fp[fpi].creaders++;
  } else {
    // writer
    fp[fpi].cwriters++;
  }
  
  return 0;
}

// this method releases the module and makes it available for new
// operations
static int fifo_module_release(struct inode * inode, struct file * file) {
  int fpi, minor;
  
  minor = MINOR(file->f_dentry->d_inode->i_rdev);
  fpi = minor / 2;
  
  if (minor % 2 == 1) {
    // reader
    fp[fpi].creaders--;
  } else {
    // writer
    fp[fpi].cwriters--;
  }
  
  return 0;
}

// module's file operations
static struct file_operations fifo_module_fops = {
	.owner =	THIS_MODULE,
	.read =		fifo_module_read,
	.write =	fifo_module_write,
	.open =		fifo_module_open,
	.release =	fifo_module_release,
};

// initialize module (executed when using insmod)
int __init init_fifo_module(void) {
  fifo_major = register_chrdev(0, DEVICE_NAME, &fifo_module_fops);

  if (fifo_major < 0) {
    printk ("Registering fifo failed: %d\n", fifo_major);
    return fifo_major;
  } else {
    printk ("Registered fifo device. Major number: %d\n", fifo_major);
  }
  
  init_waitqueue_head(&fp[0].read_wait);
  init_waitqueue_head(&fp[0].write_wait);
  init_waitqueue_head(&fp[1].read_wait);
  init_waitqueue_head(&fp[1].write_wait);
  fp[0].pread = fp[0].pwrite = 0;
  fp[1].pread = fp[1].pwrite = 0;
  fp[0].creaders = fp[0].cwriters = 0;
  fp[1].creaders = fp[1].cwriters = 0;

  return 0;
}

// cleanup module (executed when using rmmod)
void __exit cleanup_fifo_module(void) {
  unregister_chrdev(fifo_major, DEVICE_NAME);
}

module_init(init_fifo_module);
module_exit(cleanup_fifo_module);


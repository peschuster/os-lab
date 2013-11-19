#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/time.h>

MODULE_LICENSE("GPL");

struct proc_dir_entry* proc_clock;

/* read_times is required to prevent endless reads */
int read_times = 0;

ssize_t mod_clock_read(struct file* f, char *buf, size_t count, loff_t *offp ) 
{
  ssize_t len = 0;
  
  /* only output data, if offset == 0 and read_times was reset. */
  if (*offp == 0 && read_times == 0) {
    struct timeval tv;
    struct tm t;
    
    do_gettimeofday(&tv);
    
    time_to_tm(tv.tv_sec, 0, &t);
    len += sprintf(buf + len, "%d:%d:%d:%ld\n", 
                    t.tm_hour, t.tm_min, t.tm_sec, tv.tv_usec);
  }
  
  read_times += 1;
  
  /* We're returing '0' 
    -> this stops the current read process
      -> next time a read as allowed 
        -> reset 'read_times'. */
  if (len == 0) {
    read_times = 0;
  }
   
  return len;
}

struct file_operations mod_clock_fops = {
 owner: THIS_MODULE,
 read: mod_clock_read
};

static int __init mod_clock_init(void) {
  proc_clock = proc_create("clock", 0, NULL, &mod_clock_fops);
  
  return 0;
}

static void __exit mod_clock_dispose(void) {
  remove_proc_entry("clock", NULL);
}

module_init(mod_clock_init);
module_exit(mod_clock_dispose);

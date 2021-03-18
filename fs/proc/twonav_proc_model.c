#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include<linux/sched.h>
#include <asm/uaccess.h>

extern char* tn_hwtype;

static int len, temp;

static int twonav_fake_devicetree_dir(void) 
{	
	struct proc_dir_entry *parent;
	
	parent = proc_mkdir("device-tree", NULL);
	if (!parent)
		return -ENOMEM;	

	return 0;
};

static int twonav_read_proc(struct file *filp, char *buf, size_t count, loff_t *offp) 
{
	if(count > temp)
		count=temp;
	
	temp= temp-count;

	if(tn_hwtype)
		copy_to_user(buf, tn_hwtype, count);
	
	if(count == 0)
		temp = len;
	   
	return count;
}

static struct file_operations twonav_proc_fops = {
	read: twonav_read_proc
};

static void twonav_create_new_proc_entry(void) 
{
	int res = twonav_fake_devicetree_dir();
	if(res == 0) {
		proc_create("device-tree/model", 0, NULL, &twonav_proc_fops);
		if(tn_hwtype) {
			len = strlen(tn_hwtype);
			temp = len;
		}
	}	
}


int twonav_proc_init (void) {
	twonav_create_new_proc_entry();
 	return 0;
}

void twonav_proc_cleanup(void) {
	remove_proc_entry("device-tree/model", NULL);
	remove_proc_entry("device-tree", NULL);
}

MODULE_LICENSE("GPL"); 
module_init(twonav_proc_init);
module_exit(twonav_proc_cleanup);
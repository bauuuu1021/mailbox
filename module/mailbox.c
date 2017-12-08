#include "mailbox.h"
#include <linux/slab.h>

MODULE_LICENSE("Dual BSD/GPL");

static void get_process_name(char *ouput_name);
static ssize_t mailbox_read(struct kobject *kobj,
                            struct kobj_attribute *attr, char *buf);
static ssize_t mailbox_write(struct kobject *kobj,
                             struct kobj_attribute *attr, const char *buf, size_t count);

static struct kobject *hw2_kobject;
static struct kobj_attribute mailbox_attribute
    = __ATTR(mailbox, 0660, mailbox_read, mailbox_write);

static int num_entry_max = 2;
struct mailbox_entry_t *data;
int currentNum=0;

module_param(num_entry_max, int, S_IRUGO);

static void get_process_name(char *ouput_name)
{
	memcpy(ouput_name, current->comm, sizeof(current->comm));
}

static ssize_t mailbox_read(struct kobject *kobj,
                            struct kobj_attribute *attr, char *buf)
{
	if (currentNum==0)
		return ERR_EMPTY;
	currentNum--;   //-1 before accessing
	char temp[100];
	memset(temp,0,strlen(temp));

	//combine all information in struct into a string
	snprintf(temp,sizeof(temp),"%s %s %s",data[currentNum].query_word,
	         data[currentNum].word_count,data[currentNum].file_path);
	printk("[read]:%s\n",temp);
	strcpy(buf,temp);

	return strlen(buf);
}

static ssize_t mailbox_write(struct kobject *kobj,
                             struct kobj_attribute *attr, const char *buf, size_t count)
{
	if (currentNum==num_entry_max)
		return ERR_FULL;

	//seperate the string and out info. in correspond element in struct
	sscanf(buf,"%s %s %s",data[currentNum].query_word,data[currentNum].word_count,
	       data[currentNum].file_path);
	printk("[write]:%s\n",buf);
	currentNum++;   //+1 after write something

	return count;
}

static int __init mailbox_init(void)
{
	printk("Insert, num_entry_max=%d\n",num_entry_max);
	hw2_kobject = kobject_create_and_add("hw2", kernel_kobj);
	sysfs_create_file(hw2_kobject, &mailbox_attribute.attr);

	//build doubly linked list
	int i;
	data=kmalloc(num_entry_max * (sizeof(
	                                  struct mailbox_entry_t)),GFP_KERNEL);
	for (i=0; i<num_entry_max; i++) {
		int temp=((i-1)%num_entry_max)>0?((i-1)%num_entry_max):((i-1)%num_entry_max) *
		         (-1);
		data[i].entry.next=&data[(i+1)%num_entry_max];
		data[i].entry.prev=&data[temp];
		printk("malloc done\n");
	}
	return 0;
}
static void __exit mailbox_exit(void)
{
	printk("Remove\n");
	kfree(data);
	kobject_put(hw2_kobject);
}

module_init(mailbox_init);
module_exit(mailbox_exit);

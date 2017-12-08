#ifndef MAILBOX_H
#define MAILBOX_H

#include <linux/module.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/fs.h>
#include <linux/spinlock_types.h>

#define ERR_EMPTY -1
#define ERR_FULL -2

struct mailbox_entry_t {
	char query_word[32];
	char word_count[3];     //to make data transform easy
	char file_path[4096];
	struct list_head entry;
};

#endif

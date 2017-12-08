#include "slave.h"
#include <stdio.h>
#include <fcntl.h>

struct mail_t mail;
int repeat=0;    //flag, 0 not happen, 1 happen

int main(int argc, char **argv)
{
	printf("[slave]exec success\n");

	int count=0;
	struct mail_t mail;
	char temp[32];
	int backupCount;//if the mail has been counted, recover the previous value
	int fd=open("/sys/kernel/hw2/mailbox",O_RDWR);
	if (fd==-1)
		printf("[slave]fd error\n");

	while (!receive_from_fd(fd,&mail));   //continue receiving if EMPTY

	backupCount=mail.data.word_count;
	if (repeat==0) {  //if the mail has not counted yet
		count=0;
		FILE * fp=fopen(mail.file_path,"r");
		if (!fp)
			printf("[slave]file open failed\n");

		while ( fscanf(fp,"%s ", temp)!= EOF)
			if (!strcmp(temp,mail.data.query_word))     //compare two string
				count++;    //same

		mail.data.word_count=count;     //put result into struct

	} else if (repeat)  //the mail has been visited, backup previous value
		mail.data.word_count=backupCount;

	repeat=0;

	//send to kernel
	fd=open("/sys/kernel/hw2/mailbox",O_RDWR);
	while (!send_to_fd(fd,&mail));
	sleep(1);

}

int send_to_fd(int sysfs_fd, struct mail_t *mail)
{

	int ret_val;
	char sendBuf[4096];

	memset(sendBuf,'\0',sizeof(sendBuf));
	//combine all info. into string, ignore query_word
	snprintf(sendBuf,sizeof(sendBuf),"xx %u %s",mail->data.word_count,
	         mail->file_path);

	ret_val = write(sysfs_fd,sendBuf,sizeof(sendBuf));
	if (ret_val == ERR_FULL) {
		printf("kernel buffer is full\n");
		return 0;
	}

}

int receive_from_fd(int sysfs_fd, struct mail_t *mail)
{

	char buf[100];
	char correct[100];
	memset(correct,'\0',sizeof(correct));
	memset(mail->data.query_word,'\0',sizeof(mail->data.query_word));

	//get the strlen of buf by ret_val
	int ret_val = read(sysfs_fd,buf,sizeof(buf));
	if (ret_val == ERR_EMPTY) {
		printf("kernel buffer is empty\n");
		return 0;
	}

	strncpy(correct,buf,
	        ret_val);   //copy the correct part (lenth) of buf[] to correct[]

	//put info. that get from kernel into struct mail_t
	char countC[5];  //master send xx, ignore it
	sscanf(correct,"%s %s %s",mail->data.query_word,countC,
	       mail->file_path);

	//if what just received is a counted struct
	if (!strcmp(mail->data.query_word,"xx")) {  //has been counted
		mail->data.word_count=atoi(countC);
		repeat=1; //flag
	}
}

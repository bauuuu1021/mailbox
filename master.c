#include <unistd.h>
#include <fcntl.h>
#include "master.h"
#include <signal.h>
#include <dirent.h>

int main(int argc, char **argv)
{

	int i;
	struct mail_t mail;
	int inputWord=0,inputDir=0,inputSlave=0;	//record where are the inputs in argv[]
	for (i=1; i<argc;) {

		if (!strcmp(argv[i],"-d"))
			inputDir=(i+1);
		else if (!strcmp(argv[i],"-s"))
			inputSlave=(i+1);
		else if (!strcmp(argv[i],"-q"))
			inputWord=(i+1);

		i+=2;
	}

	int forkTime = (inputSlave>0?atoi(argv[inputSlave]):1);
	pid_t * pid = malloc(forkTime * sizeof (pid_t));
	for (i=0; i<forkTime; i++) { //fork() as many slaves as input request
		pid[i]=fork();

		if (pid[i]==0)
			execl("/home/user/hw2_mailbox/slave",NULL); //absolute slave path
		sleep(0.5);
	}

	//put information in argv into struct
	strcpy(mail.data.query_word,argv[inputWord]);
	//get sysfs_fd to connect with kernel
	int fd=open("/sys/kernel/hw2/mailbox",O_RDWR);
	if (fd==-1)
		printf("[master]fd error\n");

	DIR *dir,*newDir;
	struct dirent *ent,*newEnt;
	char path[100];
	strcpy(path,argv[inputDir]);
	char newPath[100];

	if ((dir=opendir(path))==NULL)
		printf("[master]fd error, please check if path end up with '/'\n");

	while ((ent=readdir(dir))!=NULL) {
		strcpy(newPath,path);   //cp the path to avoid missing current address
		strcat(newPath,ent->d_name);    //combine current path and file name
		/*
				//Dir
				if ((newDir=opendir(newPath))!=NULL) {
					printf("***dir:%s***\n",newPath);
					while ((newEnt=readdir(newDir))!=NULL)
						printf("*****%s\n",ent->d_name);
					fd=open("/sys/kernel/hw2/mailbox",O_RDWR);
					strcat(newPath,newEnt->d_name);
		            strcpy(mail.file_path,newPath);
					while (!send_to_fd(fd,&mail));  //continue send if FULL

				}
		*/
		//file
		if ((newDir=opendir(newPath))==NULL)  { //is a file
			printf("[master]file name:%s\n",ent->d_name);
			fd=open("/sys/kernel/hw2/mailbox",O_RDWR);
			strcpy(mail.file_path,newPath);
			while (!send_to_fd(fd,&mail));  //continue send if FULL

		}
	}
	closedir(dir);

	sleep(1);   //avoid accessing data just pass

	fd=open("/sys/kernel/hw2/mailbox",O_RDWR);
	while (!receive_from_fd(fd,
	                        &mail)); //continue to receive if EMPTY

	for (i=0; i<forkTime; i++) {
		if (!kill(pid[i],9))    //killed successfully
			printf("[master]killed successfully\n");
		else
			printf("[master]killed fail\n");
	}
}   //end main()


int send_to_fd(int sysfs_fd, struct mail_t *mail)
{

	char sendBuf[4096];
	memset(sendBuf,'\0',sizeof(sendBuf));

	//combine all info. into a string, ignore word_count
	snprintf(sendBuf,sizeof(sendBuf),"%s xx %s",mail->data.query_word,
	         mail->file_path);

	int ret_val = write(sysfs_fd,sendBuf,sizeof(sendBuf));
	if (ret_val == ERR_FULL) {
		printf("kernel buffer is full\n");
		return 0;
	}

}


int receive_from_fd(int sysfs_fd, struct mail_t *mail)

{

	//buf to get string from kernel
	//correct to modify the string
	char buf[100];
	char correct[100];

	memset(correct,'\0',sizeof(correct));
	//get the strlen of buf by ret_val
	int ret_val = read(sysfs_fd,buf,sizeof(buf));
	if (ret_val == ERR_EMPTY) {
		printf("kernel buffer is empty\n");
		return 0;
	}

	strncpy(correct,buf,
	        ret_val);   //copy the correct part (lenth) of buf[] to correct[]

	//put info. that get from kernel into struct mail_t
	char countC[5];
	sscanf(correct,"%s %s %s",mail->data.query_word,countC,
	       mail->file_path);

	printf("************************************************\n");
	printf("count:%s\n",countC);
	printf("path:%s\n",mail->file_path);
	printf("************************************************\n");

}

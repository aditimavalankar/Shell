#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/utsname.h>
#include<sys/wait.h>
#include<string.h>
#include<pwd.h>
#include<time.h>
#include<stdio.h>
#include<signal.h>
#include<dirent.h>
struct utsname u;
struct passwd *p;
struct stat STAT;
typedef struct node
{
	int pid;
	int status;
	char *name;
}node;
node f[100]; //foreground array
node b[100];//background array
int cntf=0,cntb=0;
char name[50];
char *directory;
int FLAG; //to decide whether to print prompt or not
pid_t P;
void child(int n)
{
	int s;
	int pd;
	pd=waitpid(WAIT_ANY,&s,WNOHANG);
	int i;
	for(i=0;i<cntb;i++)
	{
		if(b[i].pid==pd)
		{
			printf("\n%s with pid %d exited normally.\n",b[i].name,b[i].pid);
			b[i].status=0;
			if(FLAG==0)
			{
				char dir[200];
				getcwd(dir,200);
				char *n=strstr(dir,directory);
				char *n2=dir;
				write(2,"<",2);
				write(2,p->pw_name,strlen(p->pw_name));
				write(2,":",2);
				write(2,name,strlen(name));
				write(2,":",2);
				int constant=strlen(directory);
				if(n!=NULL)
				{
					n2+=constant;
					write(2,"~",2);
					write(2,n2,strlen(n2));
				}
				else
					write(2,dir,strlen(dir));
				write(2,">",2);
			}
		}
	}
	signal(SIGCHLD,child);
	return;
}
void sig(int n)
{
	if(n==2 || n==3)
	{
		signal(SIGINT,sig);
		signal(SIGQUIT,sig);
		//signal(SIGTSTP,sig);
		fflush(stdout);
	}
	if(n==20) //for ctrl+z, want to shift process from fore to back
	{
		kill(P,SIGTSTP);
		signal(SIGTSTP,sig);
	}
	fflush(stdout);
	return;
}
int main()
{
	int id=getuid();
	int i,j,x;
	//char *command=malloc(sizeof(char)*200);
	p=getpwuid(id);
	char command[1000];
	int count,cnt;
	pid_t pid;
	char ch[2];
	int flag;// to decide if process if foreground or background
	gethostname(name,50);
	int n1=strlen(p->pw_name);
	int n2=strlen(name);
	char dir[200];
	int m,s;
	FILE *file;
	directory=getenv("PWD");//the initial home directory in which user logs in
	while(1)
	{
		flag=0; 
		FLAG=0;
		count=0;
		cnt=0;
		signal(SIGINT,SIG_IGN);
		signal(SIGINT,sig);
		signal(SIGCHLD,SIG_IGN);
		signal(SIGCHLD,child);
		signal(SIGQUIT,SIG_IGN);
		signal(SIGQUIT,sig);
		signal(SIGTSTP,SIG_IGN);
		signal(SIGTSTP,sig);
		printf("<%s@%s:",p->pw_name,name);
		getcwd(dir,200);
		char *n=strstr(dir,directory);
		char *n2=dir;
		int constant=strlen(directory);
		if(n!=NULL)
		{
			n2+=constant; //takes length of home, skips those many characters, replaces with tilda
			printf("~%s",n2);
		}
		else
			printf("%s",dir);
		printf(">");
		scanf("%[^\n]",command);
		x=getchar();
		char command2[100];
		strcpy(command2,command);
		if(!strlen(command))
			continue;
		cnt=0;
		char *n1=strtok(command," "); //parses the input by space, need to pass the words in an array
		char **arr=malloc(sizeof(char *)*1);
		arr[0]=malloc(sizeof(char **)*100);
		while(n1!=NULL)
		{			
			arr[cnt]=n1;
			arr[cnt][strlen(n1)]='\0';
			n1=strtok(NULL," ");
			cnt++;
		}
		arr[cnt++]=n1;
		if(strcmp(arr[0],"quit")==0) //condition (only) to quit terminal
		{
			exit(0);
		}
		node nod;
		if(cnt>2)
		{
			if(strcmp(arr[cnt-2],"&")==0)
			{
				arr[cnt-2]=NULL;
				flag=1;			//for background process
			}
		}
		for(i=0;i<strlen(command2);i++)
		{
			if(command2[i]=='<')
				flag=2;			//for input redirection only
		}
		for(i=0;i<strlen(command2);i++)
		{
			if(command2[i]=='>')
			{
				if(flag==2)
				{
					flag=25;		//for both input and output redirection
					break;
				}
				else
				{
					flag=3;			//for output redirection only
					break;
				}
			}
		}
		for(i=0;i<strlen(command2);i++)
		{
			if(command2[i]=='|')
			{
				if(flag==2)
				{
					flag=41;			//for piping with input redirection
					break;
				}
				else if(flag==3)
				{
					flag=42;			//for piping with output redirection
					break;
				}
				else if(flag==25)
				{
					flag=43;
					break;
				}
				else
				{
					flag=4;				//for piping only
					break;
				}
			}
		}
//		printf("flag=%d\n",flag);
		if(strcmp(arr[0],"jobs")==0 && flag==0) //shows background processes
		{
			if(cnt!=2)
			{
				perror("Incorrect syntax of command\n");
				continue;
			}
			int co=1;
			for(i=0;i<cntb;i++)
				if(b[i].status==1)
				{
					printf("[%d]\t%s\t[%d]\n",co,b[i].name,b[i].pid);
					co++;
				}
			continue;
		}
		if(strcmp(arr[0],"kjob")==0 && flag==0) //kills job with given id
		{
			int cou=0;
			int fla=0;
			if(cnt==4)
			{
				for(i=0;i<cntb;i++)
				{
					if(b[i].status==1)
						cou++;
					if(cou==atoi(arr[1]))
					{
						fla=1;
						break;
					}
				}
				if(fla==0)
					perror("No such process exists\n");
				else
					kill(b[i].pid,atoi(arr[2]));
	//			if(cnt==4)
	//				kill(b[atoi(arr[1])-1].pid,atoi(arr[2]));
			}
			else
				perror("Incorrect number of arguments\n");
			continue;
		}
		if(strcmp(arr[0],"cd")==0 && flag==0)
		{
			if(cnt==2)
				chdir(directory);
			else if(strcmp(arr[1],"~")==0 || strcmp(arr[1],"~/")==0)
				chdir(directory);
			else if(chdir(arr[1])>=0);
			else
				printf("No such directory exists\n");
			continue;
		}
		if(strcmp(arr[0],"overkill")==0 && flag==0) //kills all processes
		{
			FLAG=1;
			if(cnt!=2)
			{
				perror("Incorrect command\n");
				continue;
			}
			for(i=0;i<cntb;i++)
			{
				kill(b[i].pid,9);
			}
			continue;
		}
		if(strcmp(arr[0],"fg")==0 && flag==0) //bring back process to fore
		{
			int coun=0,fl=0;
			if(cnt!=3)
			{
				perror("Incorrect number of arguments given\n");
				continue;
			}
			for(i=0;i<cntb;i++)
			{
				if(b[i].status==1)
					coun++;
				if(coun==atoi(arr[1]))
				{
					fl=1;
					break;
				}
			}
			if(fl==0)
			{
				perror("Process not found\n");
				continue;
			}
			pid=waitpid(b[i].pid,&s,0);
			b[(atoi)(arr[1])].status=0;
		}
		if(strcmp(arr[0],"pinfo")==0) //process info provided
		{
			if(cnt==2)
				pid=getpid();
			else if(cnt==3)
				pid=atoi(arr[1]);
			else
			{
				perror("Incorrect number of arguments\n");
				continue;
			}
			char PID[6];
			//printf("Pid:\t\t\t%d\n",pid);
			int prod=1;
			while(pid/prod!=0)
				prod*=10;
			prod/=10;
			i=0;
			while(prod!=0)
			{
				PID[i++]=(pid/prod)%10+'0';
				prod/=10;
			}
			PID[i]='\0';
			char file[30],temporary[20];
			strcpy(file,"/proc/");
			strcpy(temporary,strcat(PID,"/stat"));
			strcpy(file,strcat(file,temporary));
			int t=open(file,O_RDONLY);
			if(t<0)
			{
				printf("No such pid exists\n");
				continue;
			}
			i=0;
			char ch;
			printf("Pid:\t\t\t%d\n",pid);
			while(read(t,&ch,1))
			{
				//read(t,&ch,1);
				if(ch==' ')
					i++;
				if(i==1)
				{
					printf("Executable Path:\t");
					read(t,&ch,1);
					while(ch!=' ')
					{
						printf("%c",ch);
						read(t,&ch,1);
					}
					i++;
					printf("\n");
				}
				if(i==2)
				{
					printf("Process status:\t\t");
					read(t,&ch,1);
					while(ch!=' ')
					{
						printf("%c",ch);
						read(t,&ch,1);
					}
					i++;
					printf("\n");
				}
				if(i==22)
				{
					printf("Memory:\t\t\t");
					read(t,&ch,1);
					while(ch!=' ')
					{
						printf("%c",ch);
						read(t,&ch,1);
					}
					printf("\n");
					break;
				}
			}
			continue;
		}
		pid_t pid2;
		char *final[100];
		int in,out;
		if(flag==0 || flag==1)
		{
			pid=fork();
			if(pid<0)
			{
				write(2,"Fork unsuccessful. Exiting...\n",strlen("Fork unsuccessful. Exiting...\n"));
				exit(-1);
			}
			else if(pid==0)
			{
				if(execvp(arr[0],arr)<0)//runs command by passing two arrays
				{
					printf("Command not found\n");
					continue;
				}
			}
			else
			{
				char *pointer;
				if(flag==0)
				{
					P=pid;
					f[cntf].pid=pid;
					f[cntf].status=0;
					f[cntf].name=malloc(sizeof(char)*100);
					strcpy(f[cntf].name,command2);
					cntf++;
					waitpid(pid,&s,WUNTRACED);
					if(WIFSTOPPED(s))
					{
						b[cntb].pid=P;
						b[cntb].status=1; //changing status of process, fore process becomes back process
						b[cntb].name=malloc(sizeof(char)*100);
						strcpy(b[cntb].name,command);
						cntb++;
					}
				}
				else if(flag==1)
				{
					P=pid;
					b[cntb].pid=P;
					b[cntb].status=1;
					b[cntb].name=malloc(sizeof(char)*100);
					pointer=strtok(command2,"&");
					strcpy(b[cntb].name,pointer);
					cntb++;
				}
			}
		}
		else if(flag==2)		//for input redirection only
		{
			in=dup(STDIN_FILENO);
			int co=0;
			char *file[100];
			char *ne=strtok(command2,"<");	//tokenizing along <
			while(ne!=NULL)
			{
				file[co]=ne;
				file[co][strlen(ne)]='\0';
				co++;
				ne=strtok(NULL,"<");
			}
			file[co++]=ne;
			ne=strtok(file[0]," ");		//tokenizing along the spaces
			co=0;
			while(ne!=NULL)
			{
				final[co]=ne;
				final[co][strlen(ne)]='\0';
				co++;
				ne=strtok(NULL," ");
			}
			final[co++]=ne;
			if(file[1][0]==' ')
				ne=strtok(file[1]," ");
			strcpy(file[1],ne);
			int op=open(file[1],O_RDONLY);
			if(op<0)
			{
				printf("This file does not exist\n");	//Checking if the input file exists or not
				continue;
			}
			dup2(op,0);
			pid=fork();
			if(pid<0)
			{
				printf("Error in forking\n");
				exit(-1);
			}
			else if(pid==0)
			{
				if(execvp(final[0],final)<0)
				{
					printf("Cannot execute command\n");
					continue;
				}
			}
			else
				waitpid(pid,NULL,0);
			dup2(in,0);
			close(in);
		}
		else if(flag==3)		//for output redirection only
		{
			out=dup(STDOUT_FILENO);
			int co=0;
			char *file[100];
			char *ne=strtok(command2,">");		//tokenizing along >
			while(ne!=NULL)
			{
				file[co]=ne;
				file[co][strlen(ne)]='\0';
				co++;
				ne=strtok(NULL,">");
			}
			file[co++]=ne;
			ne=strtok(file[0]," ");			//tokenizing along the spaces
			co=0;
			while(ne!=NULL)
			{
				final[co]=ne;
				final[co][strlen(ne)]='\0';
				co++;
				ne=strtok(NULL," ");
			}
			final[co++]=ne;
			ne=strtok(file[1]," ");
			strcpy(file[1],ne);
			//int op=creat(file[1],O_WRONLY | S_IRWXU);
			if(access(file[1],F_OK)==0)
			{
				printf("This file already exists\n");	//checking if the output file already exists or not
				continue;
			}
			int op=creat(file[1],O_WRONLY | S_IRWXU);
			dup2(op,STDOUT_FILENO);
			pid=fork();
			if(pid<0)
			{
				printf("Error in forking\n");
				exit(-1);
			}
			else if(pid==0)
			{
				if(execvp(final[0],final)<0)
				{
					printf("Cannot execute command\n");
					continue;
				}
			}
			else
				waitpid(pid,NULL,0);
			dup2(out,1);
			close(out);
		}
		else if(flag==25)		//for both input and output redirection
		{
			in=dup(STDIN_FILENO);
			out=dup(STDOUT_FILENO);
			char *ne=strtok(command2,"<");		//tokenizing along <
			int co=0;
			char *mid1[100];
			while(ne!=NULL)
			{
				mid1[co]=ne;
				mid1[co][strlen(ne)]='\0';
				co++;
				ne=strtok(NULL,"<");
			}
			mid1[co++]=ne;
			char *mid2[100];
			char infile[100];
			char outfile[100];
			co=0;
			ne=strtok(mid1[1],">");			//tokenizing along >
			while(ne!=NULL)
			{
				mid2[co]=ne;
				mid2[co][strlen(ne)]='\0';
				co++;
				ne=strtok(NULL,">");
			}
			mid2[co++]=ne;
			ne=strtok(mid2[1]," ");		
			strcpy(outfile,ne);
			ne=strtok(mid1[1]," ");
			strcpy(infile,ne);
			co=0;
			ne=strtok(mid1[0]," ");			//tokenizing along the spaces
			while(ne!=NULL)
			{
				final[co]=ne;
				final[co][strlen(ne)]='\0';
				co++;
				ne=strtok(NULL," ");
			}
			final[co++]=ne;
			int op1=open(infile,O_RDONLY);
			//int op2=creat(outfile,O_WRONLY | S_IRWXU);
			if(op1<0)
			{
				printf("The input file does not exist.\n");	//checking if the input file already exists or not
				continue;
			}
			if(access(outfile,F_OK)==0)
			{
				printf("The output file already exists\n");	//checking if the output file already exists or not
				continue;
			}
			int op2=creat(outfile,O_WRONLY | S_IRWXU);
			pid=fork();
			dup2(op1,STDIN_FILENO);
			dup2(op2,STDOUT_FILENO);
			if(pid<0)
			{
				printf("Error in forking\n");
				exit(-1);
			}
			else if(pid==0)
			{
				if(execvp(final[0],final)<0)
				{
					printf("Error in executing command\n");
					continue;
				}
			}
			else
				waitpid(pid,NULL,0);
			dup2(in,0);
			dup2(out,1);
			close(in);
			close(out);
		}
		else if(flag==4 || flag==41 || flag==42 || flag==43)		//for piping
		{
			int flag1=0;
			in=dup(STDIN_FILENO);
			out=dup(STDOUT_FILENO);
			int co=0;
			char *mid[100];
			char *ne=strtok(command2,"|");			//tokenizing along the pipe
			while(ne!=NULL)
			{
				mid[co]=ne;
				mid[co][strlen(ne)]='\0';
				co++;
				ne=strtok(NULL,"|");
			}
			mid[co++]=ne;
			//printf("Tokenized along the pipe\n");
			//mid[co++]=ne;
			int in,out;
			char *com[100];
			pid_t pid;
			int coun;
			int index;
			int fi=-1,fo;
			//printf("Entering loop\n");
			for(index=0;index<co-1;index++)			//running a loop for each command
			{
				//printf("flag=%d\n",flag);
				int fd[2];
				if(index<co-2)				//if it is not the last command,a pipe is created
				{
					pipe(fd);
					fo=fd[1];			//making fo the write end
				}
				else
					fo=-1;				//no need to write further since it is the last command
				if(strstr(mid[index],"<"))
				{
					ne=strtok(mid[index],"<");	//tokenizing along <
					coun=0;
					while(ne!=NULL)
					{
			//			printf("%s\n",ne);
						com[coun]=ne;
						com[coun][strlen(ne)]='\0';
						coun++;
						ne=strtok(NULL,"<");
					}
					com[coun++]=ne;
					int cou=0;
					ne=strtok(com[0]," ");		//tokenizing along the spaces
					while(ne!=NULL)
					{
			//			printf("%s\n",ne);
						final[cou]=ne;
						final[cou][strlen(ne)]='\0';
						cou++;
						ne=strtok(NULL," ");
					}
					final[cou++]=ne;
					in=dup(STDIN_FILENO);		//duplicating the default input settings
					pid=fork();
					if(pid<0)
					{
						printf("Error in forking\n");
						exit(-1);
					}
					else if(pid==0)
					{
						dup2(fd[1],1);		//duplicating the file descriptor of the write end
						int op=open(strtok(com[1]," "),O_RDONLY);
						if(op<0)
						{
							printf("The file does not exist\n");	//checking if the input file exists or not
							exit(1);
						}
						dup2(op,0);		//duplicating the file descriptor of the file opened
						if(execvp(final[0],final)<0)
						{
							printf("Cannot execute command\n");
							exit(1);
							break;
						}
						dup2(in,0);	//duplicating the default mode back
						close(in);
						close(op);
					}
					else
						waitpid(pid,NULL,0);
				}
				else if(strstr(mid[index],">"))
				{
					ne=strtok(mid[index],">");		//tokenizing along >
					coun=0;
					while(ne!=NULL)
					{
			//			printf("%s\n",ne);
						com[coun]=ne;
						com[coun][strlen(ne)]='\0';
						coun++;
						ne=strtok(NULL,">");
					}
					com[coun++]=ne;
					int cou=0;
					ne=strtok(com[0]," ");			//tokenizing along the spaces
					while(ne!=NULL)
					{
			//			printf("%s\n",ne);
						final[cou]=ne;
						final[cou][strlen(ne)]='\0';
						cou++;
						ne=strtok(NULL," ");
					}
					final[cou++]=ne;
					pid=fork();
					out=dup(STDOUT_FILENO);		//duplicating the default settings in out
					if(pid<0)
					{
						printf("Error in forking\n");
						exit(-1);
					}
					else if(pid==0)
					{
						dup2(fd[0],0);		//duplicating the read port
						if(access(strtok(com[1]," "),F_OK)==0)
						{
							printf("This file already exists\n");	//checking if the file exists or not
							exit(1);
							break;
						}
						int op=creat(strtok(com[1]," "),O_WRONLY | S_IRWXU);
						dup2(op,1);
						if(execvp(final[0],final)<0)
						{
							printf("Cannot execute command\n");
							exit(1);
							break;
						}
						dup2(out,1);
						close(out);
					}
					else
						waitpid(pid,NULL,0);
				}
				else			//in case of no redirection
				{
			//		printf("Entering piping\n");
					ne=strtok(mid[index]," ");		//tokenizing along the spaces
					coun=0;
					while(ne!=NULL)
					{
			//			printf("%s\n",ne);
						final[coun]=ne;
						final[coun][strlen(ne)]='\0';
						coun++;
						ne=strtok(NULL," ");
					}
					final[coun++]=ne;
			//		printf("Tokenized along spaces for only piping\n");
					pid=fork();
					if(pid==0)
					{
						if(fo!=-1 && fi!=1)	//fo is -1 for the last command.
						{
							dup2(fo,1);	//duplicating the output file descriptor
							close(fo);
						}
						if(fi!=-1 && fi!=0)	//fi is -1 for the first command
						{
							dup2(fi,0);	//duplicating the input file descriptor
							close(fi);
						}
						if(execvp(final[0],final)<0)
						{
							printf("Cannot execute.\n");
							exit(1);
						}
					}
					else
						waitpid(pid,NULL,0);
				}
				close(fo);
				close(fi);
				fi=fd[0];	//making fi the read end of the pipe
				if(flag1==1)
					break;
			}
		}
		sleep(1);
	}
	return 0;
}

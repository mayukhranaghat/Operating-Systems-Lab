#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<string.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<sys/types.h>


#define clear() printf("\033[H\033[J")


int mycd(char **);
int myexit(char **);


char *mycmd[] = {
  "cd",
  "exit"
};

int (*intcmd[]) (char **) = {
  &mycd,
  &myexit
};




int my_cmd_len(void);
void infloop(void);
int execute(char **, char **);
int extcmd(char **);
char *read_line(void);
int semicolontokenizer(char*, char**);
int pipetokenizer(char* , char**);
void spacetokenizer(char* , char**);
int tokenizer(char *,char **,char **);
int split_line(char *);
void printDir(void);
void setup(void);







int main(int argc, char **argv)
{

  setup(); 


  infloop();

  return 0;
}

/*FUNC DEF*/


void setup()
{
	clear();
	printf("---------------------------MYSHELL--------------------------\n");
	char* user = "Mayukh";
	printf("Owner  = %s : \n",user);
	//sleep(2);clear();
}


void printDir() 
{ 
	char cwd[1024]; 
	getcwd(cwd, sizeof(cwd)); 
	printf("%s", cwd); 
	//return cwd;
}

int my_cmd_len() {
  return sizeof(mycmd) / sizeof(char *);
}

int mycd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "mycd: expected argument: \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("mycd");
    }
  }
  return 1;
}


int myexit(char **args)
{
  return -1;
}

void infloop(void)
{
  char *line;
  int flag;

  do {
    printf("\nMyShell@");
    printDir();
    printf("$~");
    line = read_line();
    flag = split_line(line);
    free(line);
  } while (flag);
}

int execute(char **cmd1,char **cmd2)
{


    pid_t pid1,pid2;
	int pfd[2];
	
	pid1 = fork();
	

			if(pid1 == 0)
			{
				if(pipe(pfd) < 0)
				{
					printf("Error during pipe creation\n");
					exit(0);
				}
				pid2 = fork();
				if(pid2 == 0)
				{	
					close(pfd[0]);
					//pfd[1] = open("test.txt",O_RDWR); 
					dup2(pfd[1],1);
//					close(pfd[1]);
					char *args[] = {cmd1[0],NULL};
					execvp(args[0],args);
				}
				else
				{
					close(pfd[1]);
					wait(NULL);
					dup2(pfd[0],0);
					char *args[] = {cmd2[0],NULL};				        
					execvp(args[0], args);
				}
			}
			else{
				wait(NULL);
			}


	return 1;
}


int extcmd(char **args)
{
  pid_t pid;
  int flag;

  pid = fork();
  if (pid == 0) {
    if (execvp(args[0], args) == -1) {
      perror("\nexecvp failure!!\n");
    }
    exit(0);
  } else if (pid < 0) {
    perror("\nfork failure!!\n");
  } else {
    do {
      waitpid(pid, &flag, WUNTRACED);
    } while (!WIFEXITED(flag) && !WIFSIGNALED(flag));
  }

  return 1;
}

char *read_line(void)
{
  int pos = 0;
  char *buffer = malloc(sizeof(char) * 1024);
  char ch;

  if (!buffer) {
    fprintf(stderr, "\nAllocation error!!\n");
    exit(0);
  }

  while (1) {
    ch = getchar();

    if (ch == EOF) 
    {
      exit(EXIT_SUCCESS);
    } 
    else if (ch == '\n') 
    {
      buffer[pos] = '\0';
      return buffer;
    } 
    else 
    {
      buffer[pos] = ch;
    }
    pos++;
  }

  /*while((ch = getchar())!=EOF)
	{
		msg[i] = ch;
		i++;
	}*/
}


int semicolontokenizer(char* line, char** semicolontoken)
{
	int i;
	for(i=0; i<1024; i++){
		semicolontoken[i]=strsep(&line, ";");
		if(semicolontoken[i]==NULL)
			break;
	}	
	return i;
} 

int pipetokenizer(char* str, char** strpiped) 
{ 
	int i; 
	for (i = 0; i < 2; i++) { 
		strpiped[i] = strsep(&str, "|"); 
		if (strpiped[i] == NULL) 
			break; 
	} 
	//printf("\t%s\t%s\t",strpiped[0],strpiped[1]);
	if (strpiped[1] == NULL) 
		return 0; 
	else 
		return 1; 
} 

void spacetokenizer(char* str, char** parsed)
{ 
	int i; 
	for (i = 0; i < 1024; i++) { 
		parsed[i] = strsep(&str, " "); 
		if (parsed[i] == NULL) 
			break; 
		if (strlen(parsed[i]) == 0) 
			i--; 
	} 
} 

int tokenizer(char *semicolontoken,char **token_norm,char **token_piped) 
{ 
	char* strpiped[2]; 
	int piped = 0,flag = 0; 
	piped = pipetokenizer(semicolontoken, strpiped); 
	if(piped){ 
		spacetokenizer(strpiped[0], token_norm); 
		spacetokenizer(strpiped[1], token_piped); 
	} 
	else 	
		spacetokenizer(semicolontoken, token_norm); 
	

	int i;
	for (i = 0; i < my_cmd_len(); i++) 
	{
    	if (strcmp(token_norm[0], mycmd[i]) == 0) 
    	{
      		flag = (*intcmd[i])(token_norm);
      		if(flag == -1)
      			return -1;
      		return 0;
    	}
  	}
	return 1 + piped; 
}

int split_line(char *line)
{
	int execFlag;
	int i;
  	char* semicolontoken[1024];
	int token_len = semicolontokenizer(line, semicolontoken);
	for(i=0;i<token_len;i++){


		char* token_norm[1024]; 
		
		char* token_piped[1024]; 
		
		execFlag = 0; 
		
		execFlag = tokenizer(semicolontoken[i], token_norm, token_piped); 
		if (execFlag == 1) 
			extcmd(token_norm); 
		if (execFlag == 2) 
			execute(token_norm, token_piped);
	}
	if(execFlag == -1)
		return 0;
	return 1;
}
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/stat.h>   
#include <unistd.h>

#define SERV_PORT 8080
#define BUFSIZE 2048

struct {
 char *ext;
 char *filetype;
} extensions [] = {
 {"gif", "image/gif" },
 {"jpg", "image/jpeg"},
 {"jpeg","image/jpeg"},
 {"png", "image/png" },
 {"zip", "image/zip" },
 {"gz",  "image/gz"  },
 {"tar", "image/tar" },
 {"htm", "text/htm" },
 {"html","text/html" },
 {"exe","text/plain" },
 {0,0} };


void ReplaceEle(char *filename,int filesize,int time,char *f_name){
    FILE* html=fopen("index.html","r");
    char *buffer=calloc(4096,sizeof(char));
    fread(buffer,sizeof(char),4096,html);
    char *tr=buffer;
    while(strstr(tr,"</tr>")){;
    tr=strstr(tr,"</tr>");
    tr+=strlen("</tr>");}
    fclose(html);
    html=fopen("index.html","w+");
    char *buffer2=calloc(4096,sizeof(char));
    strcat(buffer2,"<tr>\n\t<td><a href=\"");
    strcat(buffer2,f_name);
    strcat(buffer2,"\" download=\"");
    strcat(buffer2,f_name);
    strcat(buffer2,"\">");
    strcat(buffer2,filename);
    strcat(buffer2,"</a>");
    strcat(buffer2,"</td>\n\t<td>");
    char s[10]={0};
    sprintf(s,"%d",filesize);
    strcat(buffer2,s);
    strcat(buffer2,"</td>\n\t<td>");
    char t[10]={0};
    sprintf(t,"%d",time);
    strcat(buffer2,t);
    strcat(buffer2,"</td>\n\t</tr>");
    strcat(buffer2,tr+1);
    char *temp=calloc(4096,sizeof(char));
    strncpy(temp,buffer,tr-buffer);
    strcat(temp,buffer2);
    fwrite(temp,sizeof(char), strlen(temp), html);
    fclose(html);
    free(buffer);
    free(temp);
    free(buffer2);
    free(f_name);
}

void handle_socket(int fd)
{
 int j, file_fd, buflen, len;
 long i, ret;
 char * fstr;
 static char buffer[BUFSIZE+1]={};
 
 int fn_len=0;
 char *filename=NULL;
 int len_len=0;
 char *len_str=NULL;
 FILE *test_txt=NULL;
 
     
long cnt=0;

 time_t start,end;
 int init_switch=1;
 char* tempb=buffer;
 char *f_name=calloc(1024,sizeof(char));
 start=time(NULL);
 do{
 ret=read(fd, tempb,BUFSIZE);
 if(!strncmp(buffer,"GET",3)|| !strncmp(buffer,"get",3))break;
 if(init_switch){
 init_switch=0;
 char *ct=buffer;
 char *fn=buffer;
 char *len=buffer;
 int sw=0;
 while(strstr(len,"Content-Length: ")!=NULL){
 len=strstr(len,"Content-Length: ");
 len+=strlen("Content-Length: ");
 }
 while(strstr(ct,"Content-Type: ")!=NULL){
 ct=strstr(ct,"Content-Type: ");
 ct+=strlen("Content-Type: ");
 sw=1;
 }
 while(strstr(fn,"filename=\"")!=NULL){
 fn=strstr(fn,"filename=\"");
 fn+=strlen("filename=\"");
 }
 fn_len=strstr(fn,"\"")-fn;
 filename=calloc(fn_len+1,sizeof(char));
 strncpy(filename,fn,fn_len);
 len_len=strstr(len,"\n")-len;
 len_str=calloc(len_len+1,sizeof(char));
 strncpy(len_str,len,len_len);
 len_len=atoi(len_str);

 if(sw==1){
 ct=strstr(ct,"\n");
 ct+=3;
 }
 strncpy(f_name,len_str,strlen(len_str));
 strcat(f_name,strstr(filename,"."));
 test_txt=fopen(f_name, "ab+");
 fwrite(ct, sizeof(char), BUFSIZE-(ct-buffer), test_txt);
 cnt=BUFSIZE-(ct-buffer);
 fclose(test_txt);
 tempb=calloc(BUFSIZE+1,sizeof(char));
 continue;
 }
 
 test_txt=fopen(f_name, "ab+");
 fwrite(tempb, sizeof(char), BUFSIZE, test_txt);
 struct stat st;
 stat(f_name, &st);
 int size = st.st_size;
 if(st.st_size>=len_len+2048){ fclose(test_txt);free(tempb);break;}
 fclose(test_txt);
 free(tempb);
 tempb=calloc(BUFSIZE+1,sizeof(char));
 cnt+=BUFSIZE;
 }while(cnt<len_len);
 end=time(NULL);
 
 if(filename!=NULL){
 ReplaceEle(filename,len_len,end-start,f_name);
 free(filename);
 free(len_str);}
 printf("%s\n",buffer);

 if(ret==0||ret==-1) {
  exit(3);
 }
//尾巴補\0
 if(ret>0&&ret<BUFSIZE){
  buffer[ret] = 0;
  printf("%s\n", buffer);}
 else
  buffer[0] = 0;
//刪\r\n
 for(i=0;i<ret;i++) 
  if(buffer[i]=='\r'||buffer[i]=='\n')
   buffer[i] = 0;

 if((strncmp(buffer,"GET ",4)&&strncmp(buffer,"get ",4))&&(strncmp(buffer,"POST ",5)&&strncmp(buffer,"post ",5)))
  exit(3);
//分隔HTTP/1.0
 if(strncmp(buffer,"GET ",4)==0 || strncmp(buffer,"get ",4)==0){
 for(i=4;i<BUFSIZE;i++) {
  if(buffer[i] == ' ') {
   buffer[i] = 0;
   break;
  }
 }}
 else if(strncmp(buffer,"POST ",5)==0 || strncmp(buffer,"post ",5)==0){
 
 for(i=5;i<BUFSIZE;i++) {
  if(buffer[i] == ' ') {
   buffer[i] = 0;
   break;
  }
 }}
 

 if(!strncmp(&buffer[0],"GET /\0",6)||!strncmp(&buffer[0],"get /\0",6)||!strncmp(&buffer[0],"post",4)||!strncmp(&buffer[0],"POST",4))//家post
  strcpy(buffer,"GET /index.html\0");

 buflen = strlen(buffer);
 fstr = (char *)0;

 for(i=0;extensions[i].ext!=0;i++) {
  len = strlen(extensions[i].ext);
  if(!strncmp(&buffer[buflen-len], extensions[i].ext, len)) {
   fstr = extensions[i].filetype;
   break;
  }
 }

 if(fstr == 0) {
  fstr = extensions[i-1].filetype;
 }

 if((file_fd=open(&buffer[5],O_RDONLY))==-1)
  write(fd, "Failed to open file", 19);

 sprintf(buffer,"HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n", fstr);
 write(fd,buffer,strlen(buffer));

 while((ret=read(file_fd, buffer, BUFSIZE))>0) {
  write(fd,buffer,ret);
 }

 exit(1);
}

void sigchld(int signo)
{
 pid_t pid;
 
 while((pid=waitpid(-1, NULL, WNOHANG))>0);
}

int main(int argc, char **argv)
{
 int listenfd, socketfd;
 pid_t pid;
 socklen_t length;
 struct sockaddr_in cli_addr, serv_addr;
 

//開啟網路socket
 listenfd = socket(AF_INET, SOCK_STREAM, 0);
//初始化，將struct涵蓋的bits設為0
 bzero(&serv_addr, sizeof(serv_addr));
 serv_addr.sin_family = AF_INET;//sockaddr_in設為Ipv4
 serv_addr.sin_port = htons(SERV_PORT); //port80
 serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //使用任何在本機的對外IP

//將socket和IP、端口绑定…創建監聽器
 bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
//開始監聽
 listen(listenfd, 64);
//避免生殭屍的訊號處理
 signal(SIGCHLD, sigchld);

 while(1) {
  length = sizeof(cli_addr);
//等待客戶端連線
  if((socketfd = accept(listenfd, (struct sockaddr *)&cli_addr, &length))<0)
   exit(3);

//fork()子行程
  if((pid = fork()) < 0) {
   exit(3);
  }else {
   if(pid == 0) { //子
    close(listenfd);
    handle_socket(socketfd);
    exit(0);
   }else { //父
    close(socketfd);
   }
  }
 }
 return 0;
}

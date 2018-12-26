#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <zlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
//202.115.22.200:8111
const unsigned short port=8111;
const char ip[]="202.115.22.200",username[]="Xyct";
int cur=1,masz,iscp;
unsigned char sd[203333],rcv[2033456];
void wtVar(unsigned int a){
    do{
        sd[cur]=a&0x7f;
        a>>=7;
        if(a){
            sd[cur]|=0x80;
        }
        cur++;
    }while(a);
}
int rdVar(){
    int ret=0,now=0;
    do{
        ret|=(rcv[cur]&0x7f)<<now;
        now+=7;
        cur++;
    }while(rcv[cur-1]&0x80);
    return ret;
}
unsigned long long rd(int c){
    unsigned long long ret=0;
    for(int i=0;i<c;i++){
        ret=(ret<<8)|rcv[cur+i];
    }
    cur+=c;
    return ret;
}
void rdF(char *ch,int c){
    for(int i=1;i<=c;i++){
        ch[c-i]=rcv[cur++];
    }
}
void printString(){
    int l=rdVar()+cur;
    while(cur<l)putchar(rcv[cur++]);
}
void wt(int a,int c){
    for(int i=c-1;i>=0;i--){
        sd[cur++]=a%256;
        a>>=8;
    }
}
struct sockaddr_in server;
int client;
void reciv(){
    int ret=0,now=0;
    cur=0;
    do{
        recv(client,rcv+cur,1,0);
        ret|=(rcv[cur]&0x7f)<<now;
        now+=7;
        cur++;
    }while(rcv[cur-1]&0x80);
    while(ret){
        now=recv(client,rcv+cur,ret,0);
        ret-=now;
        cur+=now;
    }
}
void sen(int length){
    send(client,sd,length,0);
   printf("%d bytes data sent\n",length);
}
void handshake(char status){
   cur=1;
   /*Packet ID*/
   sd[cur++]=0;
   /*Data*/
   //Protocol version
   wtVar(404);
   //String length
   wtVar(strlen(ip));
   memcpy(sd+cur,ip,strlen(ip));
   cur+=strlen(ip);
   //Port
   wt(port,2);
   //Next state
   wtVar(status);
   //Total length
   sd[0]=cur-1;
   sen(cur);
}
void ping(){
    puts("====Pinging====");
    handshake(1);//1 for Ping
   sd[0]=1;
   sen(2);
   reciv();    
    printf("\n%s\n",rcv+5);
   cur=1;
   sd[cur++]=1;
   wt(time(0),8);
   sd[0]=cur-1;
   sen(cur);
    reciv();
    puts("====Ping finished====");
}
void uncomp(){
    //return;
    int ret=cur;
        cur=0;
        uLong flen=rdVar();//Packet length
        int p=cur;
        uLong ulen=rdVar();//Uncompressed length
        flen-=cur-p;
        //Packet ID
        unsigned char tmp[2033456];
        uncompress(tmp,&ulen,rcv+cur,flen);
        memcpy(rcv+cur,tmp,ulen);
        cur=ret;
}
void login(){
    handshake(2);
    cur=1;
    sd[cur++]=0;//login start
    wtVar(strlen(username));
    memcpy(sd+cur,username,strlen(username));
    cur+=strlen(username);
    sd[0]=cur-1;
    sen(cur);
    while(1){
        reciv();
        cur=0;
        rdVar();//Packet length
        if(iscp&&rdVar())uncomp();
       switch(rdVar()){//Packet ID
           case 3://Set compression
               printf("Set compression: Maximum no-compression packet size=%d\n",masz=rdVar());
               iscp=1;
           break;
           case 2://Login success
                puts("Login success.");
                int l=rdVar()+cur;
                printf("UUID: ");
                while(cur<l){
                    putchar(rcv[cur++]);
                }putchar('\n');
                l=rdVar()+cur;
                printf("Username: ");
                while(cur<l){
                    putchar(rcv[cur++]);
                }putchar('\n');
            return;
           default:
            for(int i=0;i<=200;i++){
               printf("%hhx ",rcv[i]);
            }
            puts("\n");
        }    
    }
}
char ct;
void cht(){
    char ch[255]="/log";
    printf("Send message?(Y/n)");
    fgets(ch,233,stdin);
    if(ch[0]=='n'||ch[0]=='N')return;
    printf("\n>>>");
    fgets(ch,233,stdin);
    printf("%s\n",ch);
    cur=1;
    sd[cur++]=0;
    sd[cur++]=2;
    wtVar(strlen(ch)-1);
    memcpy(sd+cur,ch,strlen(ch)-1);cur+=strlen(ch)-1;
    sd[0]=cur-1;
    puts("Chat message sending.");
    sen(cur);
}
int hndl(){//Main logic
    int l,id;
    double X,Y,Z;
            switch(id=rdVar()){//Packet ID
                case 0://Spawn Object
                    printf("Spawn object EID: %d\t",rdVar());
                    //printf("UUID: \n");
                    cur+=16;//unsigned 128-bit integer
                    printf("Type: %hhx\n",rcv[cur++]);
                    rdF((char*)&X,8);rdF((char*)&Y,8);rdF((char*)&Z,8);
                    printf("Position: (%lf,%lf,%lf)\n",X,Y,Z);
                    printf("Pitch: %d/256\tYaw: %d/256\t",rcv[cur],rcv[cur+1]);
                    cur+=2;
                    printf("Data: %d\t",(int)rd(4));
                    printf("Velocity: (%d,%d,%d)\n\n",(short)rd(2),(short)rd(2),(short)rd(2));
                break;
                case 0x0d://Server difficulty
                    printf("Difficulty: %s\n",rcv[cur]&1?rcv[cur]&2?"hard":"easy":rcv[cur]&2?"normal":"peaceful");
                break;
                case 0x0e://Chat message
                    printString();
                    puts("");
                break;
                case 0x11://Declare commands
                    printf("%d commands declared\n",rdVar());
                break;
                case 0x19://Plugin message
                    puts("Plugin message");
                break;
                case 0x1C://Entity status
                    printf("Entity ID: %d\nStatus: ",(int)rd(4));
                    printf("%hhx\n",rcv[cur]);
                break;
                case 0x21://Keep alive
                    puts("\tKeep alive");
                    int org=cur;
                    cur=1;
                    sd[cur++]=0;
                    sd[cur++]=0x0E;
                    memcpy(sd+cur,rcv+org,8);cur+=8;
                    sd[0]=cur-1;
                    sen(cur);
                    if(!(ct))
                    cht();
                    ct++;
                break;
                case 0x22://Chunk data
                    l=rd(4);
                    printf("Chunk Coords: (%d,%llu)\t",l,rd(4));
                    printf("%sfull chunk\t",rcv[cur++]?"Is ":"Not ");
                    printf("Mask: %x\n",rdVar());
                break;
                case 0x2e://Player abilities
                       //Send client settings
                    puts("========Sending client settings========");
                    cur=1;
                    sd[cur++]=0;
                    sd[cur++]=4;
                      char locale[]="zh_CN";
                    wtVar(strlen(locale));
                    memcpy(sd+cur,locale,strlen(locale));cur+=strlen(locale);
                    sd[cur++]=2;//Render distance
                    wtVar(0);
                    sd[cur++]=0;//Chat colors
                    sd[cur++]=0x7f;//Display bit mask
                    wtVar(1);//Main hand
                    sd[0]=cur-1;
                    sen(cur);
                break;
                case 0x25://Join game
                    printf("EID: %d\n",(int)rd(4));
                    cur++;//Game mode: survival
                    printf("Dimension: ");
                    switch((int)rd(4)){
                        case -1:puts("Nether");break;
                        case 0:puts("Overworld");break;
                        case 1:puts("End");break;
                        default:puts("Error");break;
                    }//unsigned byte difficulty
                    //String level type default
                break;
                case 0x32://Player position and look
                    puts("========Get player position and look========");
                    double X,Y,Z,Yaw,Pitch;
                    rdF((char*)&X,8);rdF((char*)&Y,8);rdF((char*)&Z,8);rdF((char*)&Yaw,4);rdF((char*)&Pitch,4);
                    printf("Player position: (%lf,%lf,%lf)\nPlayer look:\nYaw: %f\tPitch: %f\n",X,Y,Z,Yaw,Pitch);
                    printf("Flag: %hhx\nTeleport ID: ",rcv[cur++]);
                    printf("%d\n",rdVar());
                break;
                case 0x3d://Slot selection
                    printf("Slot number %d selected.\n",rcv[cur]);
                break;
                case 0x1b://Disconnect
                    puts("Disconnect for: ");
                    printString();
                    puts("");
                return 1;
                default:
                    //printf("ID: %hhx\n",id);
                break;
            }
    return 0;
}
void play(){
    if(SDL_Init(SDL_INIT_VIDEO)){puts("SDL Init error");return;}
    SDL_Window*win=SDL_CreateWindow("Mycraft",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,1000,700,SDL_WINDOW_SHOWN);
    if(!win){printf("Create window error %s\n",SDL_GetError());return;}
    SDL_Renderer*ren=SDL_CreateRenderer(win,-1,SDL_RENDERER_ACCELERATED);
    if(!ren){printf("Renderer error %s\n",SDL_GetError());return;}
    while(1){
        reciv();
        cur=0;
        rdVar();//Packet length
        if(rdVar()){//Uncompressed length
            uncomp();
        }
        if(hndl())break;
    }
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
}

int main(int argc,char **argv){
   if((client=socket(AF_INET,SOCK_STREAM,0))<0){
       puts("client socket create fail");
       return 1;
   }
   server.sin_family=AF_INET;
   server.sin_port=htons(port);
   server.sin_addr.s_addr=inet_addr(ip);
   if(connect(client,(struct sockaddr*)&server,sizeof(server))<0){
       puts("Connect error");
       return 1;
   }
   puts("Connecting...");
    if(argc>1&&argv[1][0]=='p')ping();
    else{
   login();
   puts("========Successfully logged in========");
    play();}
    close(client);
   return 0;
}

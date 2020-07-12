#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <zlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include "objects.h"
//202.115.22.200:8111
const unsigned short port=25565, ApiVersion=578;//1.15.2 20200711
const char ip[]="106.12.203.34",username[]="Xyct";
extern const double Pi;
int cur=1,masz,iscp,tick;
double playerX,playerY,playerZ,preX,preY,preZ;
float playerYaw,playerPitch;
unsigned char sd[203333],rcv[2033456];
char nttyp[123][123]={"AreaEffectCloud","ArmorStand","Arrow","Bat","Bee","Blaze","Boat","cat","CaveSpider","Chicken","Cod","Cow","Creeper","Donkey","Dolphin","DragonFireball","Drowned","ElderGuardian","EnderCrystal","EnderDragon","Enderman","Endermite","Evoker","Evoker Fangs","XPOrb","EyeOfEnderSignal","FallingBlock","FireworksRocket","Fox","Ghast","Giant","Guardian","Hoglin","Horse","Husk","Illusioner","Iron Golem","Item","ItemFrame","Fireball","LeashKnot","Lightning Bolt","Llama","LlamaSpit","LavaSlime (Magma Cube)","MinecartRideable","MinecartChest","MinecartCommandBlock","Minecart with Furnace","Minecart with Hopper","MinecartSpawner","MinecartTNT","Mule","Mushroom","Ocelot","Painting","Panda","Parrot","Phantom","Pig","Piglin","Piglin Brute","Pillager","Polar Bear","Primed TNT","Pufferfish","Rabbit","Ravager","Salmon","Sheep","Shulker","Shulker Bullet","Silverfish","Skeleton","Skeleton Horse","Slime"};
const int MAX_NTT_NUM=10000;
entity ntt[10004];
short chunkct;
struct chunk{
    int x,z;
    chunkSect section[16];
}chk[444];
struct block{
    int x,y,z;
}blk[123456];
int blkct;
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
void wtF(char *ch,int c){
    for(int i=1;i<=c;i++){
        sd[cur++]=ch[c-i];
    }
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
   wtVar(ApiVersion);
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
                    ct=1;
    char ch[255]="/log";
    /*printf("Send message?(Y/n)");
    fgets(ch,233,stdin);
    if(ch[0]=='n'||ch[0]=='N')return;*/
    printf("\n>>>");
    fgets(ch,233,stdin);
    printf("%s\n",ch);
    cur=1;
    sd[cur++]=0;
    sd[cur++]=3;
    wtVar(strlen(ch)-1);
    memcpy(sd+cur,ch,strlen(ch)-1);cur+=strlen(ch)-1;
    sd[0]=cur-1;
    puts("Chat message sending.");
    sen(cur);
}
int shaderProgram,vertID,fragID;
SDL_Window*win;
double cubes[23334],blocks[233345];
int cubect,blockct;
int dx[]={-1,0,0,1,0,0},dy[]={0,1,0,0,-1,0},dz[]={0,0,-1,0,0,1},flor=2333;
/*void dfs(int x,int y,int z){
    if(flor<0)return;
    int dataX=x-mapData[0][0][0].x,dataZ=z-mapData[0][0][0].z;
    while(dataX<0)dataX+=336;
    while(dataZ<0)dataZ+=336;
    dataX%=336;dataZ%=336;
    vis[dataX][y][dataZ]=1;
    for(int i=0;i<6;i++){
        int X=x+dx[i],Y=y+dy[i],Z=z+dz[i];
        if(abs(X+Y+Z-playerX-playerY-playerZ)<13){
            if(mapData[(dataX+dx[i]+336)%336][Y][(dataZ+dz[i]+336)%336].id){
                blocks[blockct++]=X;
                blocks[blockct++]=Y;
                blocks[blockct++]=Z;
                vis[(dataX+dx[i]+336)%336][Y][(dataZ+dz[i]+336)%336]=1;
            }else if(!vis[(dataX+dx[i]+336)%336][Y][(dataZ+dz[i]+336)%336]){
                flor--;dfs(X,Y,Z);flor++;
            }
        }
    }
}*/
void draw(){
    if(playerY<1||chunkct<9)return;
    if(0&&blkct<1){
        for(int i=0;i<9;++i){
            for(int j=0;j<16;++j){
                if(chk[i].section[j].num<1)continue;
                int last=0,bitct=0;
                for(int longct=0;longct<chk[i].section[j].num;++longct){
                    while(64-bitct>=chk[i].section[j].bits){
                        if(last);//with increasing x coordinates, within rows of increasing z coordinates, within layers of increasing y coordinates. 
                    }
                    last=chk[i].section[j].data[longct]%(1<<(64-bitct));
                }
            }
        }
    }
    float offset=1;
    if(tick<5000||tick>19000) offset=0.2;
    else if(tick<6000) offset=0.2+(tick-5000.0)/1000*0.8;
    else if(tick>18000) offset=1-(tick-18000.0)/1000*0.8;
    glClearColor(offset*0.6,offset*0.6,offset*1,1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    if(0){
    for(int i=0;i<cubect;i+=3){
        mats(shaderProgram,cubes[i],cubes[i+1],cubes[i+2],playerX,playerY,playerZ,playerYaw,playerPitch);
        glDrawArrays(GL_TRIANGLES,0,36);
    }
    for(int i=0;i<blockct;i+=3){
        mats(shaderProgram,blocks[i],blocks[i+1],blocks[i+2],playerX,playerY,playerZ,playerYaw,playerPitch);
        GLuint ID=glGetUniformLocation(shaderProgram,"blockColor");
        float cl[]={blocks[i],blocks[i+1],blocks[i+2],1};
        glUniform4fv(ID,1,cl);
        glDrawArrays(GL_TRIANGLES,0,36);
    }}
    SDL_GL_SwapWindow(win);
}
void move(){
    if(abs(playerX*playerY*playerZ)<0.1)return;
    int ret=cur;cur=1;sd[cur++]=0;sd[cur++]=0x11;
    wtF((char*)&playerX,8);wtF((char*)&playerY,8);wtF((char*)&playerZ,8);
    printf("New pos(%lf,%lf,%lf)\tYaw: %f\tPitch: %f\n",playerX,playerY,playerZ,playerYaw,playerPitch);
    sd[cur++]=1;sd[0]=cur-1;sen(cur);cur=ret;
}
entity* findNtt(int id){
    return ntt+(id%MAX_NTT_NUM);
}
int bitLeft=0;
unsigned long long theLong=0;
int rdNBTcpd(){//NBT compound
    int id=rcv[cur++];
    if(!id)return 1;
    int l=rd(2);//Length of name
    //printf("Name length %d\t",l);
    cur+=l;
    /*while(l>0){--l;
        putchar(rcv[cur++]);
    }*/
    switch(id){
        case 0xc://Long Array
            l=rd(4);//Number of longs
            //printf("Number of longs %d\n",l);
            cur+=l*8;//Skip the longs
        break;
        default:
        printf("Unrecognised NBT tag type %x\n",id);
    }
    return 0;
}
void rdNBT(){
    cur++;//Root Compound Start 0x0a
    int l=rd(2);//Length of title
    cur+=l;//Skip title
    while(!rdNBTcpd());
    return;
}
void rdChunk(){
    ++chunkct;
    printf("Chunk ");//Chunk data
    int x=rd(4),z;
    int mask,full;
    printf("Coords: (%d,%d)\t",x,z=rd(4));
    x*=16;z*=16;
    printf("%sfull chunk\t",(full=rcv[cur++])?"Is ":"Not ");
    printf("%x\t",mask=rdVar());
    rdNBT();
    int l=rdVar();
    printf("Data size: %d bytes\t",l);//Size of data in bytes
    //int debug=cur;
    int idx=0;
    for(int i=0;i<443;++i)if(chk[i].section[0].num==0){
        idx=i;break;
    }
    for(int i=0;i<16;i++){//Array of chunk section
        if(!((1<<i)&mask)) continue;
        rd(2);//Number of non-air blocks
        unsigned char bits=rcv[cur++];//Unsigned byte Bits per blocks
        char pallete=bits<9;
        if(pallete&&bits<4)bits=4;
        if(!pallete)bits=14;
        chk[idx].section[i].pallete=pallete;
        chk[idx].section[i].bits=bits;
        //printf("\n%d%d\n",pallete,bits);
        if(pallete){
             int size=rdVar();//Length
             for(int j=0;j<size;j++){//Array of Varint
                chk[idx].section[i].palle[j]=rdVar();
                //printf("%d ",palle[j]);
             }//puts("Reading pallete");
        }
        int l=chk[idx].section[i].num=rdVar();//Number of longs
        for(int j=0;j<l;++j){
             chk[idx].section[i].data[j]=rd(8);
        }
    }//Chunk sections end
    if(full)cur+=256*4;//256 integers biomes;
    //printf("\nExpected data: %d bytes, %d bytes in fact\tFull: %d\tcur: %d\n",l,cur-debug,full,cur);
    int blockEntity=rdVar();//Number of block entities in NBT tags;
    printf("%d block entities\n",blockEntity);
}
int hndl(){//Main logic
    int l,id;
    double X,Y,Z;
            switch(id=rdVar()){//Packet ID
                case 0://Spawn Object
                    printf("Spawn object EID: %d\t",rdVar());
                    //printf("UUID: \n");
                    cur+=16;//unsigned 128-bit integer
                    printf("Type: %hhx\n",rdVar());
                    rdF((char*)&X,8);rdF((char*)&Y,8);rdF((char*)&Z,8);
                    printf("Position: (%lf,%lf,%lf)\n",X,Y,Z);
                    printf("Pitch: %d/256\tYaw: %d/256\t",rcv[cur],rcv[cur+1]);
                    cur+=2;
                    //cubes[cubect++]=X;cubes[cubect++]=Y;cubes[cubect++]=Z;
                    printf("Data: %d\t",(int)rd(4));
                    printf("Velocity: (%d,%d,%d)\n\n",(short)rd(2),(short)rd(2),(short)rd(2));
                break;
                case 3://Spawn Mob
                    printf("Spawn Mob EID: %d\t",l=rdVar());
                    //printf("UUID: \n");
                    cur+=16;//unsigned 128-bit integer
                    int type=rdVar();
                    entity* mob=findNtt(l);
                    if(mob->id>0)puts("Error: current entity discovery algorithm should be optimized!");
                    mob->id=l;mob->type=type;
                    printf("Type: %d(%s)\n",type,nttyp[type]);
                    teleportEntity(mob);
                    printf("Head Pitch: %d/256",mob->headPitch=rcv[cur++]);
                    printf("Velocity: (%d,%d,%d)\n\n",mob->vx=rd(2),mob->vy=rd(2),mob->vz=rd(2));
                    //Entity metadata
                break;
                case 6:
                    printf("An entity changes animation\n");
                break;
                case 11:
                    //puts("Block action");
                break;
                case 12:
                    puts("Block changes");
                break;
                case 0x0e://Server difficulty
                    printf("Difficulty: %s\n",rcv[cur]&1?rcv[cur]&2?"hard":"easy":rcv[cur]&2?"normal":"peaceful");
                break;
                case 0x0f://Chat message
                    printString();
                    puts("");
                break;
                case 0x11://Declare commands
                    printf("%d commands declared\n",rdVar());
                break;
                case 0x15:
                    printf("My inventory changes\n");
                break;
                case 0x17:
                    printf("Slot set\n");
                break;
                case 0x19://Plugin message
                    puts("Plugin message");
                break;
                case 0x1a:
                    printf("A sound plays\n");
                break;
                case 0x1c://Entity status
                    printf("Entity ID: %d\nStatus: ",l=(int)rd(4));
                    printf("%hhx\t%s\n",rcv[cur],nttyp[ntt[l%20000].type]);
                break;
                case 0x1d:
                    puts("An explosion happens");
                break;
                case 0x1f:
                    puts("Game state changes");
                break;
                case 0x21://Keep alive
                    puts("\tKeep alive");
                    int org=cur;
                    cur=1;
                    sd[cur++]=0;
                    sd[cur++]=0x0f;
                    memcpy(sd+cur,rcv+org,8);cur+=8;
                    sd[0]=cur-1;
                    sen(cur);
                    if(!ct)
                    cht();
                break;
                case 0x22://chunk data
                    rdChunk();
                    //puts("Chunk OK");
                break;
                case 0x23:
                    puts("An effect(sound/particle) is played");
                break;
                case 0x25:
                    printf("Light levels of a chunk updated\t");
                break;
                case 0x26://Join game
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
                case 0x29://Entity moves
                {//Fuck C
                    l=rdVar();
                    entity*tt=findNtt(l);
                    moveEntity(tt);
                    tt->onGround=rd(1);}
                break;
                case 0x2a://entity moves and rotates
                {
                    l=rdVar();
                    entity*tt=findNtt(l);
                    moveEntity(tt);
                    rotateEntity(tt);
                    tt->onGround=rd(1);}
                break;
                case 0x2b://set entity angle
                {
                    l=rdVar();
                    entity*tt=findNtt(l);
                    rotateEntity(tt);
                    tt->onGround=rd(1);}
                break;
                case 0x2c:
                    puts("An entity initialized");
                break;
                case 0x32://Player abilities
                       //Send client settings
                    /*puts("========Sending client settings========");
                    cur=1;
                    sd[cur++]=0;
                    sd[cur++]=4;
                      char locale[]="zh_CN";
                    wtVar(strlen(locale));
                    memcpy(sd+cur,locale,strlen(locale));cur+=strlen(locale);
                    sd[cur++]=3;//Render distance
                    wtVar(0);
                    sd[cur++]=0;//Chat colors
                    sd[cur++]=0x7f;//Display bit mask
                    wtVar(1);//Main hand
                    sd[0]=cur-1;
                    sen(cur);*/
                break;
                case 0x36://Player position and look
                    puts("========Get player position and look========");
                    rdF((char*)&playerX,8);rdF((char*)&playerY,8);rdF((char*)&playerZ,8);
                    rdF((char*)&playerYaw,4);rdF((char*)&playerPitch,4);
                    printf("Player position: (%lf,%lf,%lf)\n\
Player look:\nYaw: %f\tPitch: %f\n",playerX,playerY,playerZ,playerYaw,playerPitch);
                    draw();
                    printf("Flag: %hhx\nTeleport ID: ",rcv[cur++]);
                    int tid;
                    printf("%d\n",tid=rdVar());
                    cur=1;sd[cur++]=0;sd[cur++]=0;//Packet ID
                    wtVar(tid);sd[0]=cur-1;sen(cur);
                    move();
                break;
                case 0x38:
                    puts("Destroy entities:");
                    l=rdVar();
                    for(int i=0;i<l;++i){
                        int id=rdVar();
                        printf("%d\t",id);
                        entity* tt=findNtt(id);
                        tt->id=-1;//Destroy this entity
                    }
                break;
                case 0x3c://Entity head horizontal
                    l=rdVar();
                    findNtt(l)->headYaw=rd(1);
                break;
                case 0x40://Slot selection
                    printf("Slot number %d selected.\n",rcv[cur]);
                break;
                case 0x44:
                    puts("Entity metadata received");
                break;
                case 0x46://Entity set velocity
                {
                    l=rdVar();
                    entity*tt=findNtt(l);
                    tt->vx=rd(2);tt->vy=rd(2);tt->vz=rd(2);}
                break;
                case 0x47:
                    puts("Entity equipment");
                break;
                case 0x49:
                    printf("Health: ");
                    float health,saturation;
                    int food;
                    rdF((char*)&health,4);
                    printf("%f/20\n",health);
                    food=rdVar();
                    printf("Food: %d/20\n",food);
                    rdF((char*)&saturation,4);
                    printf("saturation: %f/5\n",saturation);
                break;
                case 0x4f:
                    //puts("Time changes");
                    rd(8);//printf("World age:%lld\t",rd(8));
                    long long ticks=rd(8);tick=(ticks+6000)%24000;
                    if((tick/20)%5)break;
                    printf("Time of day:%lld\t\t",ticks);
                    printf("%d -th tick of today, i.e. %02d:%02d\n",tick,tick*18/5/3600,(tick*18/5%3600)/60);
                break;
                case 0x52:
                    puts("Plays hardcoded sound effect");
                break;
                case 0x57://Teleport entity
                    l=rdVar();
                    teleportEntity(findNtt(l));
                break;
                case 0x59:
                    puts("Entity properties changed");
                break;
                case 0x1b://Disconnect
                    puts("Disconnect for: ");
                    printString();
                    puts("");
                return 1;
                default:
                    printf("ID: %hhx\n",id);
                break;
            }
    return 0;
}
int shad(char *name,char*const ipt){
    FILE*f=fopen(name,"r");
    int I=-1;while(~(ipt[++I]=fgetc(f)));ipt[I]=0;
    fclose(f);
    return I;
}
void play(){
    if(SDL_Init(SDL_INIT_VIDEO)){puts("SDL Init error");return;}
    win=SDL_CreateWindow("Mycraft",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,1440,900,SDL_WINDOW_OPENGL);
    if(!win){printf("Create window error %s\n",SDL_GetError());return;}
    SDL_GLContext contx;
    if(NULL==(contx=SDL_GL_CreateContext(win))){printf("Creat context error %s\n",SDL_GetError());return;}
    if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE)|SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,2))puts("GL_SetAttribute error");
    GLfloat cube[144]={-0.48f,-0.48f,-0.48f, // triangle 1 : begin
    -0.48f, 0.48f, 0.48f,
    0.48f,-0.48f, 0.48f};
    glewExperimental = GL_TRUE;glewInit();
    GLuint vbo,vao;glGenBuffers(1,&vbo);glGenVertexArrays(1,&vao);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBindVertexArray(vao);
    glBufferData(GL_ARRAY_BUFFER,sizeof(cube),cube,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,0);
    glEnableVertexAttribArray(0);
    shaderProgram=glCreateProgram(),vertID=glCreateShader(GL_VERTEX_SHADER),fragID=glCreateShader(GL_FRAGMENT_SHADER);
    glAttachShader(shaderProgram,vertID);glAttachShader(shaderProgram,fragID);
    char*const ipt=malloc(23333);
    int I=shad("vert.glsl",ipt);
    glShaderSource(vertID,1,(const char*const*)&ipt,&I);
    glCompileShader(vertID);
    glGetShaderiv(vertID,GL_INFO_LOG_LENGTH,&I);
    glGetShaderInfoLog(vertID,I,NULL,ipt);//printf("Vert:%s",ipt);
    I=shad("frag.glsl",ipt);
    glShaderSource(fragID,1,(const char*const*)&ipt,&I);
    glCompileShader(fragID);
    glGetShaderiv(fragID,GL_INFO_LOG_LENGTH,&I);
    glGetShaderInfoLog(fragID,I,NULL,ipt);//printf("Frag:%s",ipt);
    free(ipt);
    glLinkProgram(shaderProgram);glUseProgram(shaderProgram);
    glEnable(GL_DEPTH_TEST);glDepthFunc(GL_LEQUAL);
    //mats(shaderProgram,0,0);
    float blue=1;
    glClearColor(0.6,0.6,blue,0.8);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    //glDrawArrays(GL_TRIANGLES,0,36);
    SDL_GL_SwapWindow(win);
    SDL_Event event;
    short mouseX=-1,mouseY=-1;
    long now=clock();
    while(1){
        if(SDL_PollEvent(&event)){
            if(event.type==SDL_QUIT)break;
            if(event.type==SDL_KEYDOWN){
                //printf("Key name:%s\n",SDL_GetKeyName(event.key.keysym.sym));
                if(event.key.keysym.sym==SDLK_ESCAPE)break;
                if(event.key.keysym.sym==SDLK_t){
                    ct=0;
                    blue*=-1;
                    //glClearColor(0.5,0.2,blue,0.8);
                    //glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
                    //SDL_GL_SwapWindow(win);
                }
                double dx=.1*cos(playerYaw*Pi/180),dz=.1*sin(playerYaw*Pi/180);
                if(event.key.keysym.sym==SDLK_w){
                    playerX+=dx;playerZ+=dz;
                    //draw();
                    move();
                }
                if(event.key.keysym.sym==SDLK_a){
                    playerX+=dz;playerZ-=dx;
                    //draw();
                    move();
                }
                if(event.key.keysym.sym==SDLK_s){
                    playerX-=dx;playerZ-=dz;
                    //draw();
                    move();
                }
                if(event.key.keysym.sym==SDLK_d){
                    playerX-=dz;playerZ+=dx;
                    //draw();
                    move();
                }
                if(event.key.keysym.sym==SDLK_SPACE){
                    playerY+=0.0625;
                    //draw();
                    move();
                }
                if(event.key.keysym.sym==SDLK_LSHIFT||event.key.keysym.sym==SDLK_RSHIFT){
                    playerY-=0.0625;
                    //draw();
                    move();
                }
            }
            if(event.type==SDL_MOUSEMOTION){
                double dx=event.motion.x-mouseX,dy=event.motion.y-mouseY;
                if(abs(dx)+abs(dy)<4)continue;
                if((~mouseX)&&abs(dx)+abs(dy)<100){
                //puts("mouse motion");
                    playerYaw+=(double)(dx)/-3;playerPitch+=(double)(dy)/3;
                    if(playerPitch*playerPitch>8000)playerPitch*=89.4/abs(playerPitch);
                    while(playerYaw<0)playerYaw+=360;
                    while(playerYaw>360)playerYaw-=360;
                    //draw();
                }
//                move();
                mouseX=event.motion.x;mouseY=event.motion.y;
            }
            continue;
        }
        if(clock()-now>333){draw();now=clock();}
        reciv();
        cur=0;
        rdVar();//Packet length
        if(rdVar()){//Uncompressed length
            uncomp();
        }
        if(hndl())break;
    }
    /*for(int i=0;i<80;i++){
        for(int j=0;j<80;j++){
            printf("%d ",mapData[i][(int)playerY][j].id);
        }puts("");
    }*/
    glDeleteShader(vertID);
    glDeleteShader(fragID);
    glDisableVertexAttribArray(0);
    glDeleteBuffers(1,&vbo);
    glDeleteVertexArrays(1,&vao);
    SDL_GL_DeleteContext(contx);
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
   puts("Connecting...");
   if(connect(client,(struct sockaddr*)&server,sizeof(server))<0){
       puts("Connect error");
       return 1;
   }
    if(argc>1&&argv[1][0]=='p')ping();
    else{
        login();
        puts("========Successfully logged in========");
        play();//Main function
    }
    close(client);
    puts("\nbye");
   return 0;
}

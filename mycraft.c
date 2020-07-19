#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <zlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "objects.h"
//202.115.22.200:8111
const unsigned short ApiVersion=578;//1.15.2 20200711
unsigned short port=25565;//May be changed by DNS SRV response
const char ip[]="mcyou.cc",username[]="Xyct";
//const char ip[]="106.12.203.34",username[]="Xyct";
//maximum no-compression packet size, is-compression
int cur=1,masz,iscp,tick;
//double playerX,playerY,playerZ,preX,preY,preZ;
float playerYaw,playerPitch;//More accurate angel for render
unsigned char sd[203333]="_minecraft._tcp.",rcv[2033456];
char nttyp[123][123]={"AreaEffectCloud","ArmorStand","Arrow","Bat","Bee","Blaze","Boat","cat","CaveSpider","Chicken","Cod","Cow","Creeper","Donkey","Dolphin","DragonFireball","Drowned","ElderGuardian","EnderCrystal","EnderDragon","Enderman","Endermite","Evoker","Evoker Fangs","XPOrb","EyeOfEnderSignal","FallingBlock","FireworksRocket","Fox","Ghast","Giant","Guardian","Hoglin","Horse","Husk","Illusioner","Iron Golem","Item","ItemFrame","Fireball","LeashKnot","Lightning Bolt","Llama","LlamaSpit","LavaSlime (Magma Cube)","MinecartRideable","MinecartChest","MinecartCommandBlock","Minecart with Furnace","Minecart with Hopper","MinecartSpawner","MinecartTNT","Mule","Mushroom","Ocelot","Painting","Panda","Parrot","Phantom","Pig","Piglin","Piglin Brute","Pillager","Polar Bear","Primed TNT","Pufferfish","Rabbit","Ravager","Salmon","Sheep","Shulker","Shulker Bullet","Silverfish","Skeleton","Skeleton Horse","Slime"};
const int MAX_NTT_NUM=10000;
entity ntt[10004],*player;
short chunkct;
chunk chk[50][50];
block blk[123456];
//int blkct;
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
    //reciv();//only used for speed testing, some servers don't response
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
                gameState=PLAY;
            return;
           case 0://Disconnect
                puts("Disconnect for: ");
                printString();
                puts("");
                gameState=EXIT;
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
void move(){
    if(abs(player->x*player->y*player->z)<0.1)return;
    int ret=cur;cur=1;sd[cur++]=0;sd[cur++]=0x11;
    wtF((char*)&player->x,8);wtF((char*)&player->y,8);wtF((char*)&player->z,8);
    printf("New pos(%lf,%lf,%lf)\tYaw: %f\tPitch: %f\n",player->x,player->y,player->z,playerYaw,playerPitch);
    sd[cur++]=1;sd[0]=cur-1;sen(cur);cur=ret;
}
entity* findNtt(int id){
    return ntt+(id%MAX_NTT_NUM);
}
int bitLeft=0;
unsigned long long theLong=0;
int rdNBTcpd(){//NBT compound
    int id=rcv[cur++];
    //printf("NBT compound type id:%d\t",id);
    if(!id)return 1;
    int l=rd(2);//Length of name
    /*printf("Name length %d\t",l);
    for(int i=0;i<l;++i){
        putchar(rcv[cur+i]);
    }*/
    cur+=l;
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
    for(int i=0;i<l;++i){putchar(rcv[cur+i]);}
    cur+=l;//Skip title
    while(!rdNBTcpd());
    return;
}
void rdChunk(){
    ++chunkct;
    printf("Chunk ");//Chunk data
    int x=rd(4),z;
    int full;
    printf("Coords: (%d,%d)\t",x,z=rd(4));
    chunk*this=getChunk(x,z);
    this->x=x;  this->z=z;
    x*=16;z*=16;
    printf("%sfull chunk\t",(full=rcv[cur++])?"Is ":"Not ");
    printf("%x\t",this->mask=rdVar());
    rdNBT();
    if(full){
        int l=1024;
        //printf("Biome array length:%d\t",l);
        cur+=l*4;//Array of integers biomes;
    }
    int l=rdVar();
    printf("Data size: %d bytes\t",l);//Size of data in bytes
    //int debug=cur;
    for(int i=0;i<16;i++){//Array of chunk section
        if(!((1<<i)&this->mask)) continue;
        rd(2);//Number of non-air blocks
        unsigned char bits=rcv[cur++];//Unsigned byte Bits per blocks
        char pallete=bits<9;
        //if(pallete&&bits<4)bits=4;
        if(!pallete)bits=14;
        this->section[i].pallete=pallete;
        this->section[i].bits=bits;
        //printf("\n%d%d\n",pallete,bits);
        if(pallete){
             int size=this->section[i].palleSize=rdVar();//Length
             //printf("bits: %d Pallete size:%d\n",bits,size);
             for(int j=0;j<size;j++){//Array of Varint
                this->section[i].palle[j]=rdVar();
             }//puts("Reading pallete");
        }
        int l=this->section[i].num=rdVar();//Number of longs
        if(l!=64*bits)printf("error: number of longs too little\t");
        //printf("\tnumber of longs:%d ",l);
        for(int j=0;j<l;++j){
             this->section[i].data[j]=rd(8);
        }
    }//Chunk sections end
    int blockEntity=rdVar();//Number of block entities in NBT tags;
    printf("%d block entities\n",blockEntity);
    this->loaded=1;
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
                    printf("EID: %d\n",l=rd(4));
                    player=findNtt(l);
                    for(int i=123;i>0;--i)if(nttyp[i]['0']=='P'){
                        player->type=i;break;
                    }//Hopefully be Player for several versions
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
                    rdF((char*)&player->x,8);rdF((char*)&player->y,8);rdF((char*)&player->z,8);
                    rdF((char*)&playerYaw,4);rdF((char*)&playerPitch,4);//store yaw and pitch accurately
                    printf("Player position: (%lf,%lf,%lf)\n\
Player look:\nYaw: %f\tPitch: %f\n",player->x,player->y,player->z,playerYaw,playerPitch);
                    //draw();
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
                case 0x3b://Respawn player
                    //TODO: delete all entities
                    unloadAllChunks();
                break;
                case 0x3c://Entity head horizontal
                    l=rdVar();
                    findNtt(l)->headYaw=rd(1);
                break;
                case 0x40://Slot selection
                    printf("Slot number %d selected.\n",rcv[cur]);
                break;
                case 0x43:
                    //puts("Display scoreboard by score name");
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
                case 0x4a:
                    //puts("create a scoreboard objective");
                break;
                case 0x4d:
                    //puts("update a scoreboard item");
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
                case 0x54:
                    //puts("Player list header and footer");
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
                    gameState=EXIT;
                return 1;
                default:
                    printf("ID: %hhx\n",id);
                break;
            }
    return 0;
}
int resolveSRV(const char* host, char* resolved) {//_minecraft._tcp.host-name
    //Anything went wrong and return origin value
    strcpy(resolved,host);
	struct __res_state res;
	if (res_ninit(&res) != 0)
		return -1;
	unsigned char answer[40000];
	int len = res_nsearch(&res, host, C_IN, T_SRV, answer, sizeof(answer));

	if (len < 0) {
		fprintf(stderr, "res_nsearch: %s\n", hstrerror(h_errno));
		return -1;
	}

	ns_msg handle;
	ns_rr rr;

	ns_initparse(answer, len, &handle);
//parse SRV response
	for (int i = 0; i < ns_msg_count(handle, ns_s_an); i++) {
		if (ns_parserr(&handle, ns_s_an, i, &rr) < 0 || ns_rr_type(rr) != T_SRV) {
			perror("ns_parserr");
			continue;
		}
		if (dn_expand(ns_msg_base(handle), ns_msg_end(handle), ns_rr_rdata(rr) + 3 * NS_INT16SZ, resolved,
		              32768) < 0)
			continue;
        port= ns_get16(ns_rr_rdata(rr) + 2 * NS_INT16SZ);
        //Read and store target hostname and port
		return 0;
	}
	return -1;
}
int networkThread(char*data){
   /*if((client=socket(AF_INET,SOCK_STREAM,0))<0){
       puts("client socket create fail");
       gameState=EXIT;
       return 1;
   }
    struct sockaddr_in server;
   server.sin_family=AF_INET;
   server.sin_port=htons(port);
   server.sin_addr.s_addr=inet_addr(ip);*/
   puts("Connecting...");

   struct addrinfo hints;
    struct addrinfo *result, *rp;
       memset(&hints, 0, sizeof(struct addrinfo));
       hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
        hints.ai_socktype = SOCK_STREAM; /* TCP socket */
       hints.ai_flags = 0;
       hints.ai_protocol = 0;          /* Any protocol */
       char*s=(char*)sd,*r=(char*)rcv;
       strcat(s,ip);
       resolveSRV(s,r);
       sprintf(s,"%d",port);
       int err = getaddrinfo(r, s, &hints, &result);
       //printf("%d\n",s);
       if (err != 0) {
           fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
           gameState=EXIT;
            exit(EXIT_FAILURE);
        }

           /* getaddrinfo() returns a list of address structures.
              Try each address until we successfully connect(2).
              If socket(2) (or connect(2)) fails, we (close the socket
              and) try the next address. */
           for (rp = result; rp != NULL; rp = rp->ai_next) {
               client = socket(rp->ai_family, rp->ai_socktype,
                            rp->ai_protocol);
               if (client == -1)
                   continue;
               if (connect(client, rp->ai_addr, rp->ai_addrlen) != -1)
                   break;                  /* Success */
               close(client);
           }

       if (rp == NULL) {               /* No address succeeded */
            fprintf(stderr, "Could not connect\n");
           gameState=EXIT;
           exit(EXIT_FAILURE);
       }

           freeaddrinfo(result);           /* No longer needed */


    if(data!=NULL&&data[0]=='p'){
        gameState=PING;
        ping();
        gameState=EXIT;
    }else{
        gameState=LOGIN;
        login();
        if(gameState==PLAY)puts("========Successfully logged in========");
        while(gameState!=EXIT){
            reciv();
            cur=0;
            rdVar();//Packet length
            if(rdVar()){//Uncompressed length
                uncomp();
            }
            if(hndl())break;
        }
    }
    close(client);
    client=-1;
    return 0;
}


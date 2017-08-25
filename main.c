#include "zhwkre/bss.h"
#include "zhwkre/concurrent.h"
#include "zhwkre/list.h"
#include "zhwkre/network.h"
#include "zhwkre/serialization.h"
#include "zhwkre/unordered_map.h"
#include "zhwkre/utils.h"
#include "protocol/protocol.h"
#include "protocol/errnos.h"
#include "permissionctl/permissionctl.h"
#include "advanced_network/advanced_network.h"
#include "configure/stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int ui;
qListDescriptor *data=NULL,*user=NULL,*group=NULL;
qMutex datalock,userlock,grouplock,connclock;
qMap lv1ids,lv2ids,lv3ids,userids,groupids;
ui uhashf(void* t,ui size){
    ui sum=0;
    for(ui i=0;i<size;i++){
        sum*=97;
        sum+=*(unsigned char*)(t+i);
    }
    return (sum%Q_DEFAULT_MAXHASHV);
}
ui RUNNING=1;
ui cliconns = 0;

#define mapinsert(map,key,value) qMap_insert(map,key,value,uhashf)
#define maperase(map,key) qMap_erase(map,key,uhashf)
#define mapseek(map,key) qMap_ptr_at(map,key,uhashf)
#define mapclear(map) qMap_clear(map)

#define NETWRCHECK(sock,bss) do{\
if(!qNetwork_writebss_auto((sock),(bss))){\
fprintf(stderr,"[WARN] Network error occurrred while writing to sockfd %d\n",(sock).desc);}}while(0);

#define LOCKDATA datalock.lock(datalock)
#define UNLOCKDATA datalock.unlock(datalock)
#define LOCKUSER userlock.lock(userlock)
#define UNLOCKUSER userlock.unlock(userlock)
#define LOCKGROUP grouplock.lock(grouplock)
#define UNLOCKGROUP grouplock.unlock(grouplock)

#define CHECKPRIV(testuid,flag) do{qList_foreach(*user,uiter){\
    UserData *tmprefr = uiter->data;\
    if(tmprefr->uid == (testuid)){\
        if(tmprefr->gid == 0){\
            flag = 1;\
        }\
        break;\
    }\
}}while(0)

binary_safe_string fread2bss(FILE* f);

void* handle_client(void* clisock_x);

int main(int argc,char** argv){
    if(argc < 2){
        printf("Usage: %s <listen_address:listen_port>\n",argv[0]);
        return 255;
    }

    // test data_storage_status
    { // keep stuff local.
        FILE* data1file,*data2file,*data3file;
        data1file = fopen("data1.dat","r");
        data2file = fopen("data2.dat","r");
        data3file = fopen("data3.dat","r");
        if(!(data1file && data2file && data3file)){
            fprintf(stderr,"[WARN] datafile not exist.fallback to creation.\n");
            data = malloc(sizeof(qListDescriptor));
            q__List_initdesc(data);
            if(data1file) fclose(data1file);
            if(data2file) fclose(data2file);
            if(data3file) fclose(data3file);
        }else{
            // initialize from file
            fprintf(stderr,"[INFO] read data from data1.dat.\n");
            binary_safe_string l1dt=fread2bss(data1file);
            fclose(data1file);
            fprintf(stderr,"[INFO] read data from data2.dat.\n");
            binary_safe_string l2dt=fread2bss(data2file);
            fclose(data2file);
            fprintf(stderr,"[INFO] read data from data3.dat.\n");
            binary_safe_string l3dt=fread2bss(data3file);
            fclose(data3file);
            fprintf(stderr,"[INFO] start unserialize storaged data.\n");
            qListDescriptor tmp_unser_ld;
            qList_initdesc(tmp_unser_ld);
            qList_push_back(tmp_unser_ld,l1dt);
            qList_push_back(tmp_unser_ld,l2dt);
            qList_push_back(tmp_unser_ld,l3dt);
            data = qUnserialize(tmp_unser_ld,YES_IT_IS_A_LIST);
            fprintf(stderr,"[INFO] unserialize progress success. clear-up.\n");
            qbss_destructor(l1dt);
            qbss_destructor(l2dt);
            qbss_destructor(l3dt);
            qList_destructor(tmp_unser_ld);
        }
    }
    // test group_storage_status
    {
        FILE* groupfile;
        groupfile = fopen("group.dat","r");
        if(!groupfile){
            fprintf(stderr,"[WARN] group.dat cannot be read. fallback to creation\n");
            group = malloc(sizeof(qListDescriptor));
            q__List_initdesc(group);
            fprintf(stderr,"[WARN] creating default administrator group.\n");
            GroupData tmpadmin;
            tmpadmin.gid = 0;
            // notice '\0' at the end of str
            memcpy(tmpadmin.groupname,DEFAULT_ADMIN_GROUPNAME,strlen(DEFAULT_ADMIN_GROUPNAME)+1);
            qList_push_back(*user,tmpadmin);
        }else{
            // unserialize from file
            fprintf(stderr,"[INFO] starting read data from group.dat\n");
            binary_safe_string groupbss = fread2bss(groupfile);
            fclose(groupfile);
            qListDescriptor tmpld;
            qList_initdesc(tmpld);
            qList_push_back(tmpld,groupbss);
            fprintf(stderr,"[INFO] starting unserialization process of grouplist.\n");
            group = qUnserialize(tmpld,YES_IT_IS_A_LIST);
            fprintf(stderr,"[INFO] unserialization success. starting clean-up.\n");
            qbss_destructor(groupbss);
            qList_destructor(tmpld);
        }
    }
    // test user_storage_status
    {
        FILE* userfile;
        userfile = fopen("user.dat","r");
        if(!userfile){
            fprintf(stderr,"[WARN] user.dat cannot be read. fallback to creation\n");
            user = malloc(sizeof(qListDescriptor));
            q__List_initdesc(user);
            fprintf(stderr,"[WARN] creating default administrator user.\n");
            UserData tmpadmin;
            tmpadmin.uid = 0;
            tmpadmin.gid = 0;
            // notice '\0' at the end of str
            memcpy(tmpadmin.username,DEFAULT_ADMIN_USERNAME,strlen(DEFAULT_ADMIN_USERNAME)+1);
            memcpy(tmpadmin.password,DEFAULT_ADMIN_PASSWORD,strlen(DEFAULT_ADMIN_PASSWORD)+1);
            qList_push_back(*user,tmpadmin);
        }else{
            // unserialize from file
            fprintf(stderr,"[INFO] starting read data from user.dat\n");
            binary_safe_string userbss = fread2bss(userfile);
            fclose(userfile);
            qListDescriptor tmpld;
            qList_initdesc(tmpld);
            qList_push_back(tmpld,userbss);
            fprintf(stderr,"[INFO] starting unserialization process of userlist.\n");
            user = qUnserialize(tmpld,YES_IT_IS_A_LIST);
            fprintf(stderr,"[INFO] unserialization success. starting clean-up.\n");
            qbss_destructor(userbss);
            qList_destructor(tmpld);
        }
    }
    // init id maps
    {
        char tmpyes = 1;
        lv1ids = qMap_constructor(Q_DEFAULT_MAXHASHV);
        lv2ids = qMap_constructor(Q_DEFAULT_MAXHASHV);
        lv3ids = qMap_constructor(Q_DEFAULT_MAXHASHV);
        qList_foreach(*data,iter){
            Level1Entry *le = iter->data;
            mapinsert(lv1ids,le->pe.entryid,tmpyes);
            qList_foreach((le->ld),iiter){
                Level2Entry *lle = iiter->data;
                mapinsert(lv2ids,lle->pe.entryid,tmpyes);
                qList_foreach((lle->ld),iiiter){
                    Level3Entry *llle = iiiter->data;
                    mapinsert(lv3ids,llle->pe.entryid,tmpyes);
                }
            }
        }
        groupids = qMap_constructor(Q_DEFAULT_MAXHASHV);
        qList_foreach(*group,iter){
            GroupData *g = iter->data;
            mapinsert(groupids,g->gid,tmpyes);
        }
        userids = qMap_constructor(Q_DEFAULT_MAXHASHV);
        qList_foreach(*user,iter){
            UserData *u = iter->data;
            mapinsert(userids,u->uid,tmpyes);
        }
    }
    // init mutexes
    {
        datalock = qMutex_constructor();
        userlock = qMutex_constructor();
        grouplock = qMutex_constructor();
        connclock = qMutex_constructor();
    }
    // start-up server
    fprintf(stderr,"[INFO] Trying to open up server...\n");
    {
        qSocket listener;
        listener.domain = qIPv4;
        listener.type = qStreamSocket;
        listener.protocol = qDefaultProto;
        if(!qSocket_open(listener)){
            fprintf(stderr,"[FAIL] cannot open socket.\n");
            return 254;
        }
        if(!qSocket_bind(listener,argv[1])){
            fprintf(stderr,"[FAIL] cannot bind socket at addr %s\n",argv[1]);
            return 254;
        }
        if(!qStreamSocket_listen(listener)){
            fprintf(stderr,"[FAIL] cannot listen socket.\n");
            return 254;
        }
        fprintf(stderr,"[INFO] Server listen at %s\n",argv[1]);
        while(RUNNING){
            char tmpdst[50];
            qSocket *tmpclient = malloc(sizeof(qSocket));
            *tmpclient = qStreamSocket_accept(listener,tmpdst);
            if(tmpclient->desc == -1){
                fprintf(stderr,"[WARN] network error occurred while accepting a client.\n");
                continue;
            }
            fprintf(stderr,"[INFO] client from %s connected. its fd is %d\n",tmpdst,tmpclient->desc);
            // start thread
            // acquire connlock
            connclock.lock(connclock);
            cliconns ++;
            connclock.unlock(connclock);
            qRun(handle_client,tmpclient);
        }
    }
    // wait until all conns process over
    fprintf(stderr,"[INFO] Server shutting down...\n");
    while(cliconns != 0){};
    // save data
    {
        fprintf(stderr,"[INFO] Saving data...\n");
        LOCKDATA;
        qListDescriptor ser_data = qSerialize(data,sizeof(qListDescriptor));
        // save to data
        FILE* ld[3];
        ld[0]=fopen("data1.dat","w");
        ld[1]=fopen("data2.dat","w");
        ld[2]=fopen("data3.dat","w");
        if(!(ld[0] && ld[1] && ld[2])){
            fprintf(stderr,"[FAIL] Error occurred while opening data file. Terminating..\n");
            return 253;
        }
        ui cntr = 0;
        qList_foreach(ser_data,iter){
            binary_safe_string *refr = iter->data;
            // write to
            ui succ = fwrite(refr->str,refr->size,1,ld[cntr]);
            if(!succ){
                fprintf(stderr,"[FAIL] Error occurred while writing to data%d.dat. Terminating..\n",cntr);
                return 252;
            }
            fclose(ld[cntr]);
            cntr++;
        }
        UNLOCKDATA;
        fprintf(stderr,"[INFO] Data saving completed.\n");
    }
    // save user
    {
        fprintf(stderr,"[INFO] Saving user..\n");
        LOCKUSER;
        qListDescriptor ser_data = qSerialize(user,sizeof(qListDescriptor));
        FILE* userfile = fopen("user.dat","w");
        if(!userfile){
            fprintf(stderr,"[FAIL] Error occurred while opening user data file. Terminating..\n");
            return 253;
        }
        binary_safe_string *refr = ser_data.head->data;
        ui succ = fwrite(refr->str,refr->size,1,userfile);
        if(!succ){
            fprintf(stderr,"[FAIL] Error ocurred while writing to user data file. Terminating..\n");
            return 252;
        }
        UNLOCKUSER;
        fprintf(stderr,"[INFO] User data saving completed.\n");
    }
    // save group
    {
        fprintf(stderr,"[INFO] Saving group..\n");
        LOCKGROUP;
        qListDescriptor ser_data = qSerialize(group,sizeof(qListDescriptor));
        FILE* groupfile = fopen("group.dat","w");
        if(!groupfile){
            fprintf(stderr,"[FAIL] Error occurred while opening group data file. Terminating..\n");
            return 253;
        }
        binary_safe_string *refr = ser_data.head->data;
        ui succ = fwrite(refr->str,refr->size,1,groupfile);
        if(!succ){
            fprintf(stderr,"[FAIL] Error ocurred while writing to group data file. Terminating..\n");
            return 252;
        }
        UNLOCKGROUP;
        fprintf(stderr,"[INFO] Group data saving completed.\n");
    }
    fprintf(stderr,"[INFO] Server shutdown process completed successfully.\n");
    return 0;
}

void* handle_client(void* clisock_x){
    qSocket* clisock = clisock_x;
    // read quest
    while(true){
        ui queryid=0;
        binary_safe_string rcontent = qNetwork_readbss(*clisock,&queryid);
        if(rcontent.size == 0){
            fprintf(stderr,"[INFO] Conn ended with client with fd %d\n",clisock->desc);
        }
        switch(queryid){
            case 0:
            {
                LoginQuery lq = qDisassembleLoginQuery(rcontent);
                ui FLAG_SUCC = 0;
                LOCKUSER;
                qList_foreach((*user),uiter){
                    UserData *tmprefr = uiter->data;
                    if(fullstrcmp(tmprefr->username,lq.username) && fullstrcmp(tmprefr->password,lq.password)){
                        // authentication success
                        FLAG_SUCC = true;
                        NETWRCHECK(*clisock,qAssembleLoginReply(0,tmprefr->uid,tmprefr->gid));
                        break;
                    }
                }
                if(!FLAG_SUCC && lq.isRegister){
                    // registration
                    UserData tmpud;
                    memcpy(tmpud.username,lq.username,lq.username_len+1);
                    memcpy(tmpud.password,lq.password,lq.password_len+1);
                    tmpud.gid = 1; // assemble a not-so-correct gid.
                    ui checkuid = 0;
                    for(checkuid=0;checkuid<100000000;checkuid++){
                        if(mapseek(userids,checkuid)==NULL){
                            FLAG_SUCC = 1;
                            break;
                        }
                    }
                    if(FLAG_SUCC){
                        tmpud.uid = checkuid;
                        qList_push_back(*user,tmpud);
                        char y=1;
                        mapinsert(userids,checkuid,y);
                        NETWRCHECK(*clisock,qAssembleLoginReply(0,tmpud.uid,tmpud.gid));
                    }
                }
                UNLOCKUSER;
                if(!FLAG_SUCC){
                    NETWRCHECK(*clisock,qAssembleLoginReply(LOGIN_INCORRECT,999,999));
                }
            }
            break;
            case 1:
            {
                AlterPassQuery apq=qDisassembleAlterPassQuery(rcontent);
                ui FLAG_SUCC = 0;
                LOCKUSER;
                qList_foreach(*user,uiter){
                    UserData *tmprefr = uiter->data;
                    if(tmprefr->uid == apq.userId){
                        FLAG_SUCC = true;
                        memcpy(tmprefr->password,apq.newpass,apq.passlen+1);// \0
                        break;
                    }
                }
                UNLOCKUSER;
                NETWRCHECK(*clisock,qAssembleAlterPassReply(FLAG_SUCC?0:USER_NOT_EXIST));
            }
            break;
            case 2:
            {
                ListGroupQuery lgq = qDisassembleListGroupQuery(rcontent);
                ui FLAG_SUCC = 0;
                LOCKUSER;
                qList_foreach(*user,uiter){
                    UserData *tmprefr = uiter->data;
                    if(tmprefr->uid == lgq.userId){
                        if(tmprefr->gid == 0){
                            // success
                            FLAG_SUCC = 1;
                            break;
                        }
                    }
                }
                UNLOCKUSER;
                if(FLAG_SUCC){
                    LOCKGROUP;
                    NETWRCHECK(*clisock,qAssembleListGroupReply(0,*group));
                    UNLOCKGROUP;
                }else{
                    LOCKGROUP;
                    NETWRCHECK(*clisock,qAssembleListGroupReply(PERMISSION_DENIED,*group));
                    UNLOCKGROUP;
                }
            }
            break;
            case 3:
            {
                AlterGroupQuery apq=qDisassembleAlterGroupQuery(rcontent);
                ui FLAG_SUCC = 0,FLAG_PRIV = 0;
                LOCKGROUP;
                qList_foreach(*group,giter){
                    GroupData *r=giter->data;
                    if(r->gid == apq.groupId){
                        FLAG_SUCC = 1;
                    }
                }
                UNLOCKGROUP;
                if(FLAG_SUCC){
                    FLAG_SUCC = 0;
                    LOCKUSER;
                    qList_foreach(*user,uiter){
                        UserData *tmprefr = uiter->data;
                        if(tmprefr->uid == apq.userId){
                            if(tmprefr->gid == 0){
                                FLAG_PRIV = 1;
                            }
                            break;
                        }
                    }
                    qList_foreach(*user,uiter){
                        UserData *tmprefr = uiter->data;
                        if(tmprefr->uid == apq.destuid){
                            if(apq.groupId == 0 && !FLAG_PRIV){
                                break;
                            }
                            FLAG_SUCC = 1;
                            tmprefr->gid = apq.groupId;
                            break;
                        }
                    }
                    UNLOCKUSER;
                    NETWRCHECK(*clisock,qAssembleAlterPassReply(FLAG_SUCC?0:PERMISSION_DENIED));
                }else{
                    NETWRCHECK(*clisock,qAssembleAlterGroupReply(GROUP_NOT_EXIST));
                }
            }
            break;
            case 4:
            {
                RemoveUserQuery q = qDisassembleRemoveUserQuery(rcontent);
                ui FLAG_SUCC = 0,FLAG_PRIV = 0;
                LOCKUSER;
                qList_foreach(*user,uiter){
                    UserData *tmprefr = uiter->data;
                    if(tmprefr->uid == q.userId){
                        if(tmprefr->gid == 0){
                            FLAG_PRIV = 1;
                        }
                        break;
                    }
                }
                qList_foreach(*user,uiter){
                    UserData *tmprefr = uiter->data;
                    if(tmprefr->uid == q.targetUserId){
                        if(FLAG_PRIV){
                            FLAG_SUCC = 1;
                            qList_erase_elem(*user,uiter);
                        }
                        break;
                    }
                }
                UNLOCKUSER;
                NETWRCHECK(*clisock,qAssembleRemoveUserReply(FLAG_SUCC?0:(FLAG_PRIV?USER_NOT_EXIST:PERMISSION_DENIED)));
            }
            break;
            case 5:
            {
                RemoveGroupQuery q = qDisassembleRemoveGroupQuery(rcontent);
                ui FLAG_SUCC = 0,FLAG_PRIV = 0;
                LOCKUSER;
                CHECKPRIV(q.userId,FLAG_PRIV);
                UNLOCKUSER;
                LOCKGROUP;
                qList_foreach(*group,giter){
                    GroupData *g=giter->data;
                    if(g->gid == q.targetGroupId){
                        if(FLAG_PRIV){
                            FLAG_SUCC = 1;
                            qList_erase_elem(*group,giter);
                        }
                    }
                }
                UNLOCKGROUP;
                NETWRCHECK(*clisock,qAssembleRemoveGroupReply(FLAG_SUCC?0:(FLAG_PRIV?GROUP_NOT_EXIST:PERMISSION_DENIED)));
            }
            break;
            case 6:
            {
                ListUserQuery q = qDisassembleListUserQuery(rcontent);
                ui FLAG_SUCC = 0;
                LOCKUSER;
                CHECKPRIV(q.uid,FLAG_SUCC);
                UNLOCKUSER;
                if(FLAG_SUCC){
                    LOCKUSER;
                    NETWRCHECK(*clisock,qAssembleListUserReply(0,*user));
                    UNLOCKUSER;
                }else{
                    LOCKUSER;
                    NETWRCHECK(*clisock,qAssembleListUserReply(PERMISSION_DENIED,*user));
                    UNLOCKUSER;
                }
            }
            break;
        }
    }
}

binary_safe_string fread2bss(FILE* f){
    binary_safe_string bss = qbss_new();
    char tmpbuffer[256];
    while(!feof(f)){
        int actreadc = fread(tmpbuffer,1,256,f);
        q__bss_append(&bss,tmpbuffer,actreadc);
    }
    return bss;
}
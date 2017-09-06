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

#define STRMASK "PROTECTED"
#define SHUTDOWN_PASSPHRASE "SAVE"

typedef unsigned int ui;
qListDescriptor *data=NULL,*user=NULL,*group=NULL;
qMutex datalock,userlock,grouplock,connclock;
qMap lv1ids,lv2ids,lv3ids,userids,groupids;
Level1 lv1mask;Level2 lv2mask;Level3 lv3mask;ui strmasklen;
#define MASKSTR(str) memcpy((str),STRMASK,strmasklen)
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
char** globargv = NULL;

qSocket listener;

ui permsize = 3*sizeof(unsigned char);

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

#define ALLOCID(idvar,idmap,succflag) do{\
    for((idvar)=0;(idvar)<100000000;(idvar)++){\
        if(mapseek((idmap),(idvar))==NULL){\
            (succflag) = 1;\
            char tmpyes=1;\
            mapinsert(idmap,idvar,tmpyes);\
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
    globargv = argv;
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
            memset(tmpadmin.groupname,0,256);
            // notice '\0' at the end of str
            memcpy(tmpadmin.groupname,DEFAULT_ADMIN_GROUPNAME,strlen(DEFAULT_ADMIN_GROUPNAME)+1);
            qList_push_back(*group,tmpadmin);
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
            memset(tmpadmin.username,0,256);
            memset(tmpadmin.password,0,256);
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
    // init masks
    {
        strmasklen = strlen(STRMASK);
        memset(&lv1mask,0,sizeof(lv1mask));
        MASKSTR(lv1mask.carId);MASKSTR(lv1mask.carName);
        memset(&lv2mask,0,sizeof(lv2mask));
        MASKSTR(lv2mask.carId);MASKSTR(lv2mask.carName);MASKSTR(lv2mask.customerId);
        MASKSTR(lv2mask.selldate);MASKSTR(lv2mask.customerName);MASKSTR(lv2mask.customerTel);
        memset(&lv3mask,0,sizeof(lv3mask));
        MASKSTR(lv3mask.carId);MASKSTR(lv3mask.paydate);MASKSTR(lv3mask.sellerId);
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
    ui FLAG_CONT = 1;
    while(FLAG_CONT){
        ui queryid=0;
        binary_safe_string rcontent = qNetwork_readbss(*clisock,&queryid);
        if(rcontent.size == 0){
            fprintf(stderr,"[INFO] Conn ended with client with fd %d\n",clisock->desc);
            qSocket_close(*clisock);
            FLAG_CONT = 0;
            continue;
        }
        fprintf(stderr,"[INFO] Client %d sent query id %u\n",clisock->desc,queryid);
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
                    ALLOCID(checkuid,userids,FLAG_SUCC);
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
                if(!FLAG_SUCC){
                    GroupData tmpgd;
                    tmpgd.gid = apq.groupId;
                    qList_push_back(*group,tmpgd);
                    FLAG_SUCC = 1;
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
            case 10:
            {
                SyncDataQuery q = qDisassembleSyncDataQuery(rcontent);
                // checkpriv
                ui FLAG_PRIV=0;
                LOCKUSER;
                CHECKPRIV(q.userId,FLAG_PRIV);
                UNLOCKUSER;
                LOCKDATA;
                qListDescriptor tmprld;
                qList_initdesc(tmprld);
                qList_foreach(*data,iter){
                    Level1Entry *le = iter->data;
                    Level1Entry tmple;
                    ui FLAG_I_RECURSIVE = 0,FLAG_I_SUCC = 0;
                    if(le->pe.ownerid == q.userId || FLAG_PRIV){
                        // full permission
                        qList_initdesc(tmple.ld);
                        memcpy(&(tmple.data),&(le->data),sizeof(Level1));
                        memcpy(&(tmple.pe),&(le->pe),sizeof(PermissionEntry));
                        FLAG_I_RECURSIVE = 1;FLAG_I_SUCC = 1;
                    }else{
                        if(le->pe.groupid == q.groupId){
                            if (le->pe.permission[1] & Q_PERMISSION_R){
                                qList_initdesc(tmple.ld);
                                memcpy(&(tmple.data),&(le->data),sizeof(Level1));
                                memcpy(&(tmple.pe),&(le->pe),sizeof(PermissionEntry));
                                FLAG_I_RECURSIVE = 1;FLAG_I_SUCC = 1;
                            }else if(le->pe.permission[1] & Q_PERMISSION_C){
                                qList_initdesc(tmple.ld);
                                memcpy(&(tmple.data),&(lv1mask),sizeof(Level1));
                                memcpy(&(tmple.pe),&(lv1mask),sizeof(PermissionEntry));
                                FLAG_I_SUCC = 1;
                            }
                        }else if(le->pe.permission[2] & Q_PERMISSION_R){
                            qList_initdesc(tmple.ld);
                            memcpy(&(tmple.data),&(le->data),sizeof(Level1));
                            memcpy(&(tmple.pe),&(le->pe),sizeof(PermissionEntry));
                            FLAG_I_RECURSIVE = 1;FLAG_I_SUCC = 1;
                        }else if(le->pe.permission[2] & Q_PERMISSION_C){
                            qList_initdesc(tmple.ld);
                            memcpy(&(tmple.data),&(lv1mask),sizeof(Level1));
                            memcpy(&(tmple.pe),&(lv1mask),sizeof(PermissionEntry));
                            FLAG_I_SUCC = 1;
                        }
                    }
                    if(FLAG_I_RECURSIVE){
                        qList_foreach((le->ld),iiter){
                            Level2Entry *lle = iiter->data;
                            Level2Entry tmplle;
                            ui FLAG_II_RECURSIVE = 0,FLAG_II_SUCC = 0;
                            if(lle->pe.ownerid == q.userId || FLAG_PRIV){
                                // full permission
                                qList_initdesc(tmplle.ld);
                                memcpy(&(tmplle.data),&(lle->data),sizeof(Level2));
                                memcpy(&(tmplle.pe),&(lle->pe),sizeof(PermissionEntry));
                                FLAG_II_RECURSIVE = 1;FLAG_II_SUCC = 1;
                            }else{
                                if(lle->pe.groupid == q.groupId){
                                    if (lle->pe.permission[1] & Q_PERMISSION_R){
                                        qList_initdesc(tmplle.ld);
                                        memcpy(&(tmplle.data),&(lle->data),sizeof(Level2));
                                        memcpy(&(tmplle.pe),&(lle->pe),sizeof(PermissionEntry));
                                        FLAG_II_RECURSIVE = 1;FLAG_II_SUCC = 1;
                                    }else if(lle->pe.permission[1] & Q_PERMISSION_C){
                                        qList_initdesc(tmplle.ld);
                                        memcpy(&(tmplle.data),&(lv2mask),sizeof(Level2));
                                        memcpy(&(tmplle.pe),&(lle->pe),sizeof(PermissionEntry));
                                        FLAG_II_SUCC = 1;
                                    }
                                }else if(lle->pe.permission[2] & Q_PERMISSION_R){
                                    qList_initdesc(tmplle.ld);
                                    memcpy(&(tmplle.data),&(lle->data),sizeof(Level2));
                                    memcpy(&(tmplle.pe),&(lle->pe),sizeof(PermissionEntry));
                                    FLAG_II_RECURSIVE = 1;FLAG_II_SUCC = 1;
                                }else if(lle->pe.permission[2] & Q_PERMISSION_C){
                                    qList_initdesc(tmplle.ld);
                                    memcpy(&(tmplle.data),&(lv2mask),sizeof(Level2));
                                    memcpy(&(tmplle.pe),&(lle->pe),sizeof(PermissionEntry));
                                    FLAG_II_SUCC = 1;
                                }
                            }
                            if(FLAG_II_RECURSIVE){
                                qList_foreach((lle->ld),iiiter){
                                    Level3Entry *llle = iiiter->data;
                                    Level3Entry tmpllle;
                                    ui FLAG_III_SUCC = 0;
                                    if(llle->pe.ownerid == q.userId || FLAG_PRIV){
                                        // full permission
                                        memcpy(&(tmpllle.data),&(llle->data),sizeof(Level3));
                                        memcpy(&(tmpllle.pe),&(llle->pe),sizeof(PermissionEntry));
                                        FLAG_III_SUCC = 1;
                                    }else{
                                        if(llle->pe.groupid == q.groupId){
                                            if (llle->pe.permission[1] & Q_PERMISSION_R){
                                                memcpy(&(tmpllle.data),&(llle->data),sizeof(Level3));
                                                memcpy(&(llle->pe),&(tmpllle.pe),sizeof(PermissionEntry));
                                                FLAG_III_SUCC = 1;
                                            }else if(llle->pe.permission[1] & Q_PERMISSION_C){
                                                memcpy(&(tmpllle.data),&(lv3mask),sizeof(Level3));
                                                memcpy(&(tmpllle.pe),&(llle->pe),sizeof(PermissionEntry));
                                                FLAG_III_SUCC = 1;
                                            }
                                        }else if(llle->pe.permission[2] & Q_PERMISSION_R){
                                            memcpy(&(tmpllle.data),&(llle->data),sizeof(Level3));
                                            memcpy(&(tmpllle.pe),&(llle->pe),sizeof(PermissionEntry));
                                            FLAG_III_SUCC = 1;
                                        }else if(llle->pe.permission[2] & Q_PERMISSION_C){
                                            memcpy(&(tmpllle.data),&(lv3mask),sizeof(Level2));
                                            memcpy(&(tmpllle.pe),&(llle->pe),sizeof(PermissionEntry));
                                            FLAG_III_SUCC = 1;
                                        }
                                    }
                                    if(FLAG_III_SUCC){
                                        qList_push_back(tmplle.ld,tmpllle);
                                    }
                                }
                            }
                            if(FLAG_II_SUCC){
                                qList_push_back(tmple.ld,tmplle);
                            }
                        }
                        if(FLAG_I_SUCC){
                            qList_push_back(tmprld,tmple);
                        }
                    }
                }
                UNLOCKDATA;
                qListDescriptor ser_tmprld = qSerialize(&tmprld,sizeof(qListDescriptor));
                // test only
                qListDescriptor *tmpurld = qUnserialize(ser_tmprld,YES_IT_IS_A_LIST);
                qList_foreach(*data,titer){
                    Level1Entry *le = titer->data;
                    fprintf(stderr,"%u %s %s %u%u%u\n",le->pe.entryid,le->data.carId,le->data.carName,(unsigned int)le->pe.permission[0],(unsigned int)le->pe.permission[1],(unsigned int)le->pe.permission[2]);
                    if(le->ld.size != 0){
                        qList_foreach(le->ld,tiiter){
                            Level2Entry *lle = tiiter->data;
                            fprintf(stderr,"    %u %s %s %u%u%u\n",lle->pe.entryid,lle->data.carId,lle->data.carName,(unsigned int)lle->pe.permission[0],(unsigned int)lle->pe.permission[1],(unsigned int)lle->pe.permission[2]);
                        }
                    }
                }
                NETWRCHECK(*clisock,qAssembleSyncDataReply(0,ser_tmprld));
                qList_foreach(ser_tmprld,fiter){
                    binary_safe_string *rf = fiter->data;
                    qbss_destructor(*rf);
                }
                qList_destructor(ser_tmprld);
            }
            break;
            case 11:
            {
                AppendDataQuery* q = qDisassembleAppendDataQuery(rcontent);
                ui FLAG_PRIV = 0,FLAG_SUCC = 0,tmpid = 0;
                LOCKUSER;
                CHECKPRIV(q->userId,FLAG_PRIV);
                UNLOCKUSER;
                LOCKDATA;
                if(q->entryLvl == 0){
                    ALLOCID(tmpid,lv1ids,FLAG_SUCC);
                    if(FLAG_SUCC){
                        Level1Entry tmpent;
                        memcpy(&(tmpent.data),(rcontent.str)+sizeof(AppendDataQuery),q->datalen);
                        qList_initdesc(tmpent.ld);
                        setpe(tmpent.pe,q->userId,q->groupId,tmpid,umask);
                        qList_push_back(*data,tmpent);
                    }
                }else{
                    qList_foreach(*data,iter){
                        Level1Entry *le = iter->data;
                        if(le->pe.entryid == q->entryIds[0]){
                            // found
                            if(q->entryLvl == 1 ){
                                if((checkperm(le->pe,q->userId,q->groupId) & Q_PERMISSION_W) || FLAG_PRIV){
                                    ALLOCID(tmpid,lv2ids,FLAG_SUCC);
                                    if(FLAG_SUCC){
                                        Level2Entry tmpent;
                                        memcpy(&(tmpent.data),(rcontent.str)+sizeof(AppendDataQuery),q->datalen);
                                        qList_initdesc(tmpent.ld);
                                        setpe(tmpent.pe,q->userId,q->groupId,tmpid,umask);
                                        qList_push_back(le->ld,tmpent);
                                    }
                                }
                            }else if((checkperm(le->pe,q->userId,q->groupId) & Q_PERMISSION_R) || FLAG_PRIV){
                                // entrylvl == 2
                                qList_foreach(le->ld,iiter){
                                    Level2Entry *lle = iiter->data;
                                    if(lle->pe.entryid == q->entryIds[1]){
                                        if((checkperm(lle->pe,q->userId,q->groupId) & Q_PERMISSION_W) || FLAG_PRIV){
                                            ALLOCID(tmpid,lv3ids,FLAG_SUCC);
                                            if(FLAG_SUCC){
                                                Level3Entry tmpent;
                                                memcpy(&(tmpent.data),(rcontent.str)+sizeof(AppendDataQuery),q->datalen);
                                                setpe(tmpent.pe,q->userId,q->groupId,tmpid,umask);
                                                qList_push_back(lle->ld,tmpent);
                                            }
                                        }
                                        break;
                                    }
                                }
                            }
                            break;
                        }
                    }
                }
                UNLOCKDATA;
                free(q);
                NETWRCHECK(*clisock,qAssembleAppendDataReply(FLAG_SUCC?0:APPEND_FAIL,tmpid));
            }
            break;
            case 12:
            {
                RemoveDataQuery q = qDisassembleRemoveDataQuery(rcontent);
                ui FLAG_PRIV = 0,FLAG_SUCC = 0,tmpid = 0;
                LOCKUSER;
                CHECKPRIV(q.userId,FLAG_PRIV);
                UNLOCKUSER;
                LOCKDATA;
                qList_foreach(*data,iter){
                    Level1Entry *le = iter->data;
                    if(le->pe.entryid == q.entryIds[0]){
                        // found
                        if(q.entryLvl == 0 ){
                            if((checkperm(le->pe,q.userId,q.groupId) & Q_PERMISSION_W) || FLAG_PRIV){
                                qList_foreach(le->ld,iiter){
                                    qList_destructor(((Level2Entry*)iiter->data)->ld);
                                }
                                qList_destructor(le->ld);
                                qList_erase_elem(*data,iter);
                                FLAG_SUCC = 1;
                            }
                        }else if((checkperm(le->pe,q.userId,q.groupId) & Q_PERMISSION_R) || FLAG_PRIV){
                            qList_foreach(le->ld,iiter){
                                Level2Entry *lle = iiter->data;
                                if(lle->pe.entryid == q.entryIds[1]){
                                    if(q.entryLvl == 1){
                                        if((checkperm(lle->pe,q.userId,q.groupId) & Q_PERMISSION_W) || FLAG_PRIV){
                                            qList_destructor(lle->ld);
                                            qList_erase_elem(le->ld,iiter);
                                            FLAG_SUCC = 1;
                                        }
                                    }else if((checkperm(lle->pe,q.userId,q.groupId)&Q_PERMISSION_R)||FLAG_PRIV){
                                        qList_foreach(lle->ld,iiiter){
                                            Level3Entry *llle = iiiter->data;
                                            if(llle->pe.entryid == q.entryIds[2]){
                                                if((checkperm(llle->pe,q.userId,q.groupId) & Q_PERMISSION_W) || FLAG_PRIV){
                                                    qList_erase_elem(lle->ld,iiiter);
                                                    FLAG_SUCC = 1;
                                                }
                                                break;
                                            }
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                        break;
                    }
                }
                UNLOCKDATA;
                NETWRCHECK(*clisock,qAssembleRemoveDataReply(FLAG_SUCC?0:ERASE_FAIL));
            }
            break;
            case 13:
            {
                AlterDataQuery *q = qDisassembleAlterDataQuery(rcontent);
                ui FLAG_PRIV = 0,FLAG_SUCC = 0,tmpid = 0;
                LOCKUSER;
                CHECKPRIV(q->userId,FLAG_PRIV);
                UNLOCKUSER;
                LOCKDATA;
                qList_foreach(*data,iter){
                    Level1Entry *le = iter->data;
                    if(le->pe.entryid == q->entryIds[0]){
                        // found
                        if(q->entryLvl == 0 ){
                            if((checkperm(le->pe,q->userId,q->groupId) & Q_PERMISSION_W) || FLAG_PRIV){
                                memcpy(&(le->data),(rcontent.str)+sizeof(AlterDataQuery),q->datalen);
                                FLAG_SUCC = 1;
                            }
                        }else if((checkperm(le->pe,q->userId,q->groupId) & Q_PERMISSION_R) || FLAG_PRIV){
                            qList_foreach(le->ld,iiter){
                                Level2Entry *lle = iiter->data;
                                if(lle->pe.entryid == q->entryIds[1]){
                                    if(q->entryLvl == 1){
                                        if((checkperm(lle->pe,q->userId,q->groupId) & Q_PERMISSION_W) || FLAG_PRIV){
                                            memcpy(&(lle->data),(rcontent.str)+sizeof(AlterDataQuery),q->datalen);
                                            FLAG_SUCC = 1;
                                        }
                                    }else if((checkperm(lle->pe,q->userId,q->groupId)&Q_PERMISSION_R)||FLAG_PRIV){
                                        qList_foreach(lle->ld,iiiter){
                                            Level3Entry *llle = iiiter->data;
                                            if(llle->pe.entryid == q->entryIds[2]){
                                                if((checkperm(llle->pe,q->userId,q->groupId) & Q_PERMISSION_W) || FLAG_PRIV){
                                                    memcpy(&(llle->data),(rcontent.str)+sizeof(AlterDataQuery),q->datalen);
                                                    FLAG_SUCC = 1;
                                                }
                                                break;
                                            }
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                        break;
                    }
                }
                UNLOCKDATA;
                free(q);
                NETWRCHECK(*clisock,qAssembleAlterDataReply(FLAG_SUCC?0:ALTER_FAIL));
            }
            break;
            case 20:
            {
                AlterEntryOwnerQuery q = qDisassembleAlterEntryOwnerQuery(rcontent);
                ui FLAG_PRIV = 0,FLAG_SUCC = 0,tmpid = 0;
                LOCKUSER;
                CHECKPRIV(q.userId,FLAG_PRIV);
                UNLOCKUSER;
                LOCKDATA;
                qList_foreach(*data,iter){
                    Level1Entry *le = iter->data;
                    if(le->pe.entryid == q.entryIds[0]){
                        // found
                        if(q.entryLvl == 0 ){
                            if((checkperm(le->pe,q.userId,q.groupId) & Q_PERMISSION_W) || FLAG_PRIV){
                                le->pe.ownerid = q.destuid;
                                FLAG_SUCC = 1;
                            }
                        }else if((checkperm(le->pe,q.userId,q.groupId) & Q_PERMISSION_R) || FLAG_PRIV){
                            qList_foreach(le->ld,iiter){
                                Level2Entry *lle = iiter->data;
                                if(lle->pe.entryid == q.entryIds[1]){
                                    if(q.entryLvl == 1){
                                        if((checkperm(lle->pe,q.userId,q.groupId) & Q_PERMISSION_W) || FLAG_PRIV){
                                            lle->pe.ownerid = q.destuid;
                                            FLAG_SUCC = 1;
                                        }
                                    }else if((checkperm(lle->pe,q.userId,q.groupId)&Q_PERMISSION_R)||FLAG_PRIV){
                                        qList_foreach(lle->ld,iiiter){
                                            Level3Entry *llle = iiiter->data;
                                            if(llle->pe.entryid == q.entryIds[2]){
                                                if((checkperm(llle->pe,q.userId,q.groupId) & Q_PERMISSION_W) || FLAG_PRIV){
                                                    llle->pe.ownerid = q.destuid;
                                                    FLAG_SUCC = 1;
                                                }
                                                break;
                                            }
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                        break;
                    }
                }
                UNLOCKDATA;
                NETWRCHECK(*clisock,qAssembleAlterEntryOwnerReply(FLAG_SUCC?0:PERMISSION_DENIED));
            }
            break;
            case 21:
            {
                AlterEntryGroupQuery q = qDisassembleAlterEntryGroupQuery(rcontent);
                ui FLAG_PRIV = 0,FLAG_SUCC = 0,tmpid = 0;
                LOCKUSER;
                CHECKPRIV(q.userId,FLAG_PRIV);
                UNLOCKUSER;
                LOCKDATA;
                qList_foreach(*data,iter){
                    Level1Entry *le = iter->data;
                    if(le->pe.entryid == q.entryIds[0]){
                        // found
                        if(q.entryLvl == 0 ){
                            if((checkperm(le->pe,q.userId,q.groupId) & Q_PERMISSION_W) || FLAG_PRIV){
                                le->pe.groupid = q.destgid;
                                FLAG_SUCC = 1;
                            }
                        }else if((checkperm(le->pe,q.userId,q.groupId) & Q_PERMISSION_R) || FLAG_PRIV){
                            qList_foreach(le->ld,iiter){
                                Level2Entry *lle = iiter->data;
                                if(lle->pe.entryid == q.entryIds[1]){
                                    if(q.entryLvl == 1){
                                        if((checkperm(lle->pe,q.userId,q.groupId) & Q_PERMISSION_W) || FLAG_PRIV){
                                            lle->pe.groupid = q.destgid;
                                            FLAG_SUCC = 1;
                                        }
                                    }else if((checkperm(lle->pe,q.userId,q.groupId)&Q_PERMISSION_R)||FLAG_PRIV){
                                        qList_foreach(lle->ld,iiiter){
                                            Level3Entry *llle = iiiter->data;
                                            if(llle->pe.entryid == q.entryIds[2]){
                                                if((checkperm(llle->pe,q.userId,q.groupId) & Q_PERMISSION_W) || FLAG_PRIV){
                                                    llle->pe.groupid = q.destgid;
                                                    FLAG_SUCC = 1;
                                                }
                                                break;
                                            }
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                        break;
                    }
                }
                UNLOCKDATA;
                NETWRCHECK(*clisock,qAssembleAlterEntryGroupReply(FLAG_SUCC?0:PERMISSION_DENIED));
            }
            break;
            case 22:
            {
                AlterEntryPermissionQuery q = qDisassembleAlterEntryPermissionQuery(rcontent);
                ui FLAG_PRIV = 0,FLAG_SUCC = 0,tmpid = 0;
                fprintf(stderr,"Attempt chmod %u %u %u %u for %u%u%u\n",q.entryLvl,q.entryIds[0],q.entryIds[1],q.entryIds[2],(unsigned int)q.permission[0],(unsigned int)q.permission[1],(unsigned int)q.permission[2]);
                LOCKUSER;
                CHECKPRIV(q.userId,FLAG_PRIV);
                UNLOCKUSER;
                LOCKDATA;
                qList_foreach(*data,iter){
                    Level1Entry *le = iter->data;
                    if(le->pe.entryid == q.entryIds[0]){
                        // found
                        if(q.entryLvl == 0 ){
                            if((checkperm(le->pe,q.userId,q.groupid) & Q_PERMISSION_W) || FLAG_PRIV){
                                memcpy(le->pe.permission,q.permission,permsize);
                                FLAG_SUCC = 1;
                            }
                        }else if((checkperm(le->pe,q.userId,q.groupid) & Q_PERMISSION_R) || FLAG_PRIV){
                            qList_foreach(le->ld,iiter){
                                Level2Entry *lle = iiter->data;
                                if(lle->pe.entryid == q.entryIds[1]){
                                    if(q.entryLvl == 1){
                                        if((checkperm(lle->pe,q.userId,q.groupid) & Q_PERMISSION_W) || FLAG_PRIV){
                                            memcpy(lle->pe.permission,q.permission,permsize);
                                            FLAG_SUCC = 1;
                                        }
                                    }else if((checkperm(lle->pe,q.userId,q.groupid)&Q_PERMISSION_R)||FLAG_PRIV){
                                        qList_foreach(lle->ld,iiiter){
                                            Level3Entry *llle = iiiter->data;
                                            if(llle->pe.entryid == q.entryIds[2]){
                                                if((checkperm(llle->pe,q.userId,q.groupid) & Q_PERMISSION_W) || FLAG_PRIV){
                                                    memcpy(llle->pe.permission,q.permission,permsize);
                                                    FLAG_SUCC = 1;
                                                }
                                                break;
                                            }
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                        break;
                    }
                }
                UNLOCKDATA;
                NETWRCHECK(*clisock,qAssembleAlterEntryPermissionReply(FLAG_SUCC?0:PERMISSION_DENIED));
            }
            break;
            case 30:
            {
                StopServerQuery q = qDisassembleStopServerQuery(rcontent);
                fprintf(stderr,"[SHUT] Connection %d sent shutdown passphrase %s\n",clisock->desc,q.adminpass);
                if(fullstrcmp(q.adminpass,SHUTDOWN_PASSPHRASE)){
                    fprintf(stderr,"[SHUT] Connection %d successfully inited server shutting down process.\n",clisock->desc);
                    qSocket_close(*clisock);
                    RUNNING = 0;
                    FLAG_CONT = 0;
                    // send a fake connection
                    qSocket fakesock = qSocket_constructor(qIPv4,qStreamSocket,qDefaultProto);
                    qSocket_open(fakesock);
                    qStreamSocket_connect(fakesock,globargv[1]);
                    qStreamSocket_write(fakesock,"aaaaaaaaaaaaaaaaaaaaaaaaa",25);
                    qSocket_close(fakesock);
                    fprintf(stderr,"[SHUT] Successfully closed fakesock.\n");
                }
            }
            break;
            default:
            fprintf(stderr,"[WARN] Invalid input from conn %u. Force closing conn..\n",clisock->desc);
            qSocket_close(*clisock);
            FLAG_CONT = 0;
            break;
        }
        qbss_destructor(rcontent);
    }
    fprintf(stderr,"[INFO] Client %d disconnected.\n",clisock->desc);
    connclock.lock(connclock);
    cliconns --;
    connclock.unlock(connclock);
    return NULL;
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
#include "permissionctl.h"
#include <string.h>

#define FULLPERM (Q_PERMISSION_C|Q_PERMISSION_R|Q_PERMISSION_W)

void q__setpe(PermissionEntry* pe,unsigned int uid,unsigned int gid,unsigned int eid,unsigned char mask[3]){
    pe->ownerid = uid;
    pe->groupid = gid;
    pe->entryid = eid;
    memcpy(pe->permission,mask,3*sizeof(unsigned char));
}

unsigned char checkperm(PermissionEntry pe,unsigned int uid,unsigned int gid){
    if(pe.ownerid == uid){
        return FULLPERM;
    }else if(pe.groupid == gid){
        return pe.permission[1];
    }else{
        return pe.permission[2];
    }
}
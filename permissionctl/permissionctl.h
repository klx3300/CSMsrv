#ifndef Q_CDES_PERMISSION_H
#define Q_CDES_PERMISSION_H

#define Q_PERMISSION_C ((unsigned char)4)
#define Q_PERMISSION_R ((unsigned char)2)
#define Q_PERMISSION_W ((unsigned char)1)

struct q_UserData_st{
    unsigned int uid;
    unsigned int gid;
    char username[256];
    char password[256];
};

struct q_GroupData_st{
    unsigned int gid;
    char groupname[256];
};

struct q_PermissionEntry_st{
    unsigned int entryid; // unique, not 0
    unsigned int ownerid;
    unsigned int groupid;
    unsigned char permission[3]; // owner-group-others <=> 0-1-2 <=> u-g-o
};

typedef struct q_UserData_st UserData;
typedef struct q_GroupData_st GroupData;
typedef struct q_PermissionEntry_st PermissionEntry;


#endif
#ifndef Q_CDES_PROTO_H
#define Q_CDES_PROTO_H

struct q_UniversalHeader_st{
    unsigned int size; // include the size of queryid;
    unsigned char queryid;
};

// id=0
struct q_LoginQuery_st{
    char username[256];
    char password[256];
};

struct q_LoginReply_st{
    unsigned int errNo;
    unsigned int userId;
    unsigned int groupId;
};

// id=1
struct q_AlterPassQuery_st{
    unsigned int userId;
    char newpass[256];
};

struct q_AlterPassReply_st{
    unsigned int errNo;
};

// id=2
struct q_ListGroupQuery_st{
    unsigned int userId;
};

struct q_ListGroupReply_st{
    unsigned int groupNum;
    // followed with group array(GroupData[groupNum])
};

// id=3
struct q_AlterGroupQuery_st{
    unsigned int userId;
    unsigned int groupId;
};

struct q_AlterGroupReply_st{
    unsigned int errNo;
};

// id=4
struct q_RemoveUserQuery_st{
    unsigned int userId;
    unsigned int targetUserId;
};

struct q_RemoveUserReply_st{
    unsigned int errNo;
};

// id=5
struct q_RemoveGroupQuery_st{
    unsigned int userId;
    unsigned int targetGroupId;
};

struct q_RemoveGroupReply_st{
    unsigned int errNo;
};

// ================= divide:permissionctl -- userdata =================

// id=10
struct q_SyncDataQuery_st{
    unsigned int userId;
    unsigned int groupId;
};

struct q_SyncDataReply_st{
    unsigned int errNo;
    // followed with 3*{unsigned int length;char data[length];}
    // combine them and use unserialization to convert them to readable stuff;
};

// id=11
struct q_AppendDataQuery_st{
    unsigned int userId;
    unsigned int groupId;
    unsigned int entryIds[3]; // 0 means terminating.
    // follow with the corresponding data.
};

struct q_AppendDataReply_st{
    unsigned int errNo;
    unsigned int entryId;
};

// id=12
struct q_RemoveDataQuery_st{
    unsigned int userId;
    unsigned int groupId;
    unsigned int entryIds[3];
};

struct q_RemoveDataReply_st{
    unsigned int errNo;
};

// id=13
struct q_AlterDataQuery_st{
    unsigned int userId;
    unsigned int groupId;
    unsigned int entryIds[3];
    // follow with the corresponding data.
};

struct q_AlterDataReply_st{
    unsigned int errNo;
};

// ================== divide: dataMgr -- permissionMgr ==============

// id=20
struct q_AlterEntryOwnerQuest_st{
    unsigned int userId;
    unsigned int entryIds[3];
};

struct q_AlterEntryOwnerReply_st{
    unsigned int errNo;
};

// id=21
struct q_AlterEntryGroupQuest_st{
    unsigned int userId;
    unsigned int groupId;
    unsigned int entryIds[3];
};

struct q_ALterEntryGroupReply_st{
    unsigned int errNo;
};

// id=22
struct q_AlterEntryPermissionQuest_st{
    unsigned int userId;
    unsigned int entryIds[3];
    unsigned char permission;
};

struct q_AlterEntryPermissionReply_st{
    unsigned int errNo;
};

// ================ divide: definition =========================
typedef struct q_UniversalHeader_st UniversalHeader;
typedef struct q_LoginQuery_st LoginQuery;
typedef struct q_LoginReply_st LoginReply;
typedef struct q_AlterPassQuery_st AlterPassQuery;
typedef struct q_AlterPassReply_st AlterPassReply;
typedef struct q_ListGroupQuery_st ListGroupQuery;
typedef struct q_ListGroupReply_st ListGroupReply;
typedef struct q_AlterGroupQuery_st AlterGroupQuery;
typedef struct q_AlterGroupReply_st AlterGroupReply;
typedef struct q_RemoveUserQuery_st RemoveUserQuery;
typedef struct q_RemoveUserReply_st RemoveUserReply;
typedef struct q_RemoveGroupQuery_st RemoveGroupQuest;
typedef struct q_RemoveGroupReply_st RemoveGroupReply;
typedef struct q_SyncDataQuery_st SyncDataQuery;
typedef struct q_SyncDataReply_st SyncDataReply;
typedef struct q_AppendDataQuery_st AppendDataQuery;
typedef struct q_AppendDataReply_st AppendDataReply;
typedef struct q_RemoveDataQuery_st RemoveDataQuery;
typedef struct q_RemoveDataReply_st RemoveDataReply;
typedef struct q_AlterDataQuery_st AlterDataQuery;
typedef struct q_AlterDataReply_st AlterDataReply;
typedef struct q_AlterEntryOwnerQuest_st AlterEntryOwnerQuest;
typedef struct q_AlterEntryOwnerReply_st AlterEntryOwnerReply;
typedef struct q_AlterEntryGroupQuest_st AlterEntryGroupQuest;
typedef struct q_AlterEntryGroupReply_st AlterEntryGroupReply;
typedef struct q_AlterEntryPermissionQuest_st AlterEntryPermissionQuest;
typedef struct q_AlterEntryPermissionReply_st AlterEntryPermissionReply;


#endif
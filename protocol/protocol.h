#ifndef Q_CDES_PROTO_H
#define Q_CDES_PROTO_H

#include "../zhwkre/bss.h"
#include "../zhwkre/list.h"

struct q_UniversalHeader_st{
    unsigned int size; // include the size of queryid;
    unsigned char queryid;
};

// id=0
struct q_LoginQuery_st{
    unsigned int username_len;
    char username[256];
    unsigned int password_len;
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
    unsigned int passlen;
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
    unsigned int datasize;
    // followed with serialized qList data
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
    unsigned int datalen;
    // follow with the corresponding data.
};

struct q_AlterDataReply_st{
    unsigned int errNo;
};

// ================== divide: dataMgr -- permissionMgr ==============

// id=20
struct q_AlterEntryOwnerQuery_st{
    unsigned int userId;
    unsigned int entryIds[3];
};

struct q_AlterEntryOwnerReply_st{
    unsigned int errNo;
};

// id=21
struct q_AlterEntryGroupQuery_st{
    unsigned int userId;
    unsigned int groupId;
    unsigned int entryIds[3];
};

struct q_ALterEntryGroupReply_st{
    unsigned int errNo;
};

// id=22
struct q_AlterEntryPermissionQuery_st{
    unsigned int userId;
    unsigned int entryIds[3];
    unsigned char permission;
};

struct q_AlterEntryPermissionReply_st{
    unsigned int errNo;
};

// ================ divide: definition =========================

typedef struct q_UniversalHeader_st UniversalHeader;
// id=0
typedef struct q_LoginQuery_st LoginQuery;
// id=0
typedef struct q_LoginReply_st LoginReply;
// id=1
typedef struct q_AlterPassQuery_st AlterPassQuery;
// id=1
typedef struct q_AlterPassReply_st AlterPassReply;
// id=2
typedef struct q_ListGroupQuery_st ListGroupQuery;
// id=2
typedef struct q_ListGroupReply_st ListGroupReply;
// id=3
typedef struct q_AlterGroupQuery_st AlterGroupQuery;
// id=3
typedef struct q_AlterGroupReply_st AlterGroupReply;
// id=4
typedef struct q_RemoveUserQuery_st RemoveUserQuery;
// id=4
typedef struct q_RemoveUserReply_st RemoveUserReply;
// id=5
typedef struct q_RemoveGroupQuery_st RemoveGroupQuery;
// id=5
typedef struct q_RemoveGroupReply_st RemoveGroupReply;
// id=10
typedef struct q_SyncDataQuery_st SyncDataQuery;
// id=10
typedef struct q_SyncDataReply_st SyncDataReply;
// id=11
typedef struct q_AppendDataQuery_st AppendDataQuery;
// id=11
typedef struct q_AppendDataReply_st AppendDataReply;
// id=12
typedef struct q_RemoveDataQuery_st RemoveDataQuery;
// id=12
typedef struct q_RemoveDataReply_st RemoveDataReply;
// id=13
typedef struct q_AlterDataQuery_st AlterDataQuery;
// id=13
typedef struct q_AlterDataReply_st AlterDataReply;
// id=20
typedef struct q_AlterEntryOwnerQuery_st AlterEntryOwnerQuery;
// id=20
typedef struct q_AlterEntryOwnerReply_st AlterEntryOwnerReply;
// id=21
typedef struct q_AlterEntryGroupQuery_st AlterEntryGroupQuery;
// id=21
typedef struct q_AlterEntryGroupReply_st AlterEntryGroupReply;
// id=22
typedef struct q_AlterEntryPermissionQuery_st AlterEntryPermissionQuery;
// id=22
typedef struct q_AlterEntryPermissionReply_st AlterEntryPermissionReply;

// ========================divide: useful functions=====================

// all disassemble function not include the UniversalHeader

binary_safe_string qAssembleLoginQuery(binary_safe_string username,binary_safe_string password);
LoginQuery qDisassembleLoginQuery(binary_safe_string input);

binary_safe_string qAssembleLoginReply(unsigned int errNo,unsigned int userId,unsigned int groupId);
LoginReply qDisassembleLoginReply(binary_safe_string input);

binary_safe_string qAssembleAlterPassQuery(unsigned int uid,unsigned int passlen,binary_safe_string password);
AlterPassReply qDisassembleAlterPassQuery(binary_safe_string bss);

binary_safe_string qAssembleListGroupQuery(unsigned int uid);
ListGroupQuery qDisassembleListGroupQuery(binary_safe_string input);

binary_safe_string qAssembleListGroupReply(qListDescriptor grouplist);
qListDescriptor* qDisassembleListGroupReply(binary_safe_string input);



#endif
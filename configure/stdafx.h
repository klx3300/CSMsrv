#ifndef Q_CDES_STDAFX_H
#define Q_CDES_STDAFX_H

#include "conf.h"
#include "../permissionctl/permissionctl.h"
#include "../zhwkre/listd.h"

struct q_Level1Entry_st{
    qListDescriptor ld;
    Level1 data;
    PermissionEntry pe;
};

struct q_Level2Entry_st{
    qListDescriptor ld;
    Level2 data;
    PermissionEntry pe;
};

struct q_Level3Entry_st{
    Level3 data;
    PermissionEntry pe;
};

typedef struct q_Level1Entry_st Level1Entry;
typedef struct q_Level2Entry_st Level2Entry;
typedef struct q_Level3Entry_st Level3Entry;

#endif
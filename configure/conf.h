#ifndef Q_CDES_CONF_H
#define Q_CDES_CONF_H

struct q_CarType_st{
    char carId[20];
    char carName[20];
    int weight;
    int seatNum;
    int length;
    int width;
    int height;
};

struct q_PaymentRecord_st{
    char carId[20];
    char paydate[12]; // arrange like yyyy/mm/dd
    double amount;
    double remain;
    char sellerId[20];
};

struct q_SellInfo_st{
    char carId[20];
    char carName[20];
    unsigned int color; // 0x00FFFFFF ARGB format
    char selldate[12]; // yyyy/mm/dd
    char customerName[20];
    char customerId[18];
    char customerTel[20];
    double priceSum;
};

typedef struct q_SellInfo_st Level2;
typedef struct q_PaymentRecord_st Level3;
typedef struct q_CarType_st Level1;

#endif
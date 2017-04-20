#include "stm32f4xx.h"

//#define JudgeBufferLength       150
//#define JudgeFrameLength_1      46
//#define JudgeFrameLength_2      11
//#define JudgeFrameLength_3      24
#define JudgeBufferLength       200
#define JudgeFrameLength_1      44
#define JudgeFrameLength_2      12
#define JudgeFrameLength_3      25
#define JudgeHeaderLength        7
#define JudgeCommandIDIndex     5
#define JudgeFrameHeader        0xA5        //帧头 

void judging_system_init(void);

//裁判系统结构体
typedef struct
{
    float RealVoltage;                  //实时电压
    float RealCurrent;                  //实时电流
    int16_t LastBlood;                  //剩余血量
    uint8_t LastHartID;                 //上次收到伤害的装甲板ID号
    float LastShotSpeed;                //上次射击速度
    float LastShotFreq;
    float RemainBuffer;
		int16_t ArmorDecrease;
    int16_t OverShootSpeedDecrease;
    int16_t OverShootFreqDecrease;
    int16_t OverPowerDecrease;
    int16_t ModuleOfflineDecrease;
    uint8_t Updated;

}InfantryJudge_Struct;

//格式转换联合体
typedef union
{
    uint8_t U[4];
    float F;
    int I;
}FormatTrans;

//the data will be stored in the following struct
extern InfantryJudge_Struct InfantryJudge;
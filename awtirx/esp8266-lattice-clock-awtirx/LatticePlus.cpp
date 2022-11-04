#include "LatticePlus.h"

LatticePlus::LatticePlus()
{
    init();
}

LatticePlus::LatticePlus(System *systems)
{
    systemObj = systems;
    init();
}

/**
   @brief 单个点阵纵向移动函数

   @param tempdata 取模数据
   @param dl  移动长度
   @return uint8_t
*/
uint8_t *LatticePlus::single_down_move(const unsigned char *tempdata, int dl)
{
    uint8_t *arr = new uint8_t[8];
    memcpy(arr, tempdata, 8);
    if (dl > 8 || dl % 8 == 0)
    {
        return arr;
    }
    uint8_t tmp = tempdata[7];
    for (int k = 7; k > 0; k--)
    {
        arr[k] = tempdata[k - dl];
    }
    arr[0] = tmp;
    return arr;
}

/**
   @brief 单个点阵横向移动函数1

   @param tempdata 取模数据
   @param ll  移动长度
   @return uint8_t
*/
uint8_t *LatticePlus::single_left_move1(uint8_t *tempdata, int ll)
{
    uint8_t *arr = new uint8_t[8];
    // 如果横向位移小于最大位移,则使用最简单的模式
    for (int i = 0; i < 8; i++)
    {
        arr[i] = tempdata[i] >> ll;
    }
    return arr;
}

/**
   @brief 单个点阵横向移动函数2

   @param tempdata 取模数据
   @param ll  移动长度
   @return uint8_t
*/
uint8_t *LatticePlus::single_left_move2(uint8_t *tempdata, int ll)
{
    uint8_t *arr = new uint8_t[8];
    // 2. 根据长度再得到第二个数组
    int rightMove = 8 - ll;
    for (int i = 0; i < 8; i++)
    {
        arr[i] = 0xff & (tempdata[i] << rightMove);
    }
    return arr;
}

void LatticePlus::absolute_position_show(const unsigned char *tempdata, int width, int x, int y)
{
    int t = x / 8;                                  // 记录哪个点阵数据需要被改变
    uint8_t *ydata = single_down_move(tempdata, y); // 获取内容向下移动过后的数据
    int xl = (t > 0) ? (x % 8) : x;                 // 记录当前x坐标,y坐标
    uint8_t *xdata = single_left_move1(ydata, xl);
    for (int i = 0; i < 8; i++)
    {
        data[3 - t][i] = (data[3 - t][i] ^ xdata[i]);
    }
    free(xdata);
    if (xl > (8 - width) && t < 3) // 判断如果内容需要向下一个点阵移动的话,就渲染下一个点阵屏幕的内容
    {
        uint8_t *xdata1 = single_left_move2(ydata, xl);
        for (int i = 0; i < 8; i++)
        {
            data[3 - t - 1][i] = data[3 - t - 1][i] ^ xdata1[i];
        }
        free(xdata1);
    }
    free(ydata);
}

void LatticePlus::boot_animation()
{
    int x = 0;
    for (int i = 0; i < 8; i++)
    {
        absolute_position_show(mogu[0], 8, x, 0); //  显示蘑菇
        refreshLed();
        delay(200);
        initData();
        x++;
        absolute_position_show(mogu[1], 8, x, 0); //  显示蘑菇
        refreshLed();
        delay(200);
        initData();
        x++;
        absolute_position_show(mogu[2], 8, x, 0); //  显示蘑菇
        refreshLed();
        delay(200);
        initData();
        x++;
    }
    delay(500);
}
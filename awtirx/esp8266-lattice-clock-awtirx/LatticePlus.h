#ifndef LATTICE_PLUS_H
#define LATTICE_PLUS_H

#include "Lattice.h"

const unsigned char mogu[3][8] = {
    {0x18, 0x3C, 0x7E, 0xDB, 0xFF, 0x24, 0x5A, 0xA5}, //蘑菇1
    {0x18, 0x3C, 0x7E, 0xDB, 0xFF, 0x24, 0x5A, 0x5A}, //蘑菇2
    {0x18, 0x3C, 0x7E, 0xDB, 0xFF, 0x24, 0x24, 0x5A}  //蘑菇3
};

/**
 * @brief 继承Lattice来写代码,Lattice写的东西实在是太多了,换一个新的写法
 *
 */
class LatticePlus : public Lattice
{
private:
    /**
       @brief 单个点阵纵向移动函数

       @param tempdata 取模数据
       @param dl  移动长度
       @return uint8_t
    */
    uint8_t *single_down_move(const unsigned char *tempdata, int dl);

    /**
       @brief 单个点阵横向移动函数1

       @param tempdata 取模数据
       @param ll  移动长度
       @return uint8_t
    */
    uint8_t *single_left_move1(uint8_t *tempdata, int ll);

    /**
       @brief 单个点阵横向移动函数2

       @param tempdata 取模数据
       @param ll  移动长度
       @return uint8_t
    */
    uint8_t *single_left_move2(uint8_t *tempdata, int ll);

public:
    LatticePlus();

    /**
     * 构造函数
     */
    LatticePlus(System *systems);
    
    /**
     * @brief 根据绝对坐标位置显示8x8点阵内容
     *
     * @param tempdata 8x8点阵取模数据
     * @param width 数据位宽
     * @param x x轴坐标
     * @param y y坐标
     */
    void absolute_position_show(const unsigned char *tempdata, int width, int x, int y);

    /**
     * @brief 开机动画
     *
     */
    void boot_animation();
};
#endif
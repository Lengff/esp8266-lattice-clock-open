#include "Lattice.h"

Lattice::Lattice() { init(); }

void Lattice::init()
{
  latticeSetting.direction = EEPROMTool.loadDataOne(DIRECTION);   // 从eeprom中获取屏幕方向
  latticeSetting.isShutdown = false;                              // 默认是启用点阵屏幕
  latticeSetting.brightness = EEPROMTool.loadDataOne(BRIGHTNESS); // 从eeprom中获取亮度信息
  shutdown(latticeSetting.isShutdown);                            // 是否关闭点阵屏幕
  setBrightness(latticeSetting.brightness, true);                 // 将亮度设置为最低
  for (int i = 0; i < 32; i++)
  {
    latticeSetting.userData[i] = noseticon[i]; // 初始化自定义数据
  }
  latticeSetting.speed = 1; // 自定义动画速度
  initLattice();            // 初始化点阵显示内容
}

void Lattice::shutdown(bool down)
{
  latticeSetting.isShutdown = down;
  for (int i = 0; i < columnLength; i++)
  {
    lc.shutdown(i, down); //启用/停用点阵
  }
}

void Lattice::setBrightness(uint8_t bright, bool save)
{
  if (bright > 15)
  {
    bright = 15; // 超出最大值则设置为最大值
  }
  if (save) // 判断是否保存亮度信息,如果不保存则只进行亮度设置
  {
    latticeSetting.brightness = bright;         // 设置屏幕亮度
    EEPROMTool.saveDataOne(bright, BRIGHTNESS); // 设置亮度信息到EEPROM
  }

  for (int i = 0; i < columnLength; i++)
  {
    lc.setIntensity(i, bright); // 设置亮度
  }
}

void Lattice::setDirection(bool direct)
{
  latticeSetting.direction = direct;         // 设置屏幕显示方向
  EEPROMTool.saveDataOne(direct, DIRECTION); // 设置亮度信息到EEPROM
  refreshLed();
}

void Lattice::reset()
{
  isReset = true;
  tempindex = 3;
}

void Lattice::initLattice()
{
  for (int i = 0; i < columnLength; i++)
  {
    for (int j = 0; j < rowLength; j++)
    {
      data[i][j] = 0x0;
    }
  }
  refreshLed();
}

void Lattice::refreshLed()
{
  if (latticeSetting.direction)
  {
    // 正着显示
    for (int j = 0; j < rowLength; j++)
    {
      for (int i = 0; i < columnLength; i++)
      {
        lc.setRow(i, j, data[i][j]);
      }
    }
  }
  else
  {
    // 反着显示
    for (int j = 0; j < rowLength; j++)
    {
      for (int i = 0; i < columnLength; i++)
      {
        uint8_t dt = data[columnLength - i - 1][rowLength - j - 1];
        uint8_t rd = 0;
        for (int i = 0; i < 8; i++)
        {
          rd += ((dt & 0x01) << (7 - i));
          dt = (dt >> 1);
        }
        lc.setRow(i, j, rd);
      }
    }
  }
}

void Lattice::showDataLed(const unsigned char (*arr)[rowLength])
{
  for (int i = 0; i < columnLength; i++)
  {
    for (int j = 0; j < rowLength - 1; j++)
    {
      data[i][j] = arr[i][j];
    }
  }
  progressBar(rowLength - 1);
  refreshLed();
}

/**
 * 左右翻转(图案左右对称)
 */
void Lattice::reversalLR(uint8_t index)
{
  for (int j = 0; j < 8; j++)
  {
    // 下面的代码是将数字反转
    uint8_t temp = data[index][j];
    for (int i = 0; i < 4; i++)
    {
      uint8_t t1 = temp & (0x1 << i), t2 = temp & (0x80 >> i);
      temp = (temp & (((0xfe ^ 0xff) << i) ^ 0xff)) + (t2 >> ((8 - 1) - i * 2));
      temp = (temp & (((0x7f ^ 0xff) >> i) ^ 0xff)) + (t1 << ((8 - 1) - i * 2));
    }
    data[index][j] = temp;
  }
  refreshLed();
}

/**
 * 上下翻转(图案上下对称)
 */
void Lattice::reversalUD(uint8_t index)
{
  for (int i = 0; i < 4; i++)
  {
    uint8_t tem = data[index][7 - i];
    data[index][7 - i] = data[index][i];
    data[index][i] = tem;
  }
  refreshLed();
}

/**
 * 显示底部进度条
 */
void Lattice::progressBar(uint8_t offset)
{
  if (data[tempindex][offset] == 0xff)
  {
    if (tempindex == 0)
    {
      tempindex = 3;
      for (int i = 0; i < columnLength; i++)
      {
        data[i][offset] = 0x0;
      }
    }
    else
    {
      --tempindex;
    }
  }
  if ((data[tempindex][offset] & 0xff) == 0)
  {
    data[tempindex][offset] = 0x80;
  }
  else
  {

    if ((data[tempindex][offset] & 0xff) == 0x0)
    {
      data[tempindex][offset] = 0x80;
    }
    else
    {
      uint8_t temp = data[tempindex][offset];
      data[tempindex][offset] = temp >> 1;
      data[tempindex][offset] += 0x80;
    }
  }
}

/**
 * 向下移动缓冲数据
 */
void Lattice::upOrDownMove(uint8_t index, uint8_t region, bool direction)
{
  if (direction)
  {
    // 向上移动
    uint8_t temp = data[index][0];
    for (int k = 0; k < 8; k++)
    {
      data[index][k] = (data[index][k] & (region ^ 0xff)) + (data[index][k + 1] & region);
    }
    data[index][7] = (data[index][7] & (region ^ 0xff)) + (temp & region);
  }
  else
  {
    // 向下移动
    uint8_t temp = data[index][7];
    for (int k = 7; k > 0; k--)
    {
      data[index][k] = (data[index][k] & (region ^ 0xff)) + (data[index][k - 1] & region);
    }
    data[index][0] = (data[index][0] & (region ^ 0xff)) + (temp & region);
  }
}

unsigned char *Lattice::getNumData(uint8_t number, bool showzero)
{
  if (number > 99)
  {
    // 超过最大值,返回99
    return getNumData(99, showzero);
  }
  // 分别获取个位数字 和 十位数字
  // const unsigned char *ten = (number / 10) < 1 && showzero ? number_font_small[10] : number_font_small[(number / 10)],  *one = (number % 10) == 0 && showzero ? number_font_small[10] : number_font_small[number % 10];

  const unsigned char *ten = number_font_small[(number / 10)],
                      *one = (number % 10) == 0 && showzero ? number_font_small[10] : number_font_small[number % 10];

  unsigned char *arr = new uint8_t[rowLength];
  for (int i = 0; i < rowLength; i++)
  {
    // 将个位数左移，再加上十位数得到完整数字显示
    arr[i] = ten[i] + ((one[i] & 0xF0) >> 4);
  }
  return arr;
}

unsigned char *Lattice::getBigNumData(uint8_t number, uint8_t offset)
{
  if (number > 9)
  {
    // 超过最大值,返回9
    return getBigNumData(9, offset);
  }
  // 偏移超过最大值,则不进行偏移
  offset = offset > 7 ? 0 : offset;
  const unsigned char *temp = number_font_middle[number];
  unsigned char *arr = new uint8_t[rowLength];
  for (int i = 0; i < rowLength; i++)
  {
    arr[i] = temp[i] >> offset;
  }
  return arr;
}

void Lattice::offsetBuff(uint8_t index, int8_t offset)
{
  uint8_t os = abs(offset);
  if (offset < 0)
  {
    // 向左移动显示为
    uint8_t lastindex = (index == 3) ? 0 : (index + 1);
    uint8_t lastoffset = (0xff >> os) << os;
    uint8_t nextoffset = (lastoffset ^ 0xff) << (8 - os);
    for (int i = 0; i < 8; i++)
    {
      buff[lastindex][i] = (buff[lastindex][i] & lastoffset) +
                           ((buff[index][i] & nextoffset) >> (8 - os));
      buff[index][i] <<= os;
    }
  }
  else
  {
    // 向右移动显示位
    uint8_t lastindex = (index == 0) ? 3 : index - 1;
    uint8_t lastoffset = (0xff >> os);
    uint8_t nextoffset = (lastoffset ^ 0xff) >> (8 - os);
    for (int i = 0; i < 8; i++)
    {
      buff[lastindex][i] = (buff[lastindex][i] & lastoffset) +
                           ((buff[index][i] & (nextoffset)) << (8 - os));
      buff[index][i] >>= os;
    }
  }
}

void Lattice::offsetData(uint8_t index, int8_t offset)
{
  uint8_t os = abs(offset);
  if (offset < 0)
  {
    // 向左移动显示为
    uint8_t lastindex = (index == 3) ? 0 : (index + 1);
    uint8_t lastoffset = (0xff >> os) << os;
    uint8_t nextoffset = (lastoffset ^ 0xff) << (8 - os);
    for (int i = 0; i < 8; i++)
    {
      data[lastindex][i] = (data[lastindex][i] & lastoffset) +
                           ((data[index][i] & nextoffset) >> (8 - os));
      data[index][i] <<= os;
    }
  }
  else
  {
    // 向右移动显示位
    uint8_t lastindex = (index == 0) ? 3 : index - 1;
    uint8_t lastoffset = (0xff >> os);
    uint8_t nextoffset = (lastoffset ^ 0xff) >> (8 - os);
    for (int i = 0; i < 8; i++)
    {
      data[lastindex][i] = (data[lastindex][i] & lastoffset) +
                           ((data[index][i] & (nextoffset)) << (8 - os));
      data[index][i] >>= os;
    }
  }
}

void Lattice::showLongIcon(uint8_t index)
{
  switch (index)
  {
  case 0:
    showDataLed(wifiicon);
    break;
  case 1:
    showDataLed(nowifiicon);
    break;
  case 2:
    showDataLed(updateClock);
    break;
  case 3:
    showDataLed(otaokicon);
    break;
  case 4:
    showDataLed(otafailicon);
    break;
  default:
    break;
  }
}

void Lattice::showNum(uint8_t index, int nums)
{
  unsigned char *tmp = getNumData(nums, false);
  for (int i = 0; i < rowLength; i++)
  {
    data[index][i] = tmp[i];
  }
  free(tmp);
  refreshLed();
}

void Lattice::rightMoveData(bool isright)
{
  if (isright)
  {
    // 向左移动的函数方法
    for (int i = 0; i < rowLength; i++)
    {
      unsigned char first = (data[0][i] & 0x80) >> (rowLength - 1);
      for (int j = 1; j < columnLength; j++)
      {
        unsigned char temp = (data[j][i] & 0x80) >> (rowLength - 1);
        data[j][i] = ((data[j][i] & 0x7F) << 1) + first;
        first = temp;
        if (j == (columnLength - 1))
        {
          data[0][i] = ((data[0][i] & 0x7F) << 1) + first;
        }
      }
    }
  }
  else
  {
    // 向右移动的函数方法
    for (int i = 0; i < rowLength; i++)
    {
      unsigned char first = (data[0][i] & 0x1) << (rowLength - 1);
      for (int j = columnLength - 1; j >= 0; j--)
      {
        unsigned char temp = (data[j][i] & 0x1) << (rowLength - 1);
        data[j][i] = ((data[j][i] & 0xfe) >> 1) + first;
        first = temp;
        if (j == columnLength)
        {
          data[0][i] = ((data[0][i] & 0xfe) >> 1) + first;
        }
      }
    }
  }
  refreshLed();
}

void Lattice::rightMoveBuff()
{
  for (int i = 0; i < rowLength; i++)
  {
    uint8_t f1 = (data[0][i] & 0x80) >> (rowLength - 1);
    uint8_t f2 = (buff[0][i] & 0x80) >> (rowLength - 1);
    for (int j = 1; j < columnLength; j++)
    {
      uint8_t t1 = (data[j][i] & 0x80) >> (rowLength - 1);
      uint8_t t2 = (buff[j][i] & 0x80) >> (rowLength - 1);
      data[j][i] = ((data[j][i] & 0x7F) << 1) + f1;
      buff[j][i] = ((buff[j][i] & 0x7F) << 1) + f2;
      f1 = t1;
      f2 = t2;
      if (j == (columnLength - 1))
      {
        data[0][i] = (data[0][i] << 1) + f2;
        buff[0][i] = (buff[0][i] << 1) + f1;
      }
    }
  }
  refreshLed();
}

void Lattice::downMoveData(bool isdown)
{
  if (isdown)
  {
    // 向下移动
    for (int i = 0; i < columnLength; i++)
    {
      uint8_t t1 = data[i][(rowLength - 1)];
      for (int k = (rowLength - 1); k > 0; k--)
      {
        data[i][k] = data[i][k - 1];
      }
      data[i][0] = t1;
    }
  }
  else
  {
    // 向上移动
    for (int i = 0; i < columnLength; i++)
    {
      uint8_t t1 = data[i][0];
      for (int k = 0; k < rowLength; k++)
      {
        data[i][k] = data[i][k + 1];
      }
      data[i][rowLength - 1] = t1;
    }
  }

  refreshLed();
}

void Lattice::downMoveBuff()
{
  for (int i = 0; i < columnLength; i++)
  {
    uint8_t t1 = data[i][(rowLength - 1)];
    uint8_t t2 = buff[i][(rowLength - 1)];
    for (int k = (rowLength - 1); k > 0; k--)
    {
      data[i][k] = data[i][k - 1];
      buff[i][k] = buff[i][k - 1];
    }
    data[i][0] = t2;
    buff[i][0] = t1;
  }
  refreshLed();
}

void Lattice::downMoveBuff(uint8_t *arr)
{
  for (int i = 0; i < columnLength; i++)
  {
    uint8_t t1 = data[i][7], t2 = buff[i][7], ai = arr[i];
    for (int k = 7; k > 0; k--)
    {
      data[i][k] = (data[i][k] & (ai ^ 0xff)) + (data[i][k - 1] & ai);
      buff[i][k] = (buff[i][k] & (ai ^ 0xff)) + (buff[i][k - 1] & ai);
    }
    data[i][0] = (data[i][0] & (ai ^ 0xff)) + (t2 & ai);
    buff[i][0] = (buff[i][0] & (ai ^ 0xff)) + (t1 & ai);
  }
  refreshLed();
}

void Lattice::showTime(uint8_t *arr)
{
  uint8_t mfs[4] = {0x0, 0x0, 0x0, 0x0};
  for (int k = 0; k < columnLength; k++)
  {
    // 得到新的要显示的数字
    unsigned char *tmp = getNumData(arr[k], false);
    for (int x = 0; x < 8; x++)
    {
      buff[k][x] = tmp[x];
      // 这里只能显示三位数字,所以最后一位清零
      buff[3][x] = 0x00;
    }
    free(tmp);
    if (k == 0 || (k == 1 && arr[0] == 0) ||
        (k == 2 && (arr[1] == 0 && arr[0] == 0)))
    {
      mfs[k] = (arr[k] % 10 == 0) ? 0xff : 0xf;
    }
  }

  // 这里为啥这么做,有苦难说,还是我太菜了
  for (int k = 2; k >= 0; k--)
  {
    uint8_t offset = k * 2 + 2;
    uint8_t lastoffset = (0xff >> offset) << offset;
    uint8_t nextoffset = (lastoffset ^ 0xff) << (8 - offset);
    mfs[k + 1] =
        (mfs[k + 1] & lastoffset) + ((mfs[k] & nextoffset) >> (8 - offset));
    mfs[k] <<= offset;
    offsetBuff(k, (-1 * offset));
  }

  if (isReset)
  {
    isReset = false;
    mfs[0] = 0xff;
    mfs[1] = 0xff;
    mfs[2] = 0xff;
    mfs[3] = 0xff;
  }

  // 显示点
  buff[2][2] = (buff[2][2] & ((0x80 >> 2) ^ 0xff)) + (0x80 >> 2);
  buff[2][4] = (buff[2][4] & ((0x80 >> 2) ^ 0xff)) + (0x80 >> 2);
  buff[1][2] = (buff[1][2] & ((0x80 >> 4) ^ 0xff)) + (0x80 >> 4);
  buff[1][4] = (buff[1][4] & ((0x80 >> 4) ^ 0xff)) + (0x80 >> 4);

  for (int i = 0; i < 8; i++)
  {
    downMoveBuff(mfs);
    delay(80);
  }
}

void Lattice::showTime2(uint8_t *arr)
{
  uint8_t mfs[4] = {0xff, 0xff, 0xff, 0x00};
  for (int k = 0; k < columnLength; k++)
  {
    // 得到新的要显示的数字
    unsigned char *tmp = getBigNumData(arr[k], 4);
    for (int x = 0; x < 8; x++)
    {
      buff[k][x] = tmp[x];
    }
    free(tmp);
  }

  offsetBuff(1, 3);
  offsetBuff(2, 2);
  offsetBuff(3, 5);

  if (arr[0] == 0)
  {
    mfs[0] = 0xef;
    mfs[1] = 0x1;
    if (arr[1] == 0)
    {
      mfs[1] = 0xc1;
      mfs[2] = 0x3;
      if (arr[2] == 0)
      {
        mfs[2] = 0x7b;
      }
    }
  }

  if (isReset)
  {
    isReset = false;
    mfs[3] = 0xff;
    // 显示icon
    for (int i = 0; i < 8; i++)
    {
      buff[3][i] = icons[0][i];
    }
  }

  // 显示点
  buff[1][2] = (buff[1][2] & ((0x80 >> 4) ^ 0xff)) + (0x80 >> 4);
  buff[1][3] = (buff[1][3] & ((0x80 >> 4) ^ 0xff)) + (0x80 >> 4);
  buff[1][5] = (buff[1][5] & ((0x80 >> 4) ^ 0xff)) + (0x80 >> 4);
  buff[1][6] = (buff[1][6] & ((0x80 >> 4) ^ 0xff)) + (0x80 >> 4);

  for (int i = 0; i < 8; i++)
  {
    downMoveBuff(mfs);
    delay(80);
  }
}

void Lattice::showTime3(uint8_t *arr)
{

  uint8_t mfs[4] = {0x0, 0x0, 0x0, 0x0};
  // 这里判断哪些数字要改变,哪些数字不要改变
  for (int k = 0; k < columnLength; k++)
  {
    if (k == 0 || (k == 1 && arr[0] == 0) ||
        (k == 2 && (arr[1] == 0 && arr[0] == 0)))
    {
      mfs[k] = (arr[k] % 10 == 0) ? 0xff : 0xf;
    }
  }

  if (isReset)
  {
    isReset = false;
    mfs[0] = 0xff;
    mfs[1] = 0xff;
    mfs[2] = 0xff;
    mfs[3] = 0xff;
  }
  // 这里保留不要改变的数字
  for (int i = 0; i < 8; i++)
  {
    // 这里为啥写三行不写循环,因为我觉得循环也是三行,这样也是三行就没必要计较了
    buff[0][i] = mfs[0] == 0x0 ? buff[0][i] : mfs[0] == 0xf ? (buff[0][i] & 0xf0)
                                                            : 0x0;
    buff[1][i] = mfs[1] == 0x0 ? buff[1][i] : mfs[1] == 0xf ? (buff[1][i] & 0xf0)
                                                            : 0x0;
    buff[2][i] = mfs[2] == 0x0 ? buff[2][i] : mfs[2] == 0xf ? (buff[2][i] & 0xf0)
                                                            : 0x0;
    buff[3][i] = 0x0;
  }

  for (int i = 0; i < 16; i++)
  {

    // 判断有几个数字发生了改变
    for (int j = 0; j < 3; j++)
    {
      if (mfs[j] == 0x0)
      {
        // 如果说没有改变则忽略
        break;
      }
      uint8_t gw = arr[j] % 10, sw = arr[j] / 10;
      uint8_t geweibuff = number_font_animations[gw][i], shiweibuff = number_font_animations[sw][i];
      if (geweibuff != 0x0)
      {
        buff[j][((geweibuff >> 4) & 0xf)] += ((0x80 >> (geweibuff & 0xf)) >> 4);
      }
      if (shiweibuff != 0x0 && mfs[j] == 0xff)
      {
        buff[j][((shiweibuff >> 4) & 0xf)] += ((0x80 >> (shiweibuff & 0xf)));
      }
    }

    for (int i = 0; i < columnLength; i++)
    {
      for (int j = 0; j < rowLength; j++)
      {
        data[i][j] = buff[i][j];
      }
    }

    offsetData(2, (-1 * 6));
    offsetData(1, (-1 * 4));
    offsetData(0, (-1 * 2));

    // 显示点
    data[2][2] = (data[2][2] & ((0x80 >> 2) ^ 0xff)) + (0x80 >> 2);
    data[2][4] = (data[2][4] & ((0x80 >> 2) ^ 0xff)) + (0x80 >> 2);
    data[1][2] = (data[1][2] & ((0x80 >> 4) ^ 0xff)) + (0x80 >> 4);
    data[1][4] = (data[1][4] & ((0x80 >> 4) ^ 0xff)) + (0x80 >> 4);
    refreshLed();
    delay(50);
  }
}

void Lattice::showCountDownTime(long remain, uint8_t *arr, bool showmode, bool minutechange)
{
  uint8_t mfs[4] = {0x0, 0x0, 0x0, 0x0};
  uint8_t d, h, m;
  if (showmode)
  { // 判断显示模式
    // 日 时 分 显示方式
    d = remain / 3600 / 24;
    h = (remain - d * 24 * 3600) / 3600;
    m = (remain - (d * 24 * 3600) - (h * 3600)) / 60;
  }
  else
  {
    // 时 分 秒 显示方式
    d = remain / 3600;
    h = (remain - (d * 3600)) / 60;
    m = (remain - (d * 3600) - h * 60);
  }

  if (isReset)
  {
    // 第一次进来
    isReset = false;
    unsigned char *tmp = getNumData(d / 10, false);
    for (int x = 0; x < 8; x++)
    {
      data[3][x] = tmp[x];
      data[0][x] = 0x0;
      data[1][x] = 0x0;
    }
    free(tmp);
    unsigned char *tmp2 = getNumData((d % 10) * 10, true);
    for (int x = 0; x < 8; x++)
    {
      data[2][x] = tmp2[x];
    }
    free(tmp2);
    upOrDownMove(3, 0xf0, true);
    upOrDownMove(2, 0xf0, false);
    mfs[0] = 0xff;
    mfs[1] = 0xff;
    minutechange = true;
  }
  else
  {
    // 倒计时天数动起来
    upOrDownMove(3, 0x0f, arr[1] > 3);
    upOrDownMove(3, 0xf0, arr[0] > 3);
    upOrDownMove(2, 0xf0, arr[2] > 3);
    // 判断哪些需要向下滚动
    mfs[0] = m % 10 != 9 ? 0x0f : 0xff;
    if (m == 59)
    {
      mfs[1] = (h % 10 == 9 || h % 10 == 3) ? 0xff : 0x0f;
    }
  }

  // 比较蠢的做法
  uint8_t lastoffset = (0xff >> 2) << 2;
  uint8_t nextoffset = (lastoffset ^ 0xff) << (8 - 2);
  mfs[2] = (mfs[2] & lastoffset) + ((mfs[1] & nextoffset) >> (8 - 2));
  mfs[1] <<= 2;
  // 显示小时数
  unsigned char *tmp3 = getNumData(h, false);
  for (int x = 0; x < 8; x++)
  {
    buff[1][x] = tmp3[x];
  }
  free(tmp3);
  offsetBuff(1, -2);
  // 显示分钟数
  unsigned char *tmp4 = getNumData(m, false);
  for (int x = 0; x < 8; x++)
  {
    buff[0][x] = tmp4[x];
  }
  free(tmp4);
  // 显示点
  data[1][3] = (data[1][3] & ((0x80 >> 6) ^ 0xff)) + (0x80 >> 6);

  if (remain >= 0 && minutechange)
  {
    // 倒计时还没结束就显示动画
    for (int i = 0; i < 8; i++)
    {
      downMoveBuff(mfs);
      delay(80);
    }
  }
  else
  {
    refreshLed();
  }
}

void Lattice::showLongNumber(uint8_t *arr)
{
  if (isReset)
  {
    isReset = false;
    for (int k = 0; k < columnLength; k++)
    {
      // 得到新的要显示的数字
      unsigned char *tmp = getNumData(arr[k], false);
      for (int x = 0; x < 8; x++)
      {
        buff[k][x] = tmp[x];
      }
      free(tmp);
    }
    for (int i = 0; i < rowLength; i++)
    {
      downMoveBuff();
      delay(80);
    }
  }
}

void Lattice::showNumAndIcon(uint8_t no, uint8_t *arr)
{
  uint8_t mfs[4] = {0xff, 0xff, 0xff, 0x00};
  if (isReset)
  {
    isReset = false;
    mfs[3] = 0xff;
    if (arr[2] == 0)
    {
      arr[2] = 100;
      if (arr[1] == 0)
      {
        arr[2] = 100;
        arr[1] = arr[0];
        arr[0] = 100;
      }
    }

    for (int k = 0; k < columnLength - 1; k++)
    {
      if (arr[k] > 99)
      {
        for (int x = 0; x < 8; x++)
        {
          buff[k][x] = 0x0;
        }
      }
      else
      {
        // 得到新的要显示的数字
        unsigned char *tmp = getNumData(arr[k], false);
        for (int x = 0; x < 8; x++)
        {
          buff[k][x] = tmp[x];
        }
        free(tmp);
      }
    }
    // 这里渲染图标
    for (int i = 0; i < rowLength; i++)
    {
      buff[3][i] = icons[2][i];
    }
    for (int i = 0; i < rowLength; i++)
    {
      downMoveBuff(mfs);
      delay(80);
    }
  }
}

void Lattice::showDate2(uint8_t *arr)
{
  uint8_t mfs[4] = {0xf, 0x00, 0x00, 0x00};
  for (int k = 0; k < columnLength; k++)
  {
    // 得到新的要显示的数字
    unsigned char *tmp = getBigNumData(arr[k], 4);
    for (int x = 0; x < 8; x++)
    {
      buff[k][x] = tmp[x];
    }
    free(tmp);
  }

  offsetBuff(1, 3);
  offsetBuff(2, 2);
  offsetBuff(3, 5);

  if (arr[0] == 0)
  {
    mfs[0] = 0xef;
    mfs[1] = 0x1;
    if (arr[1] == 0)
    {
      mfs[1] = 0xc1;
      mfs[2] = 0x3;
      if (arr[2] == 0)
      {
        mfs[2] = 0x7b;
      }
    }
  }

  if (isReset)
  {
    isReset = false;
    mfs[0] = 0xff;
    mfs[1] = 0xff;
    mfs[2] = 0xff;
    mfs[3] = 0xff;
  }

  // 这里渲染图标
  for (int i = 0; i < rowLength; i++)
  {
    buff[3][i] = icons[3][i];
  }

  // 显示点
  buff[1][4] = (buff[1][4] & ((0x80 >> 3) ^ 0xff)) + (0x80 >> 3);
  buff[1][4] = (buff[1][4] & ((0x80 >> 4) ^ 0xff)) + (0x80 >> 4);
  buff[1][4] = (buff[1][4] & ((0x80 >> 5) ^ 0xff)) + (0x80 >> 5);
  for (int i = 0; i < 8; i++)
  {
    downMoveBuff(mfs);
    delay(80);
  }
}

void Lattice::showDate3(uint8_t *arr)
{
  if (isReset)
  {
    isReset = false;

    for (int k = 0; k < columnLength; k++)
    {
      // 得到新的要显示的数字
      unsigned char *tmp = getNumData(arr[k], false);
      for (int x = 0; x < 8; x++)
      {
        buff[k][x] = tmp[x];
        // 这里只能显示三位数字,所以最后一位清零
        buff[3][x] = 0x00;
      }
      free(tmp);
    }

    offsetBuff(2, -7);
    offsetBuff(1, -4);
    offsetBuff(0, -1);

    // 显示点
    buff[2][3] = (buff[2][3] & ((0x80 >> 1) ^ 0xff)) + (0x80 >> 1);
    buff[2][3] = (buff[2][3] & ((0x80 >> 2) ^ 0xff)) + (0x80 >> 2);
    buff[1][3] = (buff[1][3] & ((0x80 >> 4) ^ 0xff)) + (0x80 >> 4);
    buff[1][3] = (buff[1][3] & ((0x80 >> 5) ^ 0xff)) + (0x80 >> 5);

    for (int i = 0; i < 8; i++)
    {
      downMoveBuff();
      delay(80);
    }
  }
}

void Lattice::showTemperature(uint8_t *arr)
{
  if (isReset)
  {
    isReset = false;
    for (int k = 0; k < columnLength; k++)
    {
      // 得到新的要显示的数字
      unsigned char *tmp = getNumData(arr[k], false);
      for (int x = 0; x < 8; x++)
      {
        buff[k][x] = tmp[x];
        // 这里只能显示三位数字,所以最后一位清零
        buff[3][x] = 0x00;
        buff[2][x] = 0x00;
      }
      free(tmp);
    }
    for (int k = 1; k >= 0; k--)
    {
      offsetBuff(k, (-1 * (k * 2 + 2)));
    }
    // 显示点
    buff[1][5] = (buff[1][5] & ((0x80 >> 4) ^ 0xff)) + (0x80 >> 4);
    // 在第三个点阵处显示温图案
    for (int i = 0; i < 8; i++)
    {
      buff[3][i] = icons[1][i];
    }
    offsetBuff(3, 2);

    for (int i = 0; i < 8; i++)
    {
      downMoveBuff();
      delay(80);
    }
  }
}

void Lattice::showUserData(uint8_t mode)
{
  if (mode == 0 || isReset)
  {
    if (isReset)
    {
      isReset = false;
      for (int i = 0; i < 4; i++)
      {
        for (int j = 0; j < 8; j++)
        {
          data[i][j] = latticeSetting.userData[i * 8 + j];
        }
      }
    }
    refreshLed();
    return;
  }
  else if (mode == 1)
  {
    rightMoveData(true);
  }
  else if (mode == 2)
  {
    rightMoveData(false);
  }
  else if (mode == 3)
  {
    downMoveData(true);
  }
  else if (mode == 4)
  {
    downMoveData(false);
  }
}

void Lattice::clearShow()
{
  for (int i = 0; i < columnLength; i++)
  {
    lc.clearDisplay(i);
  }
}

void Lattice::showFull()
{
  for (int i = 0; i < columnLength; i++)
  {
    for (int j = 0; j < rowLength; j++)
    {
      data[i][j] = 0xff;
    }
  }
  refreshLed();
}

void Lattice::lightning(uint8_t index)
{
  // 这里的做法待完善,这里为了快速实现bilibili眼睛闪烁才这样做的,其实也有更简单的做法,没时间搞
  data[index][4] = data[index][4] == 0x81 ? 0xa5 : 0x81;
  refreshLed();
}

void Lattice::showOtaUpdate(uint8_t num)
{
  Serial.println("update num");
  Serial.println(num);
  uint8_t mfs[4] = {0x00, 0x00, 0x00, 0x00};
  if (isReset)
  {
    isReset = false;
    // 如果是第一次进来就显示默认数据
    for (int i = 0; i < columnLength; i++)
    {
      for (int j = 0; j < rowLength; j++)
      {
        buff[i][j] = otaicon[i][j];
      }
    }
    for (int i = 0; i < 8; i++)
    {
      downMoveBuff();
      delay(50);
    }
  }
  else
  {
    mfs[1] = 0xff;
    // 得到新的要显示的数字
    unsigned char *tmp = getNumData(num, false);
    for (int i = 0; i < rowLength; i++)
    {
      data[1][i] = tmp[i];
    }
    free(tmp);
    refreshLed();
  }
}

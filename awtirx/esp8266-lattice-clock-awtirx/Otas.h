#ifndef OTAS_H
#define OTAS_H
#include "LatticePlus.h"
#include "PilotLight.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>

class Otas
{
private:
  /**
   * @brief 显示内容对象
   *
   */
  LatticePlus *lattice;
  /**
   * @brief 状态指示灯对象
   *
   */
  PilotLight *pilotLight;

  /**
   * @brief 搞个单例
   *
   */
  static Otas *s_otas;

  /**
   * @brief 因为回调方法都是静态方法,所以这里就只能搞一个折中的了
   *
   */
  void flashing();

  /**
   * @brief 因为回调方法都是静态方法,所以这里就只能搞一个折中的了
   *
   */
  void showLongIcon(int type);

  /**
   * @brief 因为回调方法都是静态方法,所以这里就只能搞一个折中的了
   *
   */
  void reset();

  /**
   * @brief 因为回调方法都是静态方法,所以这里就只能搞一个折中的了
   *
   */
  void showOtaUpdate(int process);

public:
  Otas();

  Otas(LatticePlus *latticeobj, PilotLight *pilotLightobj);

  /**
   * @brief
   *
   * @param version
   */
  static void updateOta(int version);

protected:
  /**
   * @brief 更新开始回调函数
   *
   */
  static void update_started();

  /**
   * @brief 更新完成回调函数
   *
   */
  static void update_finished();
  /**
   * @brief 更新中回调函数
   *
   */
  static void update_progress(long cur, long total);

  /**
   * @brief 更新错误回调函数
   *
   */
  static void update_error(int err);
};

#endif

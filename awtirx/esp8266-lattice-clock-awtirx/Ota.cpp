#include "Otas.h"
Otas *Otas::s_otas;

Otas::Otas() { s_otas = this; }

Otas::Otas(Lattice *latticeobj, PilotLight *pilotLightobj)
{
    lattice = latticeobj;
    pilotLight = pilotLightobj;
    s_otas = this;
}

void Otas::flashing()
{
    pilotLight->flashing(20);
}

void Otas::showLongIcon(int type)
{
    lattice->showLongIcon(type);
}

void Otas::showOtaUpdate(int process)
{
    lattice->showOtaUpdate(process);
}

void Otas::reset()
{
    lattice->reset();
}

void Otas::update_started()
{
    Serial.println("callback: ota is start!");  // 打印日志
    s_otas->reset();                            // 重置显示内容
    Serial.println("callback: ota is start1!"); // 打印日志
}

void Otas::update_finished()
{
    Serial.println("callback:  ota is finish!"); // 打印日志
    s_otas->reset();                             // 重置显示内容
    s_otas->showLongIcon(3);                     // 显示OTA更新成功图案
}

void Otas::update_progress(long cur, long total)
{
    int process = (int)(((cur * 0.1) / (total * 0.1)) * 100);              // 计算更新进度
    Serial.printf("callback:  updateing %d of %d bytes...\n", cur, total); // 打印日志
    s_otas->flashing();                                                    // 固件升级的时候LED闪
    s_otas->showOtaUpdate(process);                                        // OTA显示当前进度图案
}

void Otas::update_error(int err)
{
    Serial.printf("callback:  ota is error:  %d\n", err); // 打印日志
    s_otas->reset();                                      // 重置显示内容
    s_otas->showLongIcon(4);                              // 显示OTA更新失败图案
}

void Otas::updateOta(int version)
{
    WiFiClient client;
    if (WiFi.status() == WL_CONNECTED) // 确保有网络
    {
        ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
        ESPhttpUpdate.onStart(update_started);                                                                    // 添加可选的回调通知程序
        ESPhttpUpdate.onEnd(update_finished);                                                                     // 添加更新完成回调方法
        ESPhttpUpdate.onProgress(update_progress);                                                                // 添加更新中回调方法
        ESPhttpUpdate.onError(update_error);                                                                      // 添加更新失败回调方法
        char *updateUrl = new char[50];                                                                           // 临时放url
        sprintf(updateUrl, "%s%d%s%d", "http://oss.lengff.com/iot/lattice/", version, ".bin?t=", millis() % 100); // 后面的对100取余就是为了解除文件CDN缓存
        Serial.println(updateUrl);
        t_httpUpdate_return ret = ESPhttpUpdate.update(client, updateUrl);
        free(updateUrl);
        switch (ret)
        {
        case HTTP_UPDATE_FAILED:
            Serial.printf("ote is fail, err code is：(%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str()); // 打印日志
            s_otas->reset();                                                                                                                 // 重置显示内容
            s_otas->showLongIcon(4);                                                                                                         // 显示OTA更新失败图案
            break;
        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("not ota update!"); // 打印日志
            s_otas->showLongIcon(3);           // 显示OTA更新成功图案
            break;
        case HTTP_UPDATE_OK:
            s_otas->reset();         // 打印日志
            s_otas->showLongIcon(3); // 显示OTA更新成功图案
            break;
        }
    }
}

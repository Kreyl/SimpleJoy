/*
 * usb_cdc.h
 *
 *  Created on: 03 сент. 2015 г.
 *      Author: Kreyl
 */

#pragma once

#include "hal.h"
#include "hal_serial_usb.h"
#include "stdarg.h"
#include "shell.h"

class UsbCDC_t : public PrintfHelper_t, public Shell_t {
private:
    uint8_t IPutChar(char c) { SDU1.vmt->put(&SDU1, c); return retvOk; }
    void IStartTransmissionIfNotYet() {}
    void SignalCmdProcessed() {}
public:
    void Init();
    void Connect();
    void Disconnect();
    bool IsActive() { return (SDU1.config->usbp->state == USB_ACTIVE); }
    void Printf(const char *S, ...);
    // Inner use
    SerialUSBDriver SDU1;
    bool CmdProcessInProgress;
};

extern UsbCDC_t UsbCDC;

#pragma once

#include "AdVdBluetooth.h"

#define ERROR_PARAM const BluetoothException*& error

extern "C" BTAPI void DisposeBluetoothException(BluetoothException* error);


extern "C" BTAPI BluetoothDevice* CreateBluetoothDevice(ERROR_PARAM);

extern "C" BTAPI void DisposeBluetoothDevice(BluetoothDevice* device);
extern "C" BTAPI void DisposeBluetoothDeviceArray(BluetoothDevice* devices);

//extern "C" BTAPI const wchar_t* BluetoothDevice_AddressAsString(const BluetoothDevice* device);//TODO const wchar_t*

extern "C" BTAPI bool BluetoothDevice_FindDeviceByAddress_WS(const wchar_t* address, BluetoothDevice& device, bool flushCache, ERROR_PARAM);
extern "C" BTAPI bool BluetoothDevice_FindDeviceByAddress_U64(uint64_t address, BluetoothDevice& device, bool flushCache, ERROR_PARAM);
extern "C" BTAPI bool BluetoothDevice_FindDeviceByName(const wchar_t* name, BluetoothDevice& device, bool flushCache, ERROR_PARAM);
extern "C" BTAPI int BluetoothDevice_GetAvailableDevices(BluetoothDevice*& devices, bool flushCache, ERROR_PARAM);



extern "C" BTAPI BluetoothService* CreateBluetoothService(ERROR_PARAM);

extern "C" BTAPI void DisposeBluetoothService(BluetoothService* service);
extern "C" BTAPI void DisposeBluetoothServiceArray(BluetoothService* services);

extern "C" BTAPI int BluetoothService_QueryServices(const BluetoothDevice& device, const ServiceClass& serviceClass, BluetoothService*& services, bool flushCache, ERROR_PARAM);
extern "C" BTAPI void BluetoothService_RegisterService(const BluetoothSocket* localSocket, const wchar_t* name, const wchar_t* comment, const ServiceClass& serviceClass, BluetoothService& service, ERROR_PARAM);
extern "C" BTAPI void BluetoothService_DeleteService(const BluetoothService& service, ERROR_PARAM);



extern "C" BTAPI BluetoothSocket* CreateBluetoothSocket(ERROR_PARAM);

extern "C" BTAPI void DisposeBluetoothSocket(BluetoothSocket* s);


extern "C" BTAPI void BluetoothSocket_Connect_S(const BluetoothSocket* s, const BluetoothService& service, ERROR_PARAM);
extern "C" BTAPI void BluetoothSocket_Connect_D_SC(const BluetoothSocket* s, const BluetoothDevice& remote, const ServiceClass& serviceClass, ERROR_PARAM);
extern "C" BTAPI void BluetoothSocket_Connect_U64_SC(const BluetoothSocket* s, uint64_t remoteAddress, const ServiceClass& serviceClass, ERROR_PARAM);
extern "C" BTAPI void BluetoothSocket_Bind(const BluetoothSocket* s, ERROR_PARAM);
extern "C" BTAPI void BluetoothSocket_Listen(const BluetoothSocket* s, ERROR_PARAM);

extern "C" BTAPI void BluetoothSocket_Select(const BluetoothSocket** checkRead, const BluetoothSocket** checkWrite, const BluetoothSocket** checkError, uint32_t microseconds, ERROR_PARAM);
extern "C" BTAPI bool BluetoothSocket_Poll(const BluetoothSocket* s, uint32_t microseconds, int selectMode, ERROR_PARAM);

extern "C" BTAPI BluetoothSocket* BluetoothSocket_Accept(const BluetoothSocket* s, ERROR_PARAM);
extern "C" BTAPI void BluetoothSocket_Send(const BluetoothSocket* s, const char* buffer, int length, ERROR_PARAM);
extern "C" BTAPI int BluetoothSocket_Receive(const BluetoothSocket* s, char* buffer, int length, ERROR_PARAM);
extern "C" BTAPI bool BluetoothSocket_IsValid(const BluetoothSocket* s, ERROR_PARAM);
extern "C" BTAPI void BluetoothSocket_GetAddressAndPort(const BluetoothSocket* s, uint64_t& address, unsigned long& port, ERROR_PARAM);
extern "C" BTAPI void BluetoothSocket_Close(BluetoothSocket* s, ERROR_PARAM);

#undef ERROR_PARAM

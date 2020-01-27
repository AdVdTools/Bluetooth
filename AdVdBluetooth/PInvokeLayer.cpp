#include "PInvokeLayer.h"

#define ERROR_PARAM const BluetoothException*& error

#define NULL_CHECK_RET(obj, errorRet) if (obj == nullptr) { error = new BluetoothException(L#obj L" was null"); return errorRet; }
#define NULL_CHECK(obj) NULL_CHECK_RET(obj, )

#define TRY_CATCH_RET(call, errorRet) try { return call; } catch(const BluetoothException& bex) { error = new BluetoothException(bex); return errorRet; }
#define TRY_CATCH(call) try { call; } catch(const BluetoothException& bex) { error = new BluetoothException(bex); }

void DisposeBluetoothException(BluetoothException* error)
{
	if (error != nullptr) {
		delete error;
		error = nullptr;
	}
}

#pragma region BluetoothDevice

BluetoothDevice* CreateBluetoothDevice(ERROR_PARAM)
{
	TRY_CATCH_RET(new BluetoothDevice(), nullptr);
}

void DisposeBluetoothDevice(BluetoothDevice* device)
{
	if (device != nullptr) {
		delete device;
		device = nullptr;
	}
}

void DisposeBluetoothDeviceArray(BluetoothDevice* devices)
{
	if (devices != nullptr) {
		delete[] devices;
		devices = nullptr;
	}
}

bool BluetoothDevice_FindDeviceByAddress_WS(const wchar_t* address, BluetoothDevice& device, bool flushCache, ERROR_PARAM)
{
	TRY_CATCH_RET(BluetoothDevice::FindDeviceByAddress(address, device, flushCache), false);
}
bool BluetoothDevice_FindDeviceByAddress_U64(uint64_t address, BluetoothDevice& device, bool flushCache, ERROR_PARAM)
{
	TRY_CATCH_RET(BluetoothDevice::FindDeviceByAddress(address, device, flushCache), false);
}
bool BluetoothDevice_FindDeviceByName(const wchar_t* name, BluetoothDevice& device, bool flushCache, ERROR_PARAM)
{
	TRY_CATCH_RET(BluetoothDevice::FindDeviceByName(name, device, flushCache), false);
}

int BluetoothDevice_GetAvailableDevices(BluetoothDevice*& devices, bool flushCache, ERROR_PARAM)
{
	TRY_CATCH_RET(BluetoothDevice::GetAvailableDevices(devices, flushCache), -1);
}

#pragma endregion



#pragma region BluetoothService

BluetoothService* CreateBluetoothService(ERROR_PARAM)
{
	TRY_CATCH_RET(new BluetoothService(), nullptr);
}

void DisposeBluetoothService(BluetoothService* service)
{
	if (service != nullptr) {
		delete service;
		service = nullptr;
	}
}

void DisposeBluetoothServiceArray(BluetoothService* services)
{
	if (services != nullptr) {
		delete[] services;
		services = nullptr;
	}
}

int BluetoothService_QueryServices(const BluetoothDevice& device, const ServiceClass& serviceClass, BluetoothService*& services, bool flushCache, ERROR_PARAM)
{
	TRY_CATCH_RET(BluetoothService::QueryServices(device, serviceClass, services, flushCache), -1);
}

void BluetoothService_RegisterService(const BluetoothSocket* localSocket, const wchar_t* name, const wchar_t* comment, const ServiceClass& serviceClass, BluetoothService& service, ERROR_PARAM)
{
	TRY_CATCH(BluetoothService::RegisterService(localSocket, name, comment, serviceClass, service));
}

void BluetoothService_DeleteService(const BluetoothService& service, ERROR_PARAM)
{
	TRY_CATCH(BluetoothService::DeleteService(service));
}

#pragma endregion



#pragma region BluetoothSocket

BluetoothSocket* CreateBluetoothSocket(ERROR_PARAM) 
{
	TRY_CATCH_RET(new BluetoothSocket(), nullptr);
}

void DisposeBluetoothSocket(BluetoothSocket* s)
{
	if (s != nullptr) {
		delete s;
		s = nullptr;
	}
}


void BluetoothSocket_Connect_S(const BluetoothSocket* s, const BluetoothService& service, ERROR_PARAM)
{
	NULL_CHECK(s);
	TRY_CATCH(s->Connect(service));
}
void BluetoothSocket_Connect_D_SC(const BluetoothSocket* s, const BluetoothDevice& remote, const ServiceClass& serviceClass, ERROR_PARAM)
{
	NULL_CHECK(s);
	TRY_CATCH(s->Connect(remote, serviceClass));
}
void BluetoothSocket_Connect_U64_SC(const BluetoothSocket* s, uint64_t remoteAddress, const ServiceClass& serviceClass, ERROR_PARAM)
{
	NULL_CHECK(s);
	TRY_CATCH(s->Connect(remoteAddress, serviceClass));
}

void BluetoothSocket_Bind(const BluetoothSocket* s, ERROR_PARAM)
{
	NULL_CHECK(s);
	TRY_CATCH(s->Bind());
}

void BluetoothSocket_Listen(const BluetoothSocket* s, ERROR_PARAM)
{
	NULL_CHECK(s);
	TRY_CATCH(s->Listen());
}


void BluetoothSocket_Select(const BluetoothSocket** checkRead, const BluetoothSocket** checkWrite, const BluetoothSocket** checkError, uint32_t microseconds, ERROR_PARAM)
{
	TRY_CATCH(BluetoothSocket::Select(checkRead, checkWrite, checkError, microseconds));
}

bool BluetoothSocket_Poll(const BluetoothSocket* s, uint32_t microseconds, int selectMode, ERROR_PARAM)
{
	NULL_CHECK_RET(s, false);
	TRY_CATCH_RET(s->Poll(microseconds, selectMode), false);
}


BluetoothSocket* BluetoothSocket_Accept(const BluetoothSocket* s, ERROR_PARAM)
{
	NULL_CHECK_RET(s, nullptr);
	TRY_CATCH_RET(s->Accept(), nullptr);
}

void BluetoothSocket_Send(const BluetoothSocket* s, const char* buffer, uint32_t offset, uint32_t length, ERROR_PARAM)
{
	NULL_CHECK(s);
	TRY_CATCH(s->Send(buffer, offset, length));
}

int BluetoothSocket_Receive(const BluetoothSocket* s, char* buffer, uint32_t offset, uint32_t length, ERROR_PARAM)
{
	NULL_CHECK_RET(s, -1);
	TRY_CATCH_RET(s->Receive(buffer, offset, length), -1);
}

bool BluetoothSocket_IsValid(const BluetoothSocket* s, ERROR_PARAM)
{
	NULL_CHECK_RET(s, false);
	TRY_CATCH_RET(s->IsValid(), false);
}

void BluetoothSocket_GetAddressAndPort(const BluetoothSocket* s, uint64_t& address, unsigned long& port, ERROR_PARAM) 
{
	NULL_CHECK(s);
	TRY_CATCH(s->GetAddressAndPort(address, port));
}

void BluetoothSocket_Close(BluetoothSocket* s, ERROR_PARAM)
{
	NULL_CHECK(s);
	TRY_CATCH(s->Close());
}

#pragma endregion



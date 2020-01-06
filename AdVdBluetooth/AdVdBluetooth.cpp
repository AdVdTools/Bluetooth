// AdVdBluetooth.cpp : Defines the exported functions for the DLL application.
//

#include "AdVdBluetooth.h"

#include <strsafe.h>

#include <winsock2.h>
#include <ws2bth.h>


#define BT_ADDR_STR_LEN 17// 6 two-digit hex values plus 5 colons

#define LISTEN_BACKLOG 4

const unsigned int BluetoothMaxNameLength = BTH_MAX_NAME_SIZE;

static WSADATA WSAData = { 0 };//TODO methods to extract info? 

bool InitializeBluetooth()
{
	//
	// Ask for Winsock version 2.2.
	//
	int result = WSAStartup(MAKEWORD(2, 2), &WSAData);
	if (result != NO_ERROR) {
		wprintf(L"Failed to initialize Winsock version 2.2 [ErrorCode: %d]\n", WSAGetLastError());
		return false;
	}
	return true;
}

bool CleanupBluetooth()
{
	int result = WSACleanup();
	if (result != NO_ERROR) {
		wprintf(L"Failed to cleanup Winsock version 2.2 [ErrorCode: %d]\n", WSAGetLastError());
		return false;
	}
	return true;
}


#pragma region ServiceClass

//unsigned char CharToHex(char c) 
//{ 
//	if (c >= '0' && c <= '9') return (c - '0');  
//	if (c >= 'A' && c <= 'F') return (c - 'A' + 10);  
//	if (c >= 'a' && c <= 'f') return (c - 'a' + 10);  
//	throw "Invalid Format";
//}


ServiceClass::ServiceClass() : id()
{

}

const char* const HexToChar = "0123456789abcdef";
ServiceClass::ServiceClass(const wchar_t* uuid)
//bool ServiceClass::FromString(const char* uuid)
{
#define CharToHex(hex, c) unsigned char hex; if (c >= '0' && c <= '9') hex = (c - '0'); \
	else if (c >= 'A' && c <= 'F') hex = (c - 'A' + 10); \
	else if (c >= 'a' && c <= 'f') hex = (c - 'a' + 10); \
	else throw BluetoothException(L"Invalid Format");
#define CharsToByte(dst, first, second) { CharToHex(hc, first); CharToHex(lc, second); dst = (hc << 4) | lc; }
//#define CharsToByte(dst, first, second) { unsigned char hc = CharToHex(first), lc = CharToHex(second); dst = (hc << 4) | lc; }

	CharsToByte(id[3], uuid[0], uuid[1]);
	CharsToByte(id[2], uuid[2], uuid[3]);
	CharsToByte(id[1], uuid[4], uuid[5]);
	CharsToByte(id[0], uuid[6], uuid[7]);
	if (uuid[8] != '-') throw BluetoothException(L"Invalid Format");
	CharsToByte(id[5], uuid[9], uuid[10]);
	CharsToByte(id[4], uuid[11], uuid[12]);
	if (uuid[13] != '-') throw BluetoothException(L"Invalid Format");
	CharsToByte(id[7], uuid[14], uuid[15]);
	CharsToByte(id[6], uuid[16], uuid[17]);
	if (uuid[18] != '-') throw BluetoothException(L"Invalid Format");
	CharsToByte(id[8], uuid[19], uuid[20]);
	CharsToByte(id[9], uuid[21], uuid[22]);
	if (uuid[23] != '-') throw BluetoothException(L"Invalid Format");
	CharsToByte(id[10], uuid[24], uuid[25]);
	CharsToByte(id[11], uuid[26], uuid[27]);
	CharsToByte(id[12], uuid[28], uuid[29]);
	CharsToByte(id[13], uuid[30], uuid[31]);
	CharsToByte(id[14], uuid[32], uuid[33]);
	CharsToByte(id[15], uuid[34], uuid[35]);
	if (uuid[36] != '\0') throw BluetoothException(L"Invalid Format");

#undef CharsToByte
#undef CharToHex
}

const wchar_t* ServiceClass::MakeString() const {
#define ByteToChars(src, first, second) first = HexToChar[src >> 4]; second = HexToChar[src & 0x0f]
	
	wchar_t* uuid = new wchar_t[37];//TODO custom string class that frees itself? return with copy ellision?

	ByteToChars(id[3], uuid[0], uuid[1]);
	ByteToChars(id[2], uuid[2], uuid[3]);
	ByteToChars(id[1], uuid[4], uuid[5]);
	ByteToChars(id[0], uuid[6], uuid[7]);
	uuid[8] = '-';
	ByteToChars(id[5], uuid[9], uuid[10]);
	ByteToChars(id[4], uuid[11], uuid[12]);
	uuid[13] = '-';
	ByteToChars(id[7], uuid[14], uuid[15]);
	ByteToChars(id[6], uuid[16], uuid[17]);
	uuid[18] = '-';
	ByteToChars(id[8], uuid[19], uuid[20]);
	ByteToChars(id[9], uuid[21], uuid[22]);
	uuid[23] = '-';
	ByteToChars(id[10], uuid[24], uuid[25]);
	ByteToChars(id[11], uuid[26], uuid[27]);
	ByteToChars(id[12], uuid[28], uuid[29]);
	ByteToChars(id[13], uuid[30], uuid[31]);
	ByteToChars(id[14], uuid[32], uuid[33]);
	ByteToChars(id[15], uuid[34], uuid[35]);
	uuid[36] = '\0';
	return uuid;

#undef ByteToChars
}

//TODO rename
//const UUID SPPBaseUUID = { 0x00001101, 0x0000, 0x1000, { 0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb } };
// First three groups have bytes reversed
//const ServiceClass SPPBaseUUID = { 0x01, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb };
//const ServiceClass SPPBaseUUID = ServiceClass("00001101-0000-1000-8000-00805f9b34fb");
const ServiceClass SPPBaseUUID = ServiceClass(L"00001101-0000-1000-8000-00805F9B34FB");//TODO TEST

#pragma endregion


#pragma region Bluetooth Utils

#define ServiceClassToGUID(serviceClass) *((GUID*)(&serviceClass))
#define GUIDToServiceClass(guid) *((ServiceClass*)(&guid))

// WSA methods already return an error if WSAStartup has not been called
class BluetoothLookup
{
public:
	BluetoothLookup() {
		pWSAQuerySet = (WSAQUERYSET*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, querySetSize);//TODO heap alloc needed?
		hLookup = nullptr;

		status = NO_ERROR;
		reallocatedMemory = false;

		if (pWSAQuerySet == nullptr) {
			status = STATUS_NO_MEMORY;
			throw BluetoothException(L"Unable to allocate memory for WSAQUERYSET");
		}
	}

	int GetStatus() const {
		return status;
	}

#define CheckValid() if (status != NO_ERROR) throw BluetoothException(L"Lookup object is in an invalid state", status)
	
	

	void SetupForDeviceQuery(bool flush = false)
	{
		CheckValid();

		// WSALookupService is used for both service search and device inquiry
		flags = LUP_CONTAINERS /*signals device inquiry*/
			| LUP_RETURN_NAME /* Return friendly device name (if available) in lpszServiceInstanceName) */
			| LUP_RETURN_ADDR /* Return BTH_ADDR in lpcsaBuffer */;
		
		if (flush) flags |= LUP_FLUSHCACHE; // Flush the device cache in future inquiries

		ZeroMemory(pWSAQuerySet, querySetSize);
		pWSAQuerySet->dwNameSpace = NS_BTH;
		pWSAQuerySet->dwSize = sizeof(WSAQUERYSET);
	}

	void SetupForServiceQuery(uint64_t address, const ServiceClass& serviceClass, bool flush = false)
	{
		CheckValid();

		serviceClassId = ServiceClassToGUID(serviceClass);

		// Service queries fail with WSASERVICE_NOT_FOUND if remote offline.
		// Let the user catch and identify the error, if they wish to tell 
		// the difference between 0 services (no error) and remote offline.

		int addrLen = sizeof(SOCKADDR_BTH);
		unsigned long addrStrLength = BT_ADDR_STR_LEN + 3;

		SOCKADDR_BTH addr = { 0 };
		addr.addressFamily = AF_BTH;
		addr.btAddr = address;
		status = WSAAddressToString((SOCKADDR*)&addr, addrLen, nullptr, contextAddress, &addrStrLength);
		
		if (status != NO_ERROR) {
			throw BluetoothException(L"WSAAddressToString() call failed", WSAGetLastError());
		}

		// WSALookupService is used for both service search and device inquiry
		flags = LUP_RETURN_NAME /* Return friendly device name (if available) in lpszServiceInstanceName) */
			| LUP_RETURN_COMMENT /* Return comment */
			| LUP_RETURN_TYPE /* Useless since it returns 0s on Next */ //TODO remove?
			| LUP_RETURN_ADDR /* Return BTH_ADDR in lpcsaBuffer */;
		
		if (flush) flags |= LUP_FLUSHCACHE; // Flush the device cache in future inquiries

		ZeroMemory(pWSAQuerySet, querySetSize);
		pWSAQuerySet->dwNameSpace = NS_BTH;
		pWSAQuerySet->dwSize = sizeof(WSAQUERYSET);
		pWSAQuerySet->lpServiceClassId = &serviceClassId;
		pWSAQuerySet->dwNumberOfCsAddrs = 0;
		pWSAQuerySet->lpszContext = contextAddress;
	}

	void BeginLookup() {
		CheckValid();

		hLookup = nullptr;

		status = WSALookupServiceBegin(pWSAQuerySet, flags, &hLookup);
		reallocatedMemory = false;

		if (status != NO_ERROR) {
			throw BluetoothException(L"WSALookupServiceBegin failed", WSAGetLastError());
		}

		if (hLookup == nullptr) {
			throw BluetoothException(L"Lookup handle was null");
		}
	}

	bool Next() {
		CheckValid();
		
		if (hLookup == nullptr) {
			throw BluetoothException(L"Lookup handle was null");
		}

		status = WSALookupServiceNext(hLookup, flags, &querySetSize, pWSAQuerySet);

		if (status == NO_ERROR) {
			reallocatedMemory = false;
			return true;
		}

		int error = WSAGetLastError();
		if (error == WSA_E_NO_MORE) { //No more data
			status = NO_ERROR;
			reallocatedMemory = false;
			return false;
		}
		else if (error == WSAEFAULT && !reallocatedMemory) { // Don't reallocate more than once in a row
			// The buffer for QUERYSET was insufficient.
			// In such case 3rd parameter "ulPQSSize" of function "WSALookupServiceNext()" receives
			// the required size.  So we can use this parameter to reallocate memory for QUERYSET.
			HeapFree(GetProcessHeap(), 0, pWSAQuerySet);
			pWSAQuerySet = (PWSAQUERYSET)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, querySetSize);
			if (pWSAQuerySet == nullptr) {
				status = STATUS_NO_MEMORY;
				throw BluetoothException(L"Unable to allocate memory for WSAQERYSET");
			}
			else {
				status = NO_ERROR;
				reallocatedMemory = true;
				return Next();
			}
		}
		else {
			throw BluetoothException(L"WSALookupServiceNext failed", error);
		}
	}

	const wchar_t* GetName() {
		CheckValid();
		return pWSAQuerySet->lpszServiceInstanceName;
	}

	const wchar_t* GetComment() {
		CheckValid();
		return pWSAQuerySet->lpszComment;
	}

	const void* GetAddr() {
		CheckValid();
		return pWSAQuerySet->lpcsaBuffer->RemoteAddr.lpSockaddr;
	}

#undef CheckValid

	void EndLookup() {
		if (hLookup != nullptr) {
			status = WSALookupServiceEnd(hLookup);
			hLookup = nullptr;

			// WSALookupServiceEnd should only return an error on:
			//    WSA_INVALID_HANDLE, WSANOTINITIALISED and	WSA_NOT_ENOUGH_MEMORY
			// EndLookup might (and should) be called in a catch block after an exception
			// is thrown on the lookup object. The following exception might mask it, but
			// it's still relevant and probably matches the previously thrown exception.
			// Otherwise this exception would not be thrown, since the status is cleaned.
			if (status != NO_ERROR) {
				throw BluetoothException(L"WSALookupServiceEnd failed", WSAGetLastError());
			}
		}
	}

	~BluetoothLookup() {
		if (pWSAQuerySet != nullptr) {
			HeapFree(GetProcessHeap(), 0, pWSAQuerySet);
			pWSAQuerySet = nullptr;
		}
	}

private:
	WSAQUERYSET* pWSAQuerySet = nullptr;
	unsigned long querySetSize = sizeof(WSAQUERYSET);
	HANDLE hLookup = nullptr;
	GUID serviceClassId;
	wchar_t contextAddress[BT_ADDR_STR_LEN + 3];// 3 extra for trailing NULL character a 2 optional parenthesis
	unsigned long flags;

	int status;
	bool reallocatedMemory;
};

#define ReplaceWithClone(dst, src) if (dst != nullptr) delete[] dst; dst = CloneString(src)

wchar_t* CloneString(const wchar_t* src)
{
	if (src == nullptr) return nullptr;

	size_t length;
	HRESULT res = StringCchLength(src, 1024, &length);
	if (FAILED(res)) return nullptr;
	wchar_t* dst = (wchar_t*)new wchar_t[length + 1];
	res = StringCchCopy(dst, length + 1, src);
	if (FAILED(res)) {
		delete[] dst;
		return nullptr;
	}
	return dst;
}


const wchar_t* BluetoothAddressToString(uint64_t address)
{
	wchar_t addrBuffer[BT_ADDR_STR_LEN + 3] = { 0 }; // 3 extra for trailing NULL character a 2 optional parenthesis

	int addrLen = sizeof(SOCKADDR_BTH);
	unsigned long addrStrLength = BT_ADDR_STR_LEN + 3;

	SOCKADDR_BTH addr = { 0 };
	addr.addressFamily = AF_BTH;
	addr.btAddr = address;
	int result = WSAAddressToString((SOCKADDR*)&addr, addrLen, nullptr, addrBuffer, &addrStrLength);

	if (result != NO_ERROR) {
		throw BluetoothException(L"WSAAddressToString() call failed", WSAGetLastError());
	}

	const wchar_t* addressStr = CloneString(addrBuffer);
	return addressStr;
}


uint64_t BluetoothAddressFromString(const wchar_t* address)
{
	wchar_t addressStr[BT_ADDR_STR_LEN + 3] = { 0 }; // 3 extra for trailing NULL character a 2 optional parenthesis

	StringCchCopy(addressStr, BT_ADDR_STR_LEN + 3, address);

	SOCKADDR_BTH addr = { 0 };

	int addrLen = sizeof(SOCKADDR_BTH);
	int result = WSAStringToAddress(addressStr, AF_BTH, nullptr, (SOCKADDR*)&addr, &addrLen);

	if (result != NO_ERROR) {
		throw BluetoothException(L"WSAStringToAddress() call failed", WSAGetLastError());
	}

	return addr.btAddr;
}

#pragma endregion


#pragma region BluetoothDevice

BluetoothDevice::BluetoothDevice()
{
	name = nullptr;
	address = BTH_ADDR_NULL;// 0UL;
}

BluetoothDevice::~BluetoothDevice()
{
	if (name != nullptr) delete[] name;
}

BluetoothDevice& BluetoothDevice::operator=(const BluetoothDevice& other)
{
	ReplaceWithClone(name, other.name);
	address = other.address;

	return *this;
}

BluetoothDevice::BluetoothDevice(const BluetoothDevice& other) : BluetoothDevice()
{
	name = CloneString(other.name);
	address = other.address;
}


//TODO connect method that uses address as string without the need of a device
bool BluetoothDevice::FindDeviceByAddress(const wchar_t* address, BluetoothDevice& device, bool flushCache) // TODO TEST
{
	wchar_t deviceAddr[BT_ADDR_STR_LEN + 3] = { 0 }; // 3 extra for trailing NULL character a 2 optional parenthesis

	StringCchCopy(deviceAddr, BT_ADDR_STR_LEN + 3, address);

	SOCKADDR_BTH addr = { 0 };

	int addrLen = sizeof(SOCKADDR_BTH);
	int result = WSAStringToAddress(deviceAddr, AF_BTH, nullptr, (SOCKADDR*)&addr, &addrLen);//TODO use utility method BluetoothAddressFromString instead?

	if (result != NO_ERROR) {
		throw BluetoothException(L"WSAStringToAddress() call failed", WSAGetLastError());
	}

	return FindDeviceByAddress(addr.btAddr, device, flushCache);
}

bool BluetoothDevice::FindDeviceByAddress(uint64_t address, BluetoothDevice& device, bool flushCache)
{
	bool found = false;

	BluetoothLookup lookup;
	try {
		lookup.SetupForDeviceQuery(flushCache);
		lookup.BeginLookup();
		while (lookup.Next())
		{
			SOCKADDR_BTH* addrData = (SOCKADDR_BTH*)lookup.GetAddr();
			if (addrData != nullptr && addrData->addressFamily == AF_BTH && addrData->btAddr == address) {
				ReplaceWithClone(device.name, lookup.GetName());
				device.address = address;
				found = true;
				break;
			}
		}
		lookup.EndLookup();
	}
	catch (const BluetoothException& bex)
	{
		lookup.EndLookup();
		throw bex;
	}

	return found;
}

bool BluetoothDevice::FindDeviceByName(const wchar_t* name, BluetoothDevice& device, bool flushCache)
{
	bool found = false;

	BluetoothLookup lookup;
	try {
		lookup.SetupForDeviceQuery(flushCache);
		lookup.BeginLookup();
		while (lookup.Next())
		{
			const wchar_t* remoteName = lookup.GetName();
			if (remoteName != nullptr && CompareString(LOCALE_INVARIANT, 0, remoteName, -1, name, -1) == CSTR_EQUAL) {//TEST CompareString vs lstrcmp
				ReplaceWithClone(device.name, remoteName);
				device.address = ((SOCKADDR_BTH*)lookup.GetAddr())->btAddr;
				found = true;
				break;
			}
		}
		lookup.EndLookup();
	}
	catch (const BluetoothException& bex)
	{
		lookup.EndLookup();
		throw bex;
	}

	return found;
}

int BluetoothDevice::GetAvailableDevices(BluetoothDevice*& devices, bool flushCache)
{
	if (devices != nullptr) {//TODO custom list class AdVdBasics library?
		throw BluetoothException(L"Parameter devices must be nullptr, this method will allocate the necessary space");
	}

	int capacity = 4;
	devices = new BluetoothDevice[capacity];

	int index = 0;

	BluetoothLookup lookup;
	try {
		lookup.SetupForDeviceQuery(flushCache);
		lookup.BeginLookup();
		while (lookup.Next())
		{
			if (index >= capacity) {// Double capacity
				int newCapacity = capacity * 2;
				BluetoothDevice* aux = new BluetoothDevice[newCapacity];
				for (int i = 0; i < capacity; ++i) aux[i] = devices[i];
				delete[] devices;
				devices = aux;
				capacity = newCapacity;
			}

			BluetoothDevice& d = devices[index];

			d.name = CloneString(lookup.GetName());
			d.address = ((SOCKADDR_BTH*)lookup.GetAddr())->btAddr;

			index++;
		}
		lookup.EndLookup();
	}
	catch (const BluetoothException& bex)
	{
		delete[] devices;
		devices = nullptr;
		
		lookup.EndLookup();
		throw bex;
	}

	return index;
}

#pragma endregion

#pragma region BluetoothService


BluetoothService::BluetoothService()
{
	name = nullptr;
	comment = nullptr;
	serviceClassId = ServiceClass();
	address = BTH_ADDR_NULL;// 0UL;
	port = 0;
}

BluetoothService::~BluetoothService()
{
	if (name != nullptr) delete[] name;
	if (comment != nullptr) delete[] comment;
}

BluetoothService& BluetoothService::operator=(const BluetoothService& other)
{
	ReplaceWithClone(name, other.name);
	ReplaceWithClone(comment, other.comment);
	serviceClassId = other.serviceClassId;
	address = other.address;
	port = other.port;

	return *this;
}

BluetoothService::BluetoothService(const BluetoothService& other) : BluetoothService()
{
	name = CloneString(other.name);
	comment = CloneString(other.comment);
	serviceClassId = other.serviceClassId;
	address = other.address;
	port = other.port;
}


int BluetoothService::QueryServices(const BluetoothDevice& device, const ServiceClass& serviceClass, BluetoothService*& services, bool flushCache) 
{
	if (services != nullptr) {
		throw BluetoothException(L"Parameter services must be nullptr, this method will allocate the necessary space");
	}

	int capacity = 1;
	services = new BluetoothService[capacity];//TODO move flexible array logic to a custom list class

	int index = 0;

	BluetoothLookup lookup;
	try {
		lookup.SetupForServiceQuery(device.GetAddress(), serviceClass, flushCache);
		lookup.BeginLookup();
		while (lookup.Next())
		{
			if (index >= capacity) {// Double capacity
				int newCapacity = capacity * 2;
				BluetoothService* aux = new BluetoothService[newCapacity];
				for (int i = 0; i < capacity; ++i) aux[i] = services[i];
				delete[] services;
				services = aux;
				capacity = newCapacity;
			}

			BluetoothService& s = services[index];
			SOCKADDR_BTH* addr = (SOCKADDR_BTH*)lookup.GetAddr();

			s.name = CloneString(lookup.GetName());
			s.comment = CloneString(lookup.GetComment());
			//s.serviceClassId = GUIDToServiceClass(addr->serviceClassId);// Lookup doesn't return the service class on next
			//s.serviceClassId = GUIDToServiceClass(lookup.GetServiceClass());// Lookup doesn't return the service class on next
			s.serviceClassId = serviceClass;// Setting the passed service class
			s.address = addr->btAddr;
			s.port = addr->port;

			index++;
		}
		lookup.EndLookup();
	}
	catch (const BluetoothException& bex)
	{
		delete[] services;
		services = nullptr;

		lookup.EndLookup();
		throw bex;
	}
	
	return index;
}

void BluetoothService::RegisterService(const BluetoothSocket* localSocket, const wchar_t* name, const wchar_t* comment, const ServiceClass& serviceClass, BluetoothService& service)
{
	// Passing localSocket by pointer to prevent its destructor from being called
	if (localSocket == nullptr) {
		throw BluetoothException(L"Parameter localSocket shouldn't be nullptr");
	}
	ReplaceWithClone(service.name, name);
	ReplaceWithClone(service.comment, comment);
	service.serviceClassId = serviceClass;
	
	localSocket->GetAddressAndPort(service.address, service.port);// Throws exception

	SOCKADDR_BTH localAddress = { 0 };//This will hold address+port
	localAddress.addressFamily = AF_BTH;
	localAddress.btAddr = service.address;
	localAddress.port = service.port;
	
	CSADDR_INFO addressInfo;
	addressInfo.LocalAddr.iSockaddrLength = sizeof(SOCKADDR_BTH);
	addressInfo.LocalAddr.lpSockaddr = (LPSOCKADDR)&localAddress;
	addressInfo.RemoteAddr.iSockaddrLength = sizeof(SOCKADDR_BTH);
	addressInfo.RemoteAddr.lpSockaddr = (LPSOCKADDR)&localAddress;
	addressInfo.iSocketType = SOCK_STREAM;
	addressInfo.iProtocol = BTHPROTO_RFCOMM;

	WSAQUERYSET wsaQuerySet = { 0 };
	wsaQuerySet.dwSize = sizeof(WSAQUERYSET);
	wsaQuerySet.lpServiceClassId = (GUID*)&service.serviceClassId;

	wsaQuerySet.lpszServiceInstanceName = service.name;
	wsaQuerySet.lpszComment = service.comment;
	//wsaQuerySet.lpVersion//TODO? only if java/android does it
	wsaQuerySet.dwNameSpace = NS_BTH;
	wsaQuerySet.dwNumberOfCsAddrs = 1;      // Must be 1.
	wsaQuerySet.lpcsaBuffer = &addressInfo; // Req'd.

	int result = WSASetService(&wsaQuerySet, RNRSERVICE_REGISTER, 0);

	if (result == SOCKET_ERROR) {
		throw BluetoothException(L"WSASetService call failed", WSAGetLastError());
	}
}

void BluetoothService::DeleteService(const BluetoothService& service)
{
	SOCKADDR_BTH localAddress = { 0 };//This will hold address+port
	localAddress.addressFamily = AF_BTH;
	localAddress.btAddr = service.address;
	localAddress.port = service.port;

	CSADDR_INFO addressInfo;
	addressInfo.LocalAddr.iSockaddrLength = sizeof(SOCKADDR_BTH);
	addressInfo.LocalAddr.lpSockaddr = (LPSOCKADDR)&localAddress;
	addressInfo.RemoteAddr.iSockaddrLength = sizeof(SOCKADDR_BTH);
	addressInfo.RemoteAddr.lpSockaddr = (LPSOCKADDR)&localAddress;
	addressInfo.iSocketType = SOCK_STREAM;
	addressInfo.iProtocol = BTHPROTO_RFCOMM;

	WSAQUERYSET wsaQuerySet = { 0 };
	wsaQuerySet.dwSize = sizeof(WSAQUERYSET);
	wsaQuerySet.lpServiceClassId = (GUID*)&service.serviceClassId;

	wsaQuerySet.lpszServiceInstanceName = service.name;//Required for delete
	//wsaQuerySet.lpszComment = service.comment;//Not required for delete
	//wsaQuerySet.lpVersion//TODO?
	wsaQuerySet.dwNameSpace = NS_BTH;
	wsaQuerySet.dwNumberOfCsAddrs = 1;      // Must be 1.
	wsaQuerySet.lpcsaBuffer = &addressInfo; // Req'd.

	int result = WSASetService(&wsaQuerySet, RNRSERVICE_DELETE, 0);

	if (result == SOCKET_ERROR) {
		throw BluetoothException(L"WSASetService call failed", WSAGetLastError());
	}
}

#pragma endregion



#pragma region BluetoothSocket

BluetoothSocket::BluetoothSocket() : BluetoothSocket(0U)
{ }

BluetoothSocket::BluetoothSocket(uintptr_t handle)
{
	if (handle != 0U) socketHandle = handle;
	else socketHandle = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);

	if ((SOCKET)socketHandle == INVALID_SOCKET) {
		throw BluetoothException(L"socket() call failed", WSAGetLastError());
	}
}

void BluetoothSocket::Connect(const BluetoothService& service) const
{
	Connect(service.GetAddress(), service.GetServiceClassId());
}
void BluetoothSocket::Connect(const BluetoothDevice& remote, const ServiceClass& serviceClass) const
{
	Connect(remote.GetAddress(), serviceClass);
}
void BluetoothSocket::Connect(uint64_t remoteAddress, const ServiceClass& serviceClass) const
{
	SOCKET s = (SOCKET)socketHandle;

	SOCKADDR_BTH address = { 0 };
	address.btAddr = remoteAddress;
	address.addressFamily = AF_BTH;
	address.serviceClassId = ServiceClassToGUID(serviceClass);
	address.port = 0;

	int result = connect(s, (SOCKADDR*)&address, sizeof(SOCKADDR_BTH));
	if (result == SOCKET_ERROR) {
		throw BluetoothException(L"connect() call failed", WSAGetLastError());
	}
}

void BluetoothSocket::Bind() const
{
	SOCKADDR_BTH localAddr = { 0 };
	localAddr.addressFamily = AF_BTH;
	localAddr.port = BT_PORT_ANY;

	SOCKET s = (SOCKET)socketHandle;

	int result = bind(s, (SOCKADDR*)&localAddr, sizeof(SOCKADDR_BTH));
	if (result == SOCKET_ERROR) {
		throw BluetoothException(L"bind() call failed", WSAGetLastError());
	}
}

void BluetoothSocket::Listen() const
{
	SOCKET s = (SOCKET)socketHandle;

	int result = listen(s, LISTEN_BACKLOG);
	if (result == SOCKET_ERROR) {
		throw BluetoothException(L"listen() call failed", WSAGetLastError());
	}
}

void BluetoothSocket::Select(const BluetoothSocket** checkRead, const BluetoothSocket** checkWrite, const BluetoothSocket** checkError, uint32_t microseconds)
{
	FD_SET readSet;
	FD_SET writeSet;
	FD_SET errorSet;
	FD_ZERO(&readSet);
	FD_ZERO(&writeSet);
	FD_ZERO(&errorSet);
	if (checkRead != nullptr) {
		const BluetoothSocket** it = checkRead;//TODO break if set is full?
		for (const BluetoothSocket* socket = *it; socket != nullptr; socket = *(++it)) {//TODO test loop works correctly
			FD_SET(socket->socketHandle, &readSet);
		}
	}
	if (checkWrite != nullptr) {
		const BluetoothSocket** it = checkWrite;
		for (const BluetoothSocket* socket = *it; socket != nullptr; socket = *(++it)) {//TODO test loop works correctly
			FD_SET(socket->socketHandle, &writeSet);
		}
	}
	if (checkError != nullptr) {
		const BluetoothSocket** it = checkError;
		for (const BluetoothSocket* socket = *it; socket != nullptr; socket = *(++it)) {//TODO test loop works correctly
			FD_SET(socket->socketHandle, &errorSet);
		}
	}

	TIMEVAL timeout;
	timeout.tv_sec = 0L;
	timeout.tv_usec = microseconds;

	int result = select(0, &readSet, &writeSet, &errorSet, &timeout);
	if (result == SOCKET_ERROR) {
		throw BluetoothException(L"select() call failed", WSAGetLastError());
	}

	if (checkRead != nullptr) {
		const BluetoothSocket** it = checkRead;
		const BluetoothSocket** it2 = it;//Position to write sockets that remain
		for (const BluetoothSocket* socket = *it; socket != nullptr; socket = *(++it)) {//TODO test loop works correctly
			*it = nullptr;
			if (FD_ISSET(socket->socketHandle, &readSet)) {
				*it2 = socket;
				++it2;
			}
		}
	}
	if (checkWrite != nullptr) {
		const BluetoothSocket** it = checkWrite;
		const BluetoothSocket** it2 = it;//Position to write sockets that remain
		for (const BluetoothSocket* socket = *it; socket != nullptr; socket = *(++it)) {//TODO test loop works correctly
			*it = nullptr;
			if (FD_ISSET(socket->socketHandle, &writeSet)) {
				*it2 = socket;
				++it2;
			}
		}
	}
	if (checkError != nullptr) {
		const BluetoothSocket** it = checkError;
		const BluetoothSocket** it2 = it;//Position to write sockets that remain
		for (const BluetoothSocket* socket = *it; socket != nullptr; socket = *(++it)) {//TODO test loop works correctly
			*it = nullptr;
			if (FD_ISSET(socket->socketHandle, &errorSet)) {
				*it2 = socket;
				++it2;
			}
		}
	}
}



bool BluetoothSocket::Poll(uint32_t microseconds, int selectMode) const
{
	FD_SET set;
	FD_ZERO(&set);
	FD_SET(socketHandle, &set);

	TIMEVAL timeout;
	timeout.tv_sec = 0L;//TODO properly split? set everything to 0 and forget this?
	timeout.tv_usec = microseconds;

	int result;
	switch (selectMode) {
	case SELECT_READ:  result = select(0, &set, nullptr, nullptr, &timeout); break;
	case SELECT_WRITE: result = select(0, nullptr, &set, nullptr, &timeout); break;
	case SELECT_ERROR: result = select(0, nullptr, nullptr, &set, &timeout); break;
	default: 
		throw BluetoothException(L"Invalid selectMode, must be SELECT_READ, SELECT_WRITE or SELECT_ERROR");
	}
	if (result == SOCKET_ERROR) {
		throw BluetoothException(L"select() call failed", WSAGetLastError());
	}

	return FD_ISSET(socketHandle, &set);// result > 0 might work too
}


BluetoothSocket* BluetoothSocket::Accept() const
{
	SOCKET s = (SOCKET)socketHandle;

	SOCKET cs = accept(s, NULL, NULL);

	if (INVALID_SOCKET == cs) {
		throw BluetoothException(L"accept() call failed", WSAGetLastError());
	}
	return new BluetoothSocket(cs);
}


void BluetoothSocket::Send(const char* buffer, int length) const
{
	SOCKET s = (SOCKET)socketHandle;

	int result = send(s, buffer, length, 0);
	if (result == SOCKET_ERROR) {
		throw BluetoothException(L"send() call failed", WSAGetLastError());
	}
}

int BluetoothSocket::Receive(char* buffer, int length) const
{
	SOCKET s = (SOCKET)socketHandle;

	char* bufferIndex = buffer;
	int totalReceived = 0;
	while (totalReceived < length) {
		int received = recv(s, bufferIndex, length - totalReceived, 0);

		if (received == 0) break;

		if (received == SOCKET_ERROR) {
			throw BluetoothException(L"recv() call failed", WSAGetLastError());
		}

		if (received > (length - totalReceived)) {
			throw BluetoothException(L"Received too much data");
		}

		bufferIndex += received;
		totalReceived += received;
	}

	return totalReceived;
}

bool BluetoothSocket::IsValid() const
{
	return socketHandle != INVALID_SOCKET;
}

void BluetoothSocket::GetAddressAndPort(uint64_t& address, unsigned long& port) const
{
	SOCKADDR_BTH addr = { 0 };

	int addressLength = sizeof(SOCKADDR_BTH);
	int result = getsockname((SOCKET)socketHandle, (SOCKADDR*)&addr, &addressLength);

	if (result == SOCKET_ERROR) {
		throw BluetoothException(L"getsockname() call failed", WSAGetLastError());
	}

	address = addr.btAddr;
	port = addr.port;
}

void BluetoothSocket::Close()
{
	SOCKET s = (SOCKET)socketHandle;
	if (s != INVALID_SOCKET) {
		int result = closesocket(s);
		if (result == SOCKET_ERROR) {
			throw BluetoothException(L"closesocket() call failed", WSAGetLastError());
		}
	}
	socketHandle = INVALID_SOCKET;
}

BluetoothSocket::~BluetoothSocket()
{
	try { 
		Close();
	}
	catch (const BluetoothException& bex)
	{
		bex.Print();
	}
}

#pragma endregion

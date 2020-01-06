#pragma once

#include <string>

#ifdef ADVDBLUETOOTH_EXPORTS
#   define BTAPI __declspec(dllexport)
#else
#   define BTAPI __declspec(dllimport)
#endif

//TODO exception class, not exported? (different header? used in cpp only?)

class BluetoothException {
	const wchar_t* message;
	int64_t errorCode;

public:
	BluetoothException(const wchar_t* message, int64_t errorCode = -1) : message(message), errorCode(errorCode) { }

	const wchar_t* GetMessage() const { return message; }
	int64_t GetErrorCode() const { return errorCode; }

	void Print() const { wprintf(L"%s, [ErrorCode: %d]\n", message, errorCode); }
};


extern "C" BTAPI const unsigned int BluetoothMaxNameLength;//TODO "C"?
//extern BTAPI const unsigned int BluetoothMaxNameLength

extern "C" BTAPI bool InitializeBluetooth();//TODO call in dll main on load?
extern "C" BTAPI bool CleanupBluetooth();//TODO call in dll main on load?

//TODO do not use from c#, can throw!?
extern BTAPI const wchar_t* BluetoothAddressToString(uint64_t address);
extern BTAPI uint64_t BluetoothAddressFromString(const wchar_t* addressStr);

//extern "C" BTAPI bool QueryBluetoothService(const BluetoothDevice& device, bool flushCache);//TODO list services vs get single?
//extern "C" BTAPI bool RegisterBluetoothService(const class BluetoothSocket* localSocket, wchar_t* serviceName, wchar_t* comment);
//extern "C" BTAPI bool DeleteBluetoothService(const class BluetoothSocket* localSocket/*, wchar_t* serviceName, wchar_t* comment*/);
//DeleteBluetoothService()?

struct BTAPI ServiceClass {
	unsigned char id[4 + 2 + 2 + 8];

	ServiceClass();
	ServiceClass(const wchar_t* uuid);

	//bool FromString(const char* uuid);
	const wchar_t* MakeString() const;

	//TODO string conversions and marshaling(as byte[]?)
	//TODO string conversion for debugging?
	//TODO comparer?
};

// Serial Port Profile (SPP) Base UUID
extern "C" BTAPI const ServiceClass SPPBaseUUID;//TODO export and rename


struct BTAPI BluetoothDevice
{
public:
	BluetoothDevice();
	~BluetoothDevice();

	BluetoothDevice& operator=(const BluetoothDevice& other);
	BluetoothDevice(const BluetoothDevice& other);

	//bool TEST(const wchar_t* testName) {
	//	if (name != nullptr) delete name;
	//	name = CloneString(testName);
	//	//if (context != nullptr) delete context;
	//	//context = CloneString(L"TESTCTXT");//TODO is context useful?
	//	//if (comment != nullptr) delete comment;
	//	//comment = CloneString(L"TESTCOMMENT");
	//	return true;
	//}
private:
	//friend class BluetoothSocket;

	const wchar_t* name;// BTH_MAX_NAME_SIZE
	//const wchar_t* context;
	//TODO include guid?
	//const wchar_t* comment;//TODO remove comment: optionally obtain it from service query method
	uint64_t address;
	//void* data;//TODO override copy ctor to copy data in pointer instead of the address

public:
	const wchar_t* GetName() const { return name; }
	//const wchar_t* GetContext() const { return context; }//TODO check why context and comment remain null
	//const wchar_t* GetComment() const { return comment; }
	uint64_t GetAddress() const { return address; }
	//const wchar_t* AddressAsString() const;


	//TODO GetAvailableRemotes method? 
	static bool FindDeviceByAddress(const wchar_t* address, BluetoothDevice& device, bool flushCache = false);//TODO string must have a specific format
	static bool FindDeviceByAddress(uint64_t address, BluetoothDevice& device, bool flushCache = false);
	static bool FindDeviceByName(const wchar_t* name, BluetoothDevice& device, bool flushCache = false);//TODO handle retrying outside
	static int GetAvailableDevices(BluetoothDevice*& devices, bool flushCache = false);

	//TODO get self info (name+address) method as a Device instance?
};


struct BTAPI BluetoothService
{
public:
	BluetoothService();//TODO ctor that gets a socket, name and comment, then instance methods to register and delete
	~BluetoothService();

	BluetoothService& operator=(const BluetoothService& other);
	BluetoothService(const BluetoothService& other);

private:

	wchar_t* name;// BTH_MAX_NAME_SIZE
	wchar_t* comment;
	//TODO include guid?
	//GUID serviceClassId;
	//unsigned char serviceClassId[4 + 2 + 2 + 8];
	ServiceClass serviceClassId;//TODO is UUID generic enough? use fixed length array instead?
	uint64_t address;
	unsigned long port;
	//void* data;//TODO override copy ctor to copy data in pointer instead of the address

public:
	const wchar_t* GetName() const { return name; }
	const wchar_t* GetComment() const { return comment; }
	ServiceClass GetServiceClassId() const { return serviceClassId; }
	uint64_t GetAddress() const { return address; }
	//const wchar_t* AddressAsString() const;
	unsigned long GetPort() const { return port; }

	static int QueryServices(const BluetoothDevice& device, const ServiceClass& serviceClass, BluetoothService*& services, bool flushCache);
	//static int QueryBluetoothServices(const BluetoothDevice& device, bool flushCache);//TODO list services vs get single?
	
	//TODO warn, can only be used on local address services created with a special ctor? TODO alt: handle with static methods?
	//bool Register() const;
	//TODO warn, can only be used on local address services created with a special ctor?
	//bool Delete() const;
	//static bool RegisterBluetoothService(const class BluetoothSocket* localSocket, wchar_t* serviceName, wchar_t* comment);
	//static bool DeleteBluetoothService(const class BluetoothSocket* localSocket/*, wchar_t* serviceName, wchar_t* comment*/);

	static void RegisterService(const class BluetoothSocket* localSocket, const wchar_t* name, const wchar_t* comment, const ServiceClass& serviceClass, BluetoothService& service);
	static void DeleteService(const BluetoothService& service);//Must be local and registered
};



class BTAPI BluetoothSocket
{
public:
	BluetoothSocket();
	~BluetoothSocket();

	BluetoothSocket& operator=(const BluetoothSocket&) = delete;
	BluetoothSocket(const BluetoothSocket&) = delete;

private:
	BluetoothSocket(uintptr_t handle);

	//friend BTAPI bool RegisterBluetoothService(const BluetoothSocket* localSocket, wchar_t* serviceName, wchar_t* comment);
	uintptr_t socketHandle;
	//TODO add other fields if required by other systems

public:
	void Connect(const BluetoothService& service) const;
	void Connect(const BluetoothDevice& remote, const ServiceClass& serviceClass) const;
	void Connect(uint64_t remoteAddress, const ServiceClass& serviceClass) const;
	void Bind() const;
	void Listen() const;//TODO make methods const when possible?

#define SELECT_READ 0
#define SELECT_WRITE 1
#define SELECT_ERROR 2

	static void Select(const BluetoothSocket** checkRead, const BluetoothSocket** checkWrite, const BluetoothSocket** checkError, uint32_t microseconds);
	bool Poll(uint32_t microseconds, int selectMode) const;
	// The caller is responsible for closing this socket/deleting the pointed data
	BluetoothSocket* Accept() const;

	void Send(const char* buffer, int length) const;
	int Receive(char* buffer, int length) const;

	bool IsValid() const;
	void GetAddressAndPort(uint64_t& address, unsigned long& port) const;
	void Close();
};

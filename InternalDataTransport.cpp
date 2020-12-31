#include "InternalDataTransit.h"

DataTransit::DataTransit(csockdata* _config)
{
	this->_transitConfig = _config;
}

csockdata* DataTransit::GetTransitConfiguration()
{
	return this->_transitConfig;
}

bool DataTransit::IsOnline()
{
	return this->_transitOnline;
}

webxlib::csocket* DataTransit::GetTransitListener()
{
	return this->_listener;
}

webxlib::csocket* DataTransit::GetConnectedSocket()
{
	return this->_consock;
}

void DataTransit::EstablishTransit()
{
	this->_listener = new webxlib::csocket(this->_transitConfig);

	if (this->_listener->Bind() != CSOCKET_SUCCESS) {
		//bind failed
	} else if (this->_listener->Listen() != CSOCKET_SUCCESS) {
		//listen failed
	}

	this->_transitOnline = true;

	while (this->_transitOnline) {
		if (!this->_listener->IsValid()) {

			delete this->_listener;
			this->_listener = new webxlib::csocket(this->_transitConfig);
			if (this->_listener->Bind() == CSOCKET_SUCCESS) {
				if (this->_listener->Listen() == CSOCKET_SUCCESS) {
					//success
				}
			} else {
				//failed
			}

			continue;
		}

		if (this->_listener->SelectReadable({0,0}) > 0) {
			this->_consock = this->_listener->Accept();

			if (this->_consock->IsValid()) {
				CreateThread(NULL, NULL, _processReceivedPackage, (LPVOID)this, 0, NULL);
			} else {
				delete this->_consock;

				continue;
			}
		}
	}

	return;
}

DWORD WINAPI DataTransit::_processReceivedPackage(LPVOID arg)
{
	DataTransit* cl = (DataTransit*)arg;

	if (!cl->GetConnectedSocket()->IsValid())
		return NULL;

	char buff[1501];
	ZeroMemory(buff, 1501);

	while (int _got = cl->RecvData(buff, 1500)) {
		if (_got == CSOCKET_ERROR || strcmp(buff, "") == 0)
			break;

		printf("package received via internal data transit ==[[%s]]== %i bytes", buff, _got);
	}

	return NULL;
}

int DataTransit::WriteData(const char* data, int size)
{
	if (this->_consock == nullptr) {

		csockdata _config;
		_config.address = "127.0.0.1";
		_config.port = "70";
		_config.dataprotocol = TCPSOCK;
		_config.ipprotocol = IPV4SOCK;

		this->_consock = new webxlib::csocket(&_config);
		if (this->_consock->IsValid()) {
			this->_consock->Connect();
			int ret = this->_consock->Send(data, size);

			this->_consock = nullptr;
			return ret;
		} else {
			return NULL;
		}
	} else {
		if (this->_consock->IsValid()) {
			return this->_consock->Send(data, size);
		} else {
			return NULL;
		}
	}

	return NULL;
}

int DataTransit::WriteData(const char* data, int size, TransitCallback* _callbackPreSend, TransitCallback* _callbackPostSend)
{
	if (!this->_consock->IsValid() || this->_consock == nullptr) {
		csockdata _config;
		_config.address = "127.0.0.1";
		_config.port = "70";
		_config.dataprotocol = TCPSOCK;
		_config.ipprotocol = IPV4SOCK;

		this->_consock = new webxlib::csocket(&_config);
		this->_consock->Connect();

		(decltype(&this->_catalyst)(_callbackPreSend->_callbackRoutinePtr))((void*)_callbackPreSend->_transit);
		int _sentbytes = this->_consock->Send(data, size);
		(decltype(&this->_catalyst)(_callbackPostSend->_callbackRoutinePtr))((void*)_callbackPostSend->_transit);

		return _sentbytes;
	} else {
		(decltype(&this->_catalyst)(_callbackPreSend->_callbackRoutinePtr))((void*)_callbackPreSend->_transit);
		int _sentbytes = this->_consock->Send(data, size);
		(decltype(&this->_catalyst)(_callbackPostSend->_callbackRoutinePtr))((void*)_callbackPostSend->_transit);

		return _sentbytes;
	}

	return NULL;
}


int DataTransit::RecvData(char* data, int size)
{
	return this->_consock->Recv(data, size);
}

int DataTransit::RecvData(char* data, int size, TransitCallback* _callbackPreRecv, TransitCallback* _callbackPostRecv)
{
	(decltype(&this->_catalyst)(_callbackPreRecv->_callbackRoutinePtr))((void*)_callbackPreRecv->_transit);
	int _recvbytes = this->_consock->Recv(data, size);
	(decltype(&this->_catalyst)(_callbackPostRecv->_callbackRoutinePtr))((void*)_callbackPostRecv->_transit);

	return _recvbytes;
}
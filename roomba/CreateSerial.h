#ifndef CREATESERIAL_H
#define CREATESERIAL_H 
class CreateSerial
{
	public:
		CreateSerial();
		~CreateSerial();
		
		int InitCreateSerial();
		void CloseCreateSerial(int fd);
		void SendSerialToCreate(int fd, char* buf, int bufLength);
		void* CreateCallbackHandler(void* arg);
		void* CreateSerialHandler(void* arg);
	
	private:

};

#endif
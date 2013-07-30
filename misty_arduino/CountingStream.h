#include <Print.h>

class CountingStream : public Print
{
	virtual size_t write(uint8_t value)
	{
		httpBuf[httpBufIndex++]=value;
		httpBuf[httpBufIndex]=0;
		return httpBufIndex;
	};
	virtual size_t write(const uint8_t *buffer, size_t size)
	{
		for (int i=0;i<size;i++)
			httpBuf[httpBufIndex++]=buffer[i];
		httpBuf[httpBufIndex]=0;
		return httpBufIndex;

	};
	public :
	byte httpBuf[2048];
	int httpBufIndex;
};


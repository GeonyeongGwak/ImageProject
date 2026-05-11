#include <windows.h>

class CCrypt

{

public:

	CCrypt(void);

	~CCrypt(void);


	static BOOL Encrypt(BYTE* source, BYTE* destination, DWORD length);
	static BOOL Encrypt(UCHAR *source, UCHAR *destination, DWORD width, DWORD length);

	static BOOL Decrypt(BYTE* source, BYTE* destination, DWORD length);
	static BOOL Decrypt(UCHAR *source, UCHAR *destination, DWORD width, DWORD length);

	static BOOL CheckDecrypt(UCHAR *source, UCHAR *destination, DWORD width, DWORD length);
};
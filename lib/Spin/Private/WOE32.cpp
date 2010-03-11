#include <Windows.h>
#include <Acari/ThreadLocalStorage.h>

BOOL __stdcall DllMain(HANDLE instance, DWORD treason, LPVOID reserved)
{
	(void)instance;
	(void)reserved;
	static bool wsa_initialized__(false);
	switch (treason)
	{
	case DLL_PROCESS_ATTACH :
		{
			WSADATA wsa_data;
			int err(WSAStartup( MAKEWORD(2, 2), &wsa_data ));
			if ( err != 0 )
			{
				// HERE!! Tell the user that we could not find a usable WinSock DLL.
				return FALSE;
			}
			else
			{ /* all is well so far */ }

			/*
			 * Confirm that the WinSock DLL supports 2.2.
			 * Note that if the DLL supports versions greater    
			 * than 2.2 in addition to 2.2, it will still return 
			 * 2.2 in wVersion since that is the version we      
			 * requested.                                        
			 */

			if ( LOBYTE( wsa_data.wVersion ) != 2 ||
				 HIBYTE( wsa_data.wVersion ) != 2 ) {
					/* Tell the user that we could not find a usable */
					/* WinSock DLL.                                  */
					WSACleanup();
					return FALSE; 
			}
			else
			{ /* The WinSock DLL is acceptable. Proceed. */ }
			wsa_initialized__ = true;
			break;
		}
	case DLL_PROCESS_DETACH :
		if (wsa_initialized__)
			WSACleanup( );
		else
		{ /* failed to initialize - nothing to clean up */ }
		break;
	case DLL_THREAD_DETACH :
		Acari::ThreadLocalStorage::getInstance()._clean_();
		break;
	default :
		/* no-op */
		break;
	}

	return TRUE;
}
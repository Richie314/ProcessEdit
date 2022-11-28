#pragma once
#include "PE_App_Struct.h"

#define PE_APP_READONLY false
#define PE_APP_READWRITE true

namespace pe
{


	/// <summary>
	/// Returns the icon's path associated with that file extension
	/// </summary>
	/// <param name="sFileType">The file extension (ANSI)</param>
	/// <returns>The path to the file if found</returns>
	PE_API StringA getFileTypeIcon(const StringA sFileType);
	/// <summary>
	/// Returns the icon's path associated with that file extension
	/// </summary>
	/// <param name="sFileType">The file extension (UNICODE)</param>
	/// <returns>The path to the file if found</returns>
	PE_API StringW getFileTypeIcon(const StringW sFileType);
	/// <summary>
	/// Simple informations about a process, such as its id and his name
	/// </summary>
	typedef struct ProcessBasicInfo
	{
		/// <summary>
		/// The name of the process, 32 UNICODE characters
		/// </summary>
		Wchar swName[32];
		/// <summary>
		/// The user which owns the process, 32 UNICODE characters
		/// </summary>
		Wchar swOwner[32];
		/// <summary>
		/// The full path of the process, 64 UNICODE characters
		/// </summary>
		Wchar swPath[64];
		/// <summary>
		/// The id of the process
		/// </summary>
		Dword dwId;
	} AppBasicInfo, * LpAppBasicInfo, * pProcessBasicInfo;
	using PIDList = Array<Dword>;
	/// <summary>
	/// Class for interacting win windows process, 
	/// to use most of these methods the program should run as administrator
	/// </summary>
	class App
	{
	private:
		Handle hProc, hToken;
		Dword pId;
		Array<Hwnd> hwnds;
		StringW sName;
		StringW sLocation;
		StringW sUser;
		Uint __LastError;
		StringA __LastErrorStr;
		Bool dispose;
		PE_API void SetLastError(
			Uint error,
			cStringA errorStr);
	public:
		PE_INLINE constexpr Uint getLastError()const
		{
			return this->__LastError;
		}
		PE_INLINE constexpr const StringA getLastErrorStr()const
		{
			return this->__LastErrorStr;
		}
	private:
		PE_API void OpenStreamProcess(Bool);
		PE_API void UpdateWindowCount();
		PE_API void OpenProcessFromID(Dword, Bool);
		PE_API void FindProcessName();
		PE_API void ExtractTokenInfo();
		PE_API App& fromOtherStream(const App&);
		PE_API App& fromOtherStream(App&&);
	public:

		////////////////////////////////////////////////

		PE_API App();

		PE_API App(cStringW sProcess,
			Bool bCanWrite = PE_APP_READWRITE);

		PE_API App(Dword dwPID, Bool bCanWrite = PE_APP_READWRITE);

		PE_API App(const App& otherProc);
		PE_API App(App&& otherProc);

		PE_API App(const ProcessBasicInfo& pbiINFO,
			Bool bCanWrite = PE_APP_READWRITE);

		PE_API App(const pProcessBasicInfo pPBI,
			Bool bCanWrite = PE_APP_READWRITE);

		PE_API ~App();

		PE_API void Close();


		PE_API App& operator = (App&&);
		PE_API App& operator = (const App&);
		/// <summary>
		/// 
		/// </summary>
		/// <param name="addr"></param>
		/// <param name="bytesToRead"></param>
		/// <param name="destination"></param>
		/// <returns></returns>
		PE_API Bool ReadMemory(
			const Memory addr, Uint bytesToRead,
			Memory* destination)const;

		PE_API Bool PE_CALL WriteMemory(const Memory addr, const Memory buff, size_t blockSize);

		PE_API Hwnd FindThisWindowFromContent(cStringA caption)const;
		PE_API Hwnd FindThisWindowFromContent(cStringW caption)const;

		PE_INLINE Bool Good()const
		{
			return HandleGood(this->hProc) && this->pId != 0;
		}

		PE_API Bool PE_CALL Terminate(UInt exitCode = 0);

		PE_API Bool isRunning();

		PE_API Bool getExitCode(Dword& exitCode);

		PE_API const Handle getProcAddr()const;

		PE_API Handle getProcAddr();

		PE_API Bool InjectDll(cStringA libraryPath);
		PE_API Bool InjectDll(cStringW libraryPath);
		/// <summary>
		/// Forces the Process to call MessageBoxA, you can define the
		/// parameters and retreive the return value
		/// </summary>
		/// <param name="hWnd">The window to call MessageBoxA on</param>
		/// <param name="caption">The caption of the message box (max 256 char)</param>
		/// <param name="message">The body of the message box (max 256 char)</param>
		/// <param name="uType">Define the messagebox type, default is MB_ICON</param>
		/// <returns>The return value of MessageBoxA, 0 for error</returns>
		PE_API Int32 InjectMessageBox(Hwnd hWnd, cStringA caption,
			cStringA message, UInt uType = MB_OK);
		/// <summary>
		/// Forces the Process to call MessageBoxW, you can define the
		/// parameters and retreive the return value
		/// </summary>
		/// <param name="hWnd">The window to call MessageBoxW on</param>
		/// <param name="caption">The caption of the message box (max 256 wchar_t)</param>
		/// <param name="message">The body of the message box (max 256 wchar_t)</param>
		/// <param name="uType">Define the messagebox type, default is MB_ICON</param>
		/// <returns>The return value of MessageBoxW, 0 for error</returns>
		PE_API Int32 InjectMessageBox(Hwnd hWnd, cStringW caption,
			cStringW message, UInt uType = MB_OK);
	private:
		struct hook_t {
			Bool Active;
			Reserved::Hook hook;
		} _DeleteFileA, _DeleteFileW;
		PE_API void InitHook();
		PE_API Bool InternalHook(
			cStringA,
			cStringA,
			Memory, Memory);
		PE_API Bool InsertHookProcedure(Memory);
		PE_API Bool RestoreHook(Memory);
		PE_API Bool FindProcAddressInLibrary(LPCSTR, LPCSTR, Memory);
		PE_API void ListLibraries();
	public:
		/// <summary>
		/// Redirect the calls to DeleteFileA to the a new function.
		/// </summary>
		/// <param name="newFunction">The new function to redirect the calls to DeleteFileA, it must have the same prototype
		/// BOOL (WINAPI*)(LPCSTR)</param>
		/// <returns>true if the overwriting has worked</returns>
		PE_API Bool HookDeleteFileA(Reserved::__DeleteFileAProc newFunction);
		/// <summary>
		/// Redirect the calls to DeleteFileW to the a new function.
		/// </summary>
		/// <param name="newFunction">The new function to redirect the calls to DeleteFileW, it must have the same prototype
		/// BOOL (WINAPI*)(LPCWSTR)</param>
		/// <returns>true if the overwriting has worked</returns>
		PE_API Bool HookDeleteFileW(Reserved::__DeleteFileWProc newFunction);
		/// <summary>
		/// Restores the DeleteFileA function to its original memory
		/// </summary>
		/// <returns>true if the function is restored correctly</returns>
		PE_API Bool UnHookDeleteFileA();
		/// <summary>
		/// Restores the DeleteFileW function to its original memory
		/// </summary>
		/// <returns>true if the function is restored correctly</returns>
		PE_API Bool UnHookDeleteFileW();
	private:
		PE_API Dword FindProcId()const;
	public:
		/// <summary>
		/// Class for console stream
		/// </summary>
		class Console_t
		{
			Memory pParent;
			Handle hStream;
			Hwnd   hWnd;
			Bool   bGood;
			PE_API void UpdateHandle();
		public:
			/// <summary>
			/// Default builder, no stream is open
			/// </summary>
			PE_API Console_t();
			/// <summary>
			/// Builds from params
			/// </summary>
			/// <param name="parent">The Pie_314::App class which owns the console</param>
			/// <param name="stream">HANDLE to the console stream</param>
			/// <param name="window">HWND of the console</param>
			PE_API Console_t(Memory parent, Handle stream, Hwnd window);
			/// <summary>
			/// Default destroyer, doesn't do nothing particular
			/// </summary>
			PE_API ~Console_t();
			/// <summary>
			/// Tells you if the console exist and is working
			/// </summary>
			/// <returns>true if it is impossible to find a console</returns>
			PE_API Bool Missing()const;
			/// <summary>
			/// Creates the console if its not present
			/// </summary>
			/// <returns>TRUE if the console is created</returns>
			PE_API BOOL PE_CALL Create();
			/// <summary>
			/// Creates the console if its not present
			/// </summary>
			/// <param name="sTitle">The title of the console</param>
			/// <returns>TRUE if the console is created and the title is set</returns>
			PE_API BOOL PE_CALL Create(cStringA sTitle);
			/// <summary>
			/// Creates the console if its not present
			/// </summary>
			/// <param name="sTitle">The title of the console</param>
			/// <returns>TRUE if the console is created and the title is set</returns>
			PE_API BOOL PE_CALL Create(cStringW sTitle);
			/// <summary>
			/// Writes the string to the console
			/// </summary>
			/// <param name="sData">The Ansi string to write</param>
			/// <returns>The return value of WriteConsoleA or FALSE for error</returns>
			PE_API BOOL PE_CALL Write(cStringA sData);
			/// <summary>
			/// Writes the string to the console
			/// </summary>
			/// <param name="sData">The Unicode string to write</param>
			/// <returns>The return value of WriteConsoleW or FALSE for error</returns>
			PE_API BOOL PE_CALL Write(cStringW sData);
			/// <summary>
			/// Destroys the console window, calls FreeConsole()
			/// </summary>
			/// <returns>The value returned by FreeConsole</returns>
			PE_API BOOL PE_CALL Destroy();
			/// <summary>
			/// Changes the title of the console
			/// </summary>
			/// <param name="sTitle">The title of the console</param>
			/// <returns>TRUE if the title is set correctly</returns>
			PE_API BOOL PE_CALL SetTitle(cStringA sTitle);
			/// <summary>
			/// Changes the title of the console
			/// </summary>
			/// <param name="sTitle">The title of the console</param>
			/// <returns>TRUE if the title is set correctly</returns>
			PE_API BOOL PE_CALL SetTitle(cStringW sTitle);

			/// <summary>
			/// Do not call this function, is for internal use only!
			/// </summary>
			PE_API void pReserved(Memory, Memory, Memory);

		};
		/// <summary>
		/// The console of the process
		/// </summary>
		Console_t Console;
	private:
		PE_API Bool InjectFunctionInit(cStringA, cStringA, Memory, size_t);
		PE_API Bool InjectFunctionCreateThread(Memory, Memory, size_t);
		PE_API BOOL InjectFunctionGetReturnValue(Memory);
		
		PE_API BOOL InjectVoidFunction(cStringA, cStringA, Memory);
		
		PE_API BOOL InjectStringFunctionA(cStringA, cStringA, Memory, cStringA);
		PE_API BOOL InjectStringFunctionW(cStringA, cStringA, Memory, cStringW);
		
		PE_API BOOL InjectPrintFunctionA(cStringA, cStringA, Memory,
			Handle, cStringA, Dword*);
		PE_API BOOL InjectPrintFunctionW(cStringA, cStringA, Memory,
			Handle, cStringW, Dword*);
		
		PE_API BOOL InjectDWORDFunction(cStringA, cStringA, Memory, Dword);
	};
	/// <summary>
	/// Finds all the process with this name
	/// </summary>
	/// <param name="name">The process name</param>
	/// <returns>An array of PIDs</returns>
	PE_API PIDList FindProcId(cStringW name);
	/// <summary>
	/// Returns the name of the process
	/// </summary>
	/// <param name="dwPID">Process ID (DWORD)</param>
	/// <returns>The Name (UNICODE), empty string for error</returns>
	PE_API StringW FindProcNameW(Dword dwPID);
	/// <summary>
	/// Returns the name of the process
	/// </summary>
	/// <param name="dwPID">Process ID (DWORD)</param>
	/// <returns>The Name (ANSI), empty string for error</returns>
	PE_API StringA FindProcNameA(Dword dwPID);
#ifdef UNICODE
	/// <summary>
	/// Returns the name of the process
	/// </summary>
	/// <param name="dwPID">Process ID (DWORD)</param>
	/// <returns>The Name (UNICODE), empty string for error</returns>
	PE_INLINE constexpr String FindProcName(Dword dwPID)
	{
		return FindProcNameW(dwPID);
	}
#else
	/// <summary>
	/// Returns the name of the process
	/// </summary>
	/// <param name="dwPID">Process ID (DWORD)</param>
	/// <returns>The Name (ANSI), empty string for error</returns>
	PE_INLINE constexpr String FindProcName(Dword dwPID)
	{
		return FindProcNameA(dwPID);
	}
#endif
};
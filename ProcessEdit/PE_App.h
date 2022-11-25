#pragma once
#include "PE_Header.h"

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
	/// <summary>
	/// Class for interacting win windows process, 
	/// to use most of these methods the program should run as administrator
	/// </summary>
	class App
	{
	private:
		Handle hProc, hToken;
		Dword pId;
		Hwnd* hwnds;
		StringW sName;
		StringW sLocation;
		StringW sUser;
		Uint __LastError;
		StringA __LastErrorStr;
		Bool dispose;
		PE_INLINE void SetLastError(Uint error,
			const StringA& errorStr)
		{
			this->__LastError = error;
			this->__LastErrorStr = errorStr;
		}
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
		PE_API App& fromOtherStream(App&);
	public:

		////////////////////////////////////////////////

		PE_API App();

		PE_API App(const StringW& sProcess,
			Bool bCanWrite = PE_APP_READWRITE);

		PE_API App(Dword dwPID, Bool bCanWrite = PE_APP_READWRITE);

		PE_API App(App& otherProc);

		PE_API App(const ProcessBasicInfo& pbiINFO,
			Bool bCanWrite = PIE_APP_READWRITE);

		PIE_API App(const pProcessBasicInfo pPBI,
			Bool bCanWrite = PIE_APP_READWRITE);

		PIE_API ~App();

		PIE_API Procedure Close();

		/// <summary>
		/// With this method you can allocate an array inside the target process
		/// </summary>
		/// <typeparam name="T">The type of pointer</typeparam>
		/// <param name="count">how many elements the block will have</param>
		/// <returns>The pointer, 0 for error, MUST be deallocated with ::Free()</returns>
		template <class T = Byte> PIE_INLINE T* Allocate(size_t count)const
		{
			return static_cast<T*>(this->Aip.Create(count));
		}
		/// <summary>
		/// Frees a pointer allocated with Allocate
		/// </summary>
		/// <param name="addr">The pointer previously allocated</param>
		/// <returns>true/false for success/error, see the last error for more info</returns>
		PIE_INLINE Bool P_ENTRY Free(Memory addr)const
		{
			return this->Aip.Destroy(addr);
		}

		PIE_API App& operator = (App&);

		PIE_API const SuperByteArray ReadMemory(const Memory addr, Uint size = 64)const;

		PIE_API Bool P_ENTRY WriteMemory(const Memory addr, const SuperByteArray& buff);
		PIE_API Bool P_ENTRY WriteMemory(const Memory addr, const SuperAnsiString& buff);
		PIE_API Bool P_ENTRY WriteMemory(const Memory addr, const SuperUnicodeString& buff);
		PIE_API Bool P_ENTRY WriteMemory(const Memory addr, const Memory buff, size_t blockSize);

		PIE_API Hwnd FindThisWindowFromContent(const SuperAnsiString& caption)const;
		PIE_API Hwnd FindThisWindowFromContent(const SuperUnicodeString& caption)const;

		PIE_API SuperAnsiString toStringA()const;
		PIE_API SuperUnicodeString toStringW()const;

		PIE_INLINE Bool Good()const
		{
			return HandleGood(this->hProc) && this->pId != 0;
		}

		PIE_API Bool P_ENTRY Terminate(UInt exitCode = 0);

		PIE_API Bool isRunning();

		PIE_API Bool getExitCode(Dword& exitCode)const;

		PIE_API const Handle getProcAddr()const;

		PIE_API Handle getProcAddr();

		PIE_INLINE const PLoadedLibraries getLoadedLibraries()
		{
			this->ListLibraries();
			return this->ListLoadedLibraries;
		}

		PIE_API Bool InjectDll(const SuperAnsiString& libraryPath);
		PIE_API Bool InjectDll(const SuperUnicodeString& libraryPath);
		/// <summary>
		/// Forces the Process to call MessageBoxA, you can define the
		/// parameters and retreive the return value
		/// </summary>
		/// <param name="hWnd">The window to call MessageBoxA on</param>
		/// <param name="caption">The caption of the message box (max 256 char)</param>
		/// <param name="message">The body of the message box (max 256 char)</param>
		/// <param name="uType">Define the messagebox type, default is MB_ICON</param>
		/// <returns>The return value of MessageBoxA, 0 for error</returns>
		PIE_API Int32 InjectMessageBox(Hwnd hWnd, const SuperAnsiString& caption,
			const SuperAnsiString& message, UInt uType = MB_OK);
		/// <summary>
		/// Forces the Process to call MessageBoxW, you can define the
		/// parameters and retreive the return value
		/// </summary>
		/// <param name="hWnd">The window to call MessageBoxW on</param>
		/// <param name="caption">The caption of the message box (max 256 wchar_t)</param>
		/// <param name="message">The body of the message box (max 256 wchar_t)</param>
		/// <param name="uType">Define the messagebox type, default is MB_ICON</param>
		/// <returns>The return value of MessageBoxW, 0 for error</returns>
		PIE_API Int32 InjectMessageBox(Hwnd hWnd, const SuperUnicodeString& caption,
			const SuperUnicodeString& message, UInt uType = MB_OK);
	private:
		struct hook_t {
			Bool Active;
			Reserved::Hook hook;
		} _DeleteFileA, _DeleteFileW;
		PLoadedLibraries ListLoadedLibraries;
		PIE_API Procedure InitHook();
		PIE_API Bool InternalHook(const char*,
			const char*,
			Memory, Memory);
		PIE_API Bool InsertHookProcedure(Memory);
		PIE_API Bool RestoreHook(Memory);
		PIE_API Bool FindProcAddressInLibrary(LPCSTR, LPCSTR, Memory);
		PIE_API Procedure ListLibraries();
	public:
		/// <summary>
		/// Redirect the calls to DeleteFileA to the a new function.
		/// </summary>
		/// <param name="newFunction">The new function to redirect the calls to DeleteFileA, it must have the same prototype
		/// BOOL (WINAPI*)(LPCSTR)</param>
		/// <returns>true if the overwriting has worked</returns>
		PIE_API Bool HookDeleteFileA(Reserved::__DeleteFileAProc newFunction);
		/// <summary>
		/// Redirect the calls to DeleteFileW to the a new function.
		/// </summary>
		/// <param name="newFunction">The new function to redirect the calls to DeleteFileW, it must have the same prototype
		/// BOOL (WINAPI*)(LPCWSTR)</param>
		/// <returns>true if the overwriting has worked</returns>
		PIE_API Bool HookDeleteFileW(Reserved::__DeleteFileWProc newFunction);
		/// <summary>
		/// Restores the DeleteFileA function to its original memory
		/// </summary>
		/// <returns>true if the function is restored correctly</returns>
		PIE_API Bool UnHookDeleteFileA();
		/// <summary>
		/// Restores the DeleteFileW function to its original memory
		/// </summary>
		/// <returns>true if the function is restored correctly</returns>
		PIE_API Bool UnHookDeleteFileW();
	private:
		PIE_API Dword FindProcId()const;
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
			PIE_API Procedure UpdateHandle();
		public:
			/// <summary>
			/// Default builder, no stream is open
			/// </summary>
			PIE_API Console_t();
			/// <summary>
			/// Builds from params
			/// </summary>
			/// <param name="parent">The Pie_314::App class which owns the console</param>
			/// <param name="stream">HANDLE to the console stream</param>
			/// <param name="window">HWND of the console</param>
			PIE_API Console_t(Memory parent, Handle stream, Hwnd window);
			/// <summary>
			/// Default destroyer, doesn't do nothing particular
			/// </summary>
			PIE_API ~Console_t();
			/// <summary>
			/// Tells you if the console exist and is working
			/// </summary>
			/// <returns>true if it is impossible to find a console</returns>
			PIE_API Bool Missing()const;
			/// <summary>
			/// Creates the console if its not present
			/// </summary>
			/// <returns>TRUE if the console is created</returns>
			PIE_API BOOL P_ENTRY Create();
			/// <summary>
			/// Creates the console if its not present
			/// </summary>
			/// <param name="sTitle">The title of the console</param>
			/// <returns>TRUE if the console is created and the title is set</returns>
			PIE_API BOOL P_ENTRY Create(const SuperAnsiString& sTitle);
			/// <summary>
			/// Creates the console if its not present
			/// </summary>
			/// <param name="sTitle">The title of the console</param>
			/// <returns>TRUE if the console is created and the title is set</returns>
			PIE_API BOOL P_ENTRY Create(const SuperUnicodeString& sTitle);
			/// <summary>
			/// Writes the string to the console
			/// </summary>
			/// <param name="sData">The Ansi string to write</param>
			/// <returns>The return value of WriteConsoleA or FALSE for error</returns>
			PIE_API BOOL P_ENTRY Write(const SuperAnsiString& sData);
			/// <summary>
			/// Writes the string to the console
			/// </summary>
			/// <param name="sData">The Unicode string to write</param>
			/// <returns>The return value of WriteConsoleW or FALSE for error</returns>
			PIE_API BOOL P_ENTRY Write(const SuperUnicodeString& sData);
			/// <summary>
			/// Destroys the console window, calls FreeConsole()
			/// </summary>
			/// <returns>The value returned by FreeConsole</returns>
			PIE_API BOOL P_ENTRY Destroy();
			/// <summary>
			/// Changes the title of the console
			/// </summary>
			/// <param name="sTitle">The title of the console</param>
			/// <returns>TRUE if the title is set correctly</returns>
			PIE_API BOOL P_ENTRY SetTitle(const SuperAnsiString& sTitle);
			/// <summary>
			/// Changes the title of the console
			/// </summary>
			/// <param name="sTitle">The title of the console</param>
			/// <returns>TRUE if the title is set correctly</returns>
			PIE_API BOOL P_ENTRY SetTitle(const SuperUnicodeString& sTitle);

			/// <summary>
			/// Do not call this function, is for internal use only!
			/// </summary>
			PIE_API Procedure pReserved(Memory, Memory, Memory);

		};
		/// <summary>
		/// The console of the process
		/// </summary>
		Console_t Console;
	private:
		PIE_API BOOL InjectVoidFunction(const char*, const char*, Memory);
		PIE_API BOOL InjectStringFunctionA(const char*, const char*, Memory, const char*);
		PIE_API BOOL InjectStringFunctionW(const char*, const char*, Memory, const wchar_t*);
		PIE_API BOOL InjectPrintFunctionA(const char*, const char*, Memory,
			Handle, const SuperAnsiString&, Dword*);
		PIE_API BOOL InjectPrintFunctionW(const char*, const char*, Memory,
			Handle, const SuperUnicodeString&, Dword*);
		PIE_API BOOL InjectDWORDFunction(const char*, const char*, Memory, Dword);

	public:
		/// <summary>
		/// Loads the icon from the process file,
		/// the icon must be destroyed with DestroyIcon later.
		/// </summary>
		/// <param name="iconType">Can be any of the enumICON *_SIZE tags, 
		/// for MAX_SIZE it is required an operative system greater than VISTA.</param>
		/// <returns>The icon requested or NULL for error, 
		/// to get extended info see app::getLastError.</returns>
		PIE_API HIcon P_ENTRY getIcon(enumICON iconType = enumICON::MAX_SIZE);

	};

	PIE_INLINE std::ostream& operator << (std::ostream& os, const App& app)
	{
		os << app.toStringA();
		os.flush();
		return os;
	}
	PIE_INLINE std::wostream& operator << (std::wostream& wos, const App& app)
	{
		wos << app.toStringW();
		wos.flush();
		return wos;
	}
	/// <summary>
	/// Finds the first process id with the name
	/// </summary>
	/// <param name="name">The process name</param>
	/// <returns>The process id (unsigned long), 0 for errors</returns>
	PIE_API PIDlist FindProcId(const SuperUnicodeString& name);
	/// <summary>
	/// Returns the name of the process
	/// </summary>
	/// <param name="dwPID">Process ID (DWORD)</param>
	/// <returns>The Name (UNICODE), empty string for error</returns>
	PIE_API SuperUnicodeString FindProcNameW(Dword dwPID);
	/// <summary>
	/// Returns the name of the process
	/// </summary>
	/// <param name="dwPID">Process ID (DWORD)</param>
	/// <returns>The Name (ANSI), empty string for error</returns>
	PIE_API SuperAnsiString FindProcNameA(Dword dwPID);
#ifdef UNICODE
	/// <summary>
	/// Returns the name of the process
	/// </summary>
	/// <param name="dwPID">Process ID (DWORD)</param>
	/// <returns>The Name (UNICODE), empty string for error</returns>
	PIE_INLINE SuperUnicodeString FindProcName(Dword dwPID)
	{
		return std::move(Pie_314::FindProcNameW(dwPID));
	}
#else
	/// <summary>
	/// Returns the name of the process
	/// </summary>
	/// <param name="dwPID">Process ID (DWORD)</param>
	/// <returns>The Name (ANSI), empty string for error</returns>
	PIE_INLINE SuperAnsiString FindProcName(Dword dwPID)
	{
		return std::move(Pie_314::FindProcNameA(dwPID));
	}
#endif

	using SuperProcessInfoArray = SuperArray<
		ProcessBasicInfo, BasicAllocator<ProcessBasicInfo>, false>;
	PIE_API SuperProcessInfoArray ListAllProcess();
};
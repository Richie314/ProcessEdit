#include "pch.h"
#include "Pie_App.h"
using namespace Pie_314;
Bool P_ENTRY Window::SimulateKeyPress(wchar_t w)
{
	if (this->Good())
		return this->SendSingleKey(w);
	this->setLastError(PIE_ERROR_HANDLE_ERROR | PIE_ERROR_INVALID_CALL);
	return false;
}
Bool P_ENTRY Window::SimulateKeyPress(char c)
{
	if (this->Good())
		return this->SendSingleKey(static_cast<wchar_t>(c));
	this->setLastError(PIE_ERROR_HANDLE_ERROR | PIE_ERROR_INVALID_CALL);
	return false;
}
Bool P_ENTRY Window::SimulateKeyPress(const SuperUnicodeString& sMessage)
{
	if (this->Good())
	{
		return this->SendKeyArray((Key*)&(sMessage.first()), sMessage.size);
	}
	this->setLastError(PIE_ERROR_HANDLE_ERROR | PIE_ERROR_INVALID_CALL);
	return false;
}
Bool P_ENTRY Window::SimulateKeyPress(const SuperAnsiString& sMessage)
{
	if (this->Good())
	{
		return this->SendKeyArray((Key*)(sMessage.forEach<wchar_t>([](const char& c) {
			return static_cast<wchar_t>(c); })).c_str(),
			sMessage.size * 2);
	}
	this->setLastError(PIE_ERROR_HANDLE_ERROR | PIE_ERROR_INVALID_CALL);
	return false;
}


Bool Window::SendSingleKey(Key key)
{
	if (!SetForegroundWindow(this->hwnd))
		return false;
	INPUT inputs[2];
	memset(inputs, 0, 2 * sizeof(INPUT));
	inputs[0].type = inputs[1].type = INPUT_KEYBOARD;
	inputs[0].ki.wScan = inputs[1].ki.wScan = key;
	inputs[0].ki.dwExtraInfo =
		inputs[1].ki.dwExtraInfo = GetMessageExtraInfo();
	inputs[0].ki.dwFlags = KEYEVENTF_UNICODE;
	inputs[1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
	Bool bReturn = (SendInput(2, inputs, sizeof(INPUT))
		== 2);
	Heap.Destroy(inputs);
	return bReturn;
}
Bool Window::SendKeyArray(const LPKey keys, size_t uCount)
{
	if (!SetForegroundWindow(this->hwnd))
		return false;
	size_t uRealCount = uCount * 2;
	auto inputs = (INPUT*)Heap.Create(uRealCount * sizeof(INPUT));
	for (Uint i = 0; i < uRealCount; i += 2)
	{
		inputs[i].type = inputs[i + 1].type = INPUT_KEYBOARD;
		inputs[i].ki.wScan = inputs[i + 1].ki.wScan = keys[i];
		inputs[i].ki.dwExtraInfo =
			inputs[i + 1].ki.dwExtraInfo = GetMessageExtraInfo();
		inputs[i].ki.dwFlags = KEYEVENTF_UNICODE;
		inputs[i + 1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
	}
	Bool bReturn = (SendInput(uRealCount, inputs, sizeof(INPUT))
		== uRealCount);
	Heap.Destroy(inputs);
	return bReturn;
}
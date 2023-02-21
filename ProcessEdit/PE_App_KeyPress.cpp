#include "pch.h"
#include "PE_App.h"
#include <vector>
using namespace pe;
Bool PE_CALL App::SimulateKeyPress(Wchar w)
{
	if (Good())
		return SendSingleKey(w);
	return false;
}
Bool PE_CALL App::SimulateKeyPress(Char c)
{
	if (Good())
		return SendSingleKey(static_cast<wchar_t>(c));
	return false;
}
Bool PE_CALL App::SimulateKeyPress(cStringW sMessage)
{
	if (Good())
	{
		return SendKeyArray((const Key*)sMessage, str::LenW(sMessage));
	}
	return false;
}
Bool PE_CALL App::SimulateKeyPress(cStringA sMessage)
{
	if (Good())
	{
		StringW sW = str::AtoW(sMessage);
		size_t size = str::LenA(sMessage);
		Bool b = SendKeyArray((const Key*)sW, size);
		str::FreeW(&sW, size);
		return b;
	}
	return false;
}


Bool App::SendSingleKey(Key key, Hwnd hwnd)
{
	if (hwnd)
	{
		if (!SetForegroundWindow(hwnd))
			return false;
	}
	else {
		if (hwnds.Value && *hwnds.Value)
			if (!SetForegroundWindow(*hwnds.Value))
				return false;
	}
	INPUT inputs[2];
	memset(inputs, 0, 2 * sizeof(INPUT));
	inputs[0].type = inputs[1].type = INPUT_KEYBOARD;
	inputs[0].ki.wScan = inputs[1].ki.wScan = key;
	inputs[0].ki.dwExtraInfo =
		inputs[1].ki.dwExtraInfo = GetMessageExtraInfo();
	inputs[0].ki.dwFlags = KEYEVENTF_UNICODE;
	inputs[1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
	return SendInput(2, inputs, sizeof(INPUT)) == 2;
}
Bool App::SendKeyArray(const Key* keys, size_t uCount, Hwnd hwnd)
{
	if (uCount == 0)
		return false;
	if (hwnd)
	{
		if (!SetForegroundWindow(hwnd))
			return false;
	}
	else {
		if (hwnds.Value && *hwnds.Value)
			if (!SetForegroundWindow(*hwnds.Value))
				return false;
	}
	std::vector<INPUT> inputs(uCount * 2U);
	for (Uint i = 0; i < inputs.size(); i += 2)
	{
		inputs[i].type = inputs[i + 1].type = INPUT_KEYBOARD;
		inputs[i].ki.wScan = inputs[i + 1].ki.wScan = keys[i];
		inputs[i].ki.dwExtraInfo =
			inputs[i + 1].ki.dwExtraInfo = GetMessageExtraInfo();
		inputs[i].ki.dwFlags = KEYEVENTF_UNICODE;
		inputs[i + 1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
	}
	return SendInput(inputs.size(), &inputs[0], sizeof(INPUT)) == inputs.size();
}
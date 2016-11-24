#include "StdAfx.h"
#include "ef_trial.h"

bool Cef_trial::Mount(CArcFile* archive)
{
	if (!archive->CheckExe(_T("ef_trial.exe")))
		return false;

	return CPaz::Mount(archive);
}

bool Cef_trial::Decode(CArcFile* archive)
{
	if (!archive->CheckExe(_T("ef_trial.exe")))
		return false;

	return CPaz::Decode(archive);
}

void Cef_trial::InitMountKey(CArcFile* archive)
{
	static const KeyInfo key_info[] =
	{
		{ _T("scr"),    "\x37\xE8\x78\x4D\xEA\xB4\x88\x3C\x4F\x52\x35\xDC\xC6\x02\x97\x5E\x4F\x15\xBF\x7D\xF8\xA4\x99\xD0\xE8\x71\x4C\xAC\xC4\xAC\xD9\x47" },
		{ _T("bg"),     "\x35\x71\xB4\xF4\x41\xAA\x16\x22\x4E\x1F\xCF\xE8\x71\x71\x55\xE3\xE6\xC0\xCA\x17\x5B\xC4\xC1\xA6\xDE\x07\x80\x0A\x21\x16\x7C\x9D" },
		{ _T("st"),     "\xDE\x53\xD1\x35\x18\x3D\x7A\x81\xF0\xDF\xFD\xF6\xE2\x2C\x5A\x0E\x14\x04\xF6\x89\x85\xAC\x3C\x7F\xDA\x69\xBF\xFF\x81\x10\x99\xFC" },
		{ _T("sys"),    "\xF9\x3D\xC1\x3A\xEC\xF3\x1E\x1B\xEB\x72\x1F\xE2\x86\xEB\x7F\x55\x7D\xF1\x15\x5D\xD3\x43\x54\x7C\x70\xD4\x4D\xC8\x33\xB2\xED\x37" },
		{ _T("bgm"),    "\x0B\x9F\x34\xF0\xFB\x16\x2C\x75\xB3\x76\x57\x48\x89\x26\x5E\xEA\x88\x55\xB9\xE0\x62\x32\x12\x83\xF8\x49\x29\x61\x44\xD4\xEE\xE2" },
		{ _T("voice"),  "\x59\xA9\x9D\x02\x42\xAB\x8D\xD1\x7F\x4C\x88\x82\xDB\x14\x51\xC0\x5A\xC1\x33\x1C\x10\xDF\x40\x36\x86\x88\x16\x83\x81\x0B\xD8\x50" },
		{ _T("se"),     "\xE8\x4B\x2C\xDB\x7F\x7A\xEA\x34\x45\x13\x51\xAC\x27\xAB\x71\x8C\xD8\x83\x94\xDD\x7B\x73\x67\xFA\xF0\x0F\x94\xAC\x78\xB1\xA4\x96" },
		{ _T("mov"),    "\x7E\x32\xD4\xA7\x2F\x0C\xAE\x61\xB8\xAA\x15\xA2\xDC\xA5\x97\xC0\xA9\xAC\xAF\xAF\x00\xD4\xD1\xF1\xCB\x1E\xE6\x16\x77\xDD\x0B\x86" }
	};

	SetKey(archive, key_info);
}

void Cef_trial::InitDecodeKey(CArcFile* archive)
{
	static const KeyInfo key_info[] =
	{
		{ _T("scr"),    "\x46\x79\xA3\xCE\x7F\x4E\xD6\x9D\x62\x0E\x42\x95\x1D\x19\x1D\x05\xCB\xEC\x7B\x25\x89\xF3\xCB\x82\x1E\x8D\xD9\xFD\x04\xF4\x10\x18" },
		{ _T("bg"),     "\x85\xD5\xAA\x49\xC1\x57\xAE\x1A\x44\x48\x5A\xDB\xFF\x37\xD3\x98\x5B\x41\xCB\x24\x23\x41\xEB\xE3\x17\x30\xAD\x2E\xCB\x70\x7F\xFA" },
		{ _T("st"),     "\xAF\xC8\xD7\x8F\xEB\xC4\x09\x09\xD9\x43\x9A\x65\xDF\xF8\xCB\xDD\x61\x8E\x21\x4D\x8C\x3E\xAB\x61\xC6\x8B\xC2\x59\x6F\xF3\x07\x0F" },
		{ _T("sys"),    "\xCB\x43\x5A\x0C\xBA\x5B\x90\x6E\x17\x1F\xA4\x50\x6B\x53\x20\x87\xC4\x22\x8E\x6A\x62\x11\x94\x5F\x00\x20\x99\xFA\x7E\xD5\xA1\x6B" },
		{ _T("bgm"),    "\x9F\xF4\x26\xED\xAE\xA4\xAE\x0D\xB3\xBB\xD8\x76\x0F\x00\xAC\x0A\x29\x0C\xE5\x08\x02\xA2\xF0\x00\x5C\x2C\x73\x4D\x43\x2B\x05\xE1" },
		{ _T("voice"),  "\xAF\x4B\xEC\x1C\x02\xE6\x8B\x6A\x24\xB8\x58\x74\xF9\x77\x08\x99\xF6\x1C\xB6\x73\x8A\x98\xC8\x28\x2F\xB0\x52\x4C\xCE\xCF\xAF\x05" },
		{ _T("se"),     "\x41\x77\x1D\x44\xB4\x29\x11\xC7\x9E\x74\x06\xA4\x18\xEE\x8D\x27\x4F\xE1\x53\xB4\xD6\x5A\xE6\x7B\x8E\x6C\xF7\xB5\xEB\x58\xD6\x2B" },
		{ _T("mov"),    "\x5B\x69\xEB\xD2\x82\x35\xE9\x28\x17\x0F\x84\x23\x17\x5E\x56\x69\x1B\xAB\xCE\x9A\x86\x11\xD4\x5D\x4E\xDE\xE5\x01\x28\x1D\x75\x66" }
	};

	SetKey(archive, key_info);
}

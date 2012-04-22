#include "stdafx.h"
#include "../ExtractBase.h"
#include "../Arc/Zlib.h"
#include "../Image.h"
#include "Nitro.h"

BOOL CNitro::Mount(CArcFile* pclArc)
{
	if (MountPak1(pclArc) == TRUE)
		return TRUE;
	if (MountPak2(pclArc) == TRUE)
		return TRUE;
	if (MountPak3(pclArc) == TRUE)
		return TRUE;
	if (MountPak4(pclArc) == TRUE)
		return TRUE;
	if (MountPK2(pclArc) == TRUE)
		return TRUE;
	if (MountN3Pk(pclArc) == TRUE)
		return TRUE;
//	if (MountPck(pclArc) == TRUE)
//		return TRUE;
	if (MountNpp(pclArc) == TRUE)
		return TRUE;
//	if (MountNpa(pclArc) == TRUE)
//		return TRUE;
	return FALSE;
}

BOOL CNitro::MountPak1(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".pak"))
		return FALSE;
	if (memcmp(pclArc->GetHed(), "\x01\0\0\0", 4) != 0)
		return FALSE;

	// �t�@�C�����擾
	DWORD ctFile;
	pclArc->Seek(4, FILE_BEGIN);
	pclArc->Read(&ctFile, 4);

	// �C���f�b�N�X�T�C�Y�擾
	DWORD index_size;
	pclArc->Read(&index_size, 4);

	// ���k�C���f�b�N�X�T�C�Y�擾
	DWORD index_compsize;
	pclArc->Read(&index_compsize, 4);

	// �o�b�t�@�m��
	YCMemory<BYTE> z_index(index_compsize);
	YCMemory<BYTE> index(index_size);
	LPBYTE pIndex = &index[0];

	// ���k�C���f�b�N�X�擾
	pclArc->Read(&z_index[0], index_compsize);

	// �C���f�b�N�X�擾
	CZlib zlib;
	zlib.Decompress(pIndex, &index_size, &z_index[0], index_compsize);

	DWORD offset = 0x10 + index_compsize;

	for (int i = 0; i < (int)ctFile; i++) {
		SFileInfo infFile;

		// �t�@�C�����擾
		TCHAR szFileName[_MAX_FNAME];
		DWORD len = *(LPDWORD)&pIndex[0];
		memcpy(szFileName, &pIndex[4], len);
		szFileName[len] = _T('\0');
		pIndex += 4 + len;

		infFile.start = *(LPDWORD)&pIndex[0] + offset; // �J�n�A�h���X��0����ɂ��Ďn�܂��Ă���̂ŕ␳
		infFile.sizeOrg = *(LPDWORD)&pIndex[4];
		DWORD bCmp = *(LPDWORD)&pIndex[12];
		infFile.sizeCmp = *(LPDWORD)&pIndex[16];
		if (infFile.sizeCmp == 0)
			infFile.sizeCmp = infFile.sizeOrg;
		pIndex += 20;

		// ���X�g�r���[�ɒǉ�
		infFile.name = szFileName;
		infFile.end = infFile.start + infFile.sizeCmp;
		if (bCmp) infFile.format = _T("zlib");
		infFile.title = _T("Pak1");
		pclArc->AddFileInfo(infFile);
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	�a���吹�f�����x�C����*.pak�̃t�@�C�������擾����֐�

BOOL	CNitro::MountPak2(
	CArcFile*			pclArc							// �A�[�J�C�u
	)
{
	if( pclArc->GetArcExten() != _T(".pak") )
	{
		return	FALSE;
	}

	if( memcmp( pclArc->GetHed(), "\x02\0\0\0", 4) != 0 )
	{
		return	FALSE;
	}

	// �t�@�C�����擾

	DWORD				dwFiles;

	pclArc->SeekHed( 4 );
	pclArc->Read( &dwFiles, 4 );

	// �C���f�b�N�X�T�C�Y�擾

	DWORD				dwIndexSize;

	pclArc->Read( &dwIndexSize, 4 );

	// ���k�C���f�b�N�X�T�C�Y�擾

	DWORD				dwIndexCompSize;

	pclArc->Read( &dwIndexCompSize, 4 );

	// �o�b�t�@�m��

	YCMemory<BYTE>		clmCompIndex( dwIndexCompSize );
	YCMemory<BYTE>		clmIndex( dwIndexSize );

	// ���k�C���f�b�N�X�擾

	pclArc->SeekHed( 0x114 );
	pclArc->Read( &clmCompIndex[0], dwIndexCompSize );

	// �C���f�b�N�X�擾

	CZlib				clZlib;

	clZlib.Decompress( &clmIndex[0], &dwIndexSize, &clmCompIndex[0], dwIndexCompSize );

	DWORD				dwIndexPtr = 0;
	DWORD				dwOffset = 0x114 + dwIndexCompSize;

	for( DWORD i = 0 ; i < dwFiles ; i++ )
	{
		SFileInfo			stFileInfo;

		// �t�@�C�����擾

		char				szFileName[_MAX_FNAME];

		DWORD				dwFileNameLength = *(DWORD*) &clmIndex[dwIndexPtr + 0];

		memcpy( szFileName, &clmIndex[dwIndexPtr + 4], dwFileNameLength );
		szFileName[dwFileNameLength] = '\0';

		dwIndexPtr += 4 + dwFileNameLength;

		// �t���O�̎擾

		DWORD				dwFlags = *(DWORD*) &clmIndex[dwIndexPtr + 12];

		// ���X�g�r���[�ɒǉ�

		stFileInfo.name = szFileName;
		stFileInfo.start = *(DWORD*) &clmIndex[dwIndexPtr + 0] + dwOffset;
		stFileInfo.sizeOrg = *(DWORD*) &clmIndex[dwIndexPtr + 4];
		stFileInfo.sizeCmp = *(DWORD*) &clmIndex[dwIndexPtr + 16];

		if( stFileInfo.sizeCmp == 0 )
		{
			stFileInfo.sizeCmp = stFileInfo.sizeOrg;
		}

		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;

		if( dwFlags != 0 )
		{
			stFileInfo.format = _T("zlib");
		}

		stFileInfo.title = _T("Pak2");

		pclArc->AddFileInfo( stFileInfo );

		dwIndexPtr += 20;
	}

	return TRUE;
}

BOOL CNitro::MountPak3(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".pak"))
		return FALSE;
	if (memcmp(pclArc->GetHed(), "\x03\0\0\0", 4) != 0)
		return FALSE;

	// �V�O�l�`���ǂݍ���
	BYTE sig[256];
	pclArc->Seek(4, FILE_BEGIN);
	pclArc->Read(sig, sizeof(sig));

	// �����L�[����
	DWORD key = 0;
	for (int i = 0; i < 256; i++) {
		if (sig[i] == 0)
			break;
		key *= 0x89;
		key += sig[i];
	}

	// �w�b�_�ǂݍ���
	BYTE header[16];
	pclArc->Read(header, sizeof(header));

	// ���k�C���f�b�N�X�T�C�Y�擾
	DWORD index_compsize = *(LPDWORD)&header[0] ^ *(LPDWORD)&header[12];

	// �C���f�b�N�X�T�C�Y�擾
	DWORD index_size = *(LPDWORD)&header[4] ^ key;

	// �t�@�C�����擾
	DWORD ctFile = *(LPDWORD)&header[8] ^ key;

	// �o�b�t�@�m��
	YCMemory<BYTE> z_index(index_compsize);
	YCMemory<BYTE> index(index_size);
	LPBYTE pIndex = &index[0];

	// ���k�C���f�b�N�X�擾
	pclArc->Read(&z_index[0], index_compsize);

	// �C���f�b�N�X�擾
	CZlib zlib;
	zlib.Decompress(pIndex, &index_size, &z_index[0], index_compsize);

	DWORD offset = 0x114 + index_compsize;
	DWORD FileEnd_prev = 0;

	for (int i = 0; i < (int)ctFile; i++) {
		SFileInfo infFile;

		// �t�@�C�����擾
		TCHAR szFileName[_MAX_FNAME];
		DWORD len = *(LPDWORD)&pIndex[0];
		memcpy(szFileName, &pIndex[4], len);
		szFileName[len] = _T('\0');
		pIndex += 4 + len;

		infFile.start = *(LPDWORD)&pIndex[0];
		infFile.key = infFile.start ^ FileEnd_prev;//(i == 0) ? infFile.start : infFile.start ^ FileEnd_prev;//(pclArc->GetFileInfo(i-1)->end - offset);
		infFile.start ^= infFile.key;
		infFile.sizeOrg = *(LPDWORD)&pIndex[4] ^ infFile.key;
		DWORD bCmp = *(LPDWORD)&pIndex[12] ^ infFile.key;
		infFile.sizeCmp = *(LPDWORD)&pIndex[16] ^ infFile.key;
		if (infFile.sizeCmp == 0)
			infFile.sizeCmp = infFile.sizeOrg;
		pIndex += 20;

		// ���X�g�r���[�ɒǉ�
		infFile.name = szFileName;
		infFile.start += offset; // �J�n�A�h���X��0����ɂ��Ďn�܂��Ă���̂ŕ␳
		infFile.end = infFile.start + infFile.sizeCmp;
		if (bCmp) infFile.format = _T("zlib");
		infFile.title = _T("Pak3");
		pclArc->AddFileInfo(infFile);

		FileEnd_prev = infFile.end - offset;
	}

	return TRUE;
}

// �@�_���ăf�����x�C����*.pak�̃t�@�C�������擾����֐�
BOOL CNitro::MountPak4(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".pak"))
		return FALSE;
	if (memcmp(pclArc->GetHed(), "\x04\0\0\0", 4) != 0)
		return FALSE;

	// �V�O�l�`���ǂݍ���
	BYTE sig[256];
	pclArc->Seek(4, FILE_BEGIN);
	pclArc->Read(sig, sizeof(sig));

	// �����L�[����
	DWORD key = 0;
	for (int i = 0; i < 256; i++) {
		if (sig[i] == 0)
			break;
		key *= 0x89;
		key += sig[i];
	}

	// �w�b�_�ǂݍ���
	BYTE header[16];
	pclArc->Read(header, sizeof(header));

	// ���k�C���f�b�N�X�T�C�Y�擾
	DWORD index_compsize = *(LPDWORD)&header[0] ^ *(LPDWORD)&header[12];

	// �C���f�b�N�X�T�C�Y�擾
	DWORD index_size = *(LPDWORD)&header[4] ^ key;

	// �t�@�C�����擾
	DWORD ctFile = *(LPDWORD)&header[8] ^ key;

	// �o�b�t�@�m��
	YCMemory<BYTE> z_index(index_compsize);
	YCMemory<BYTE> index(index_size);
	LPBYTE pIndex = &index[0];

	// ���k�C���f�b�N�X�擾
	pclArc->Read(&z_index[0], index_compsize);

	// �C���f�b�N�X�擾
	CZlib zlib;
	zlib.Decompress(pIndex, &index_size, &z_index[0], index_compsize);

	DWORD offset = 0x114 + index_compsize;
	DWORD FileEnd_prev = 0;

	for (int i = 0; i < (int)ctFile; i++) {
		SFileInfo infFile;

		// �t�@�C�����擾
		TCHAR szFileName[_MAX_FNAME];
		DWORD len = *(LPDWORD)&pIndex[0];
		memcpy(szFileName, &pIndex[4], len);
		szFileName[len] = _T('\0');
		pIndex += 4 + len;

		infFile.start = *(LPDWORD)&pIndex[0];
		infFile.key = infFile.start ^ FileEnd_prev;//(i == 0) ? infFile.start : infFile.start ^ FileEnd_prev;//(pclArc->GetFileInfo(i-1)->end - offset);
		infFile.start ^= infFile.key;
		infFile.sizeOrg = *(LPDWORD)&pIndex[4] ^ infFile.key;
		DWORD bCmp = *(LPDWORD)&pIndex[12] ^ infFile.key;
		infFile.sizeCmp = *(LPDWORD)&pIndex[16] ^ infFile.key;
		if (infFile.sizeCmp == 0)
			infFile.sizeCmp = infFile.sizeOrg;
		pIndex += 20;

		// ���X�g�r���[�ɒǉ�
		infFile.name = szFileName;
		infFile.start += offset; // �J�n�A�h���X��0����ɂ��Ďn�܂��Ă���̂ŕ␳
		infFile.end = infFile.start + infFile.sizeCmp;
		if (bCmp) infFile.format = _T("zlib");
		infFile.title = _T("Pak4");
		pclArc->AddFileInfo(infFile);

		FileEnd_prev = infFile.end - offset;
	}

	return TRUE;
}

BOOL CNitro::MountPK2(CArcFile* pclArc)
{
	if ((pclArc->GetArcExten() != _T(".PK2")) || (memcmp(pclArc->GetHed(), "ARCV", 4) != 0))
		return FALSE;

	pclArc->Seek(4, FILE_BEGIN);

	// �t�@�C�������k�C���f�b�N�X�T�C�Y�擾
	DWORD dwNameIndexSizeCmp;
	pclArc->Read(&dwNameIndexSizeCmp, 4);

	// �t�@�C�����C���f�b�N�X�T�C�Y�擾
	DWORD dwNameIndexSizeOrg;
	pclArc->Read(&dwNameIndexSizeOrg, 4);
	dwNameIndexSizeOrg &= 0x00FFFFFF;

	// �o�b�t�@�m��
	YCMemory<BYTE> vbyNameIndexCmp(dwNameIndexSizeCmp);
	YCMemory<BYTE> vbyNameIndexOrg(dwNameIndexSizeOrg);

	// �t�@�C�������k�C���f�b�N�X�擾
	pclArc->Read(&vbyNameIndexCmp[0], dwNameIndexSizeCmp);

	// �t�@�C�������k�C���f�b�N�X��
	CZlib zlib;
	zlib.Decompress(&vbyNameIndexOrg[0], dwNameIndexSizeOrg, &vbyNameIndexCmp[0], dwNameIndexSizeCmp);

	// �I�t�Z�b�g�C���f�b�N�X�T�C�Y�擾
	DWORD dwOffsetIndexSize;
	pclArc->Read(&dwOffsetIndexSize, 4);
	pclArc->Seek(8, FILE_CURRENT);

	// �o�b�t�@�m��
	YCMemory<BYTE> vbyOffsetIndex(dwOffsetIndexSize);

	// �I�t�Z�b�g�C���f�b�N�X�擾
	pclArc->Read(&vbyOffsetIndex[0], dwOffsetIndexSize);

	LPBYTE pbyNameIndex = &vbyNameIndexOrg[0];
	LPBYTE pbyOffsetIndex = &vbyOffsetIndex[0];

	for (DWORD i = 0; ; i++) {
		// �C���f�b�N�X����t�@�C�����擾
		char szFileName[MAX_PATH];
		char* pszFileName = szFileName;
		while ((*pszFileName++ = *pbyNameIndex++) != 0x0D); // 0x0D���I�[�����ƂȂ��Ă���
		pszFileName[-1] = '\0'; // 0x0D��0x00�ɒu��

		if (i == 0) {
			// ����� "_*.PK2" �Ƃ������O�������Ă��邽�߃X�L�b�v
			continue;
		}

		SFileInfo infFile;
		infFile.name = szFileName;
		infFile.start = *(LPDWORD)&pbyOffsetIndex[4];
		infFile.sizeCmp = *(LPDWORD)&pbyOffsetIndex[8];
		infFile.sizeOrg = infFile.sizeCmp;
		infFile.end = infFile.start + infFile.sizeCmp;
		pclArc->AddFileInfo(infFile);

		pbyOffsetIndex += 12;

		if (memcmp(&pbyOffsetIndex[4], "DATA", 4) == 0) {
			// ����
			break;
		}
	}

	return TRUE;
}

BOOL CNitro::MountN3Pk(CArcFile* pclArc)
{
	if ((pclArc->GetArcExten() != _T(".pak")) || (memcmp(pclArc->GetHed(), "N3Pk", 4) != 0))
		return FALSE;

	pclArc->Seek(4, FILE_BEGIN);

	// �t�@�C�����擾
	DWORD ctFile;
	pclArc->Read(&ctFile, 4);

	pclArc->Seek(4, FILE_CURRENT);

	// �C���f�b�N�X�T�C�Y�擾
	DWORD dwIndexSize = 152 * ctFile;

	// �C���f�b�N�X�擾
	YCMemory<BYTE> vbyIndex(dwIndexSize);
	pclArc->Read(&vbyIndex[0], dwIndexSize);

	LPBYTE pbyIndex = &vbyIndex[0];

	for (DWORD i = 0; i < ctFile; i++) {
		// �t�@�C�����擾
		YCString sFileName;
		sFileName = (char*)&pbyIndex[22];
		sFileName += _T("/");
		sFileName += (char*)&pbyIndex[86];

		SFileInfo infFile;
		infFile.name = sFileName;
		infFile.start = *(LPDWORD)&pbyIndex[0];
		infFile.sizeCmp = *(LPDWORD)&pbyIndex[4];
		infFile.sizeOrg = *(LPDWORD)&pbyIndex[8];
		infFile.end = infFile.start + infFile.sizeCmp;
		infFile.type = *(LPWORD)&pbyIndex[18];
		infFile.key = pbyIndex[21];
		pclArc->AddFileInfo(infFile);

		pbyIndex += 152;
	}

	return TRUE;
}

BOOL CNitro::MountPck(CArcFile* pclArc)
{
	if ((pclArc->GetArcExten() != _T(".pck")) || (memcmp(&pclArc->GetHed()[12], "\x00\x00\x00\x00", 4) != 0))
		return FALSE;

	// �t�@�C�����擾
	DWORD ctFile;
	pclArc->Read(&ctFile, 4);

	// �I�t�Z�b�g�C���f�b�N�X�T�C�Y�擾
	DWORD dwOffsetIndexSize = 12 * (ctFile - 1);

	// �t�@�C�����C���f�b�N�X�T�C�Y�擾
	DWORD dwNameIndexSize;
	pclArc->Read(&dwNameIndexSize, 4);

	pclArc->Seek(4, FILE_CURRENT);

	// �o�b�t�@�m��
	YCMemory<BYTE> vbyOffsetIndex(dwOffsetIndexSize);
	YCMemory<BYTE> vbyNameIndex(dwNameIndexSize);

	// �I�t�Z�b�g�C���f�b�N�X�擾
	pclArc->Read(&vbyOffsetIndex[0], dwOffsetIndexSize);

	// �t�@�C�����C���f�b�N�X�擾
	pclArc->Read(&vbyNameIndex[0], dwNameIndexSize);

	LPBYTE pbyOffsetIndex = &vbyOffsetIndex[0];
	LPBYTE pbyNameIndex = &vbyNameIndex[0];

	DWORD dwOffset = 12 + dwOffsetIndexSize + dwNameIndexSize;

	for (DWORD i = 0; i < ctFile; i++) {
		// �C���f�b�N�X����t�@�C�����擾
		char szFileName[MAX_PATH];
		char* pszFileName = szFileName;
		while ((*pszFileName++ = *pbyNameIndex++) != 0x0D); // 0x0D���I�[�����ƂȂ��Ă���
		pszFileName[-1] = '\0'; // 0x0D��0x00�ɒu��

		if (i == 0) {
			// ����� "*.pck" �Ƃ������O�������Ă��邽�߃X�L�b�v
			continue;
		}

		SFileInfo infFile;
		infFile.name = szFileName;
		infFile.start = *(LPDWORD)&pbyOffsetIndex[0] + dwOffset;
		infFile.sizeCmp = *(LPDWORD)&pbyOffsetIndex[4];
		infFile.sizeOrg = infFile.sizeCmp;
		infFile.end = infFile.start + infFile.sizeCmp;
		pclArc->AddFileInfo(infFile);

		pbyOffsetIndex += 12;
	}

	return TRUE;
}

// �V�g�m������e
BOOL CNitro::MountNpp(CArcFile* pclArc)
{
	if ((pclArc->GetArcExten() != _T(".npp")) || (memcmp(pclArc->GetHed(), "nitP", 4) != 0))
		return FALSE;

	// �t�@�C�����擾
	DWORD ctFile;
	pclArc->Seek(4, FILE_BEGIN);
	pclArc->Read(&ctFile, 4);

	// �t�@�C��������C���f�b�N�X�T�C�Y�擾
	DWORD index_size = 144 * ctFile;

	// �C���f�b�N�X�擾
	YCMemory<BYTE> index(index_size);
	LPBYTE pIndex = &index[0];
	pclArc->Read(pIndex, index_size);

	for (int i = 0; i < (int)ctFile; i++) {
		// �C���f�b�N�X����t�H���_���ƃt�@�C�����擾
		TCHAR szDir[64], szFileName[64];
		memcpy(szDir, &pIndex[16], 64);
		memcpy(szFileName, &pIndex[80], 64);

		TCHAR szFilePath[MAX_PATH];
		lstrcpy(szFilePath, szDir);
		PathAppend(szFilePath, szFileName);

		// ���X�g�r���[�ɒǉ�
		SFileInfo infFile;
		infFile.name = szFilePath;
		infFile.start = *(LPDWORD)&pIndex[0];
		infFile.sizeCmp = *(LPDWORD)&pIndex[4];
		infFile.sizeOrg = *(LPDWORD)&pIndex[8];
		infFile.end = infFile.start + infFile.sizeCmp;
		if (pIndex[12] != 0) infFile.format = _T("LZ");
		pclArc->AddFileInfo(infFile);

		pIndex += 144;
	}

	return TRUE;
}

BOOL CNitro::MountNpa(CArcFile* pclArc)
{
	if ((pclArc->GetArcExten() != _T(".npa")) || (memcmp(pclArc->GetHed(), "NPA\x01", 4) != 0))
		return FALSE;

	pclArc->Seek(25, FILE_BEGIN);

	// �t�@�C�����擾
	DWORD ctFile;
	pclArc->Read(&ctFile, 4);

	pclArc->Seek(8, FILE_CURRENT);

	// �C���f�b�N�X�T�C�Y�擾
	DWORD dwIndexSize;
	pclArc->Read(&dwIndexSize, 4);

	// �C���f�b�N�X�擾
	YCMemory<BYTE> vbyIndex(dwIndexSize);
	pclArc->Read(&vbyIndex[0], dwIndexSize);

	LPBYTE pbyIndex = &vbyIndex[0];
	DWORD dwOffset = 41 + dwIndexSize;
	DWORD dwBaseKey = 0x87654321;

	for (DWORD i = 0; i < ctFile; ) {
		// �t�@�C�����擾
		char szFileName[MAX_PATH];
		DWORD dwFileNameLen = *(LPDWORD)&pbyIndex[0];
		memcpy(szFileName, &pbyIndex[4], dwFileNameLen);
		szFileName[dwFileNameLen] = '\0';
		pbyIndex += 4 + dwFileNameLen;

		// �t�@�C�����擾(�̌���)
		//char szFileName[MAX_PATH];
		//char* pszFileName = szFileName;
		//while ((*pszFileName++ = *pbyIndex++) != '\0');

		if (pbyIndex[0] == 1) {
			// �t�H���_
			pbyIndex += 17;
			continue;
		}

		// �L�[�擾
		DWORD dwKey = dwBaseKey;
		DWORD dwKeyLen = strlen(szFileName);

		for (DWORD j = 0; j < dwKeyLen; j++)
			dwKey -= (BYTE)szFileName[j];
		dwKey *= dwKeyLen;

		SFileInfo infFile;
		infFile.name = szFileName;
		infFile.start = *(LPDWORD)&pbyIndex[5] + dwOffset;
		infFile.sizeCmp = *(LPDWORD)&pbyIndex[9];
		infFile.sizeOrg = *(LPDWORD)&pbyIndex[13];
		infFile.end = infFile.start + infFile.sizeCmp;
		if (infFile.sizeCmp < infFile.sizeOrg) infFile.format = _T("zlib");
		infFile.key = dwKey;
		pclArc->AddFileInfo(infFile);

		pbyIndex += 17;
		i++;
	}

	return TRUE;
}

BOOL CNitro::Decode(CArcFile* pclArc)
{
	if (DecodePak1(pclArc) == TRUE)
		return TRUE;
	if (DecodePak3(pclArc) == TRUE)
		return TRUE;
	if (DecodePak4(pclArc) == TRUE)
		return TRUE;
	if (DecodePK2(pclArc) == TRUE)
		return TRUE;
	if (DecodeN3Pk(pclArc) == TRUE)
		return TRUE;
//	if (DecodeNpa(pclArc) == TRUE)
//		return TRUE;
	return FALSE;
}

BOOL CNitro::DecodePak1(CArcFile* pclArc)
{
	SFileInfo* pInfFile = pclArc->GetOpenFileInfo();

	if (pInfFile->title != _T("Pak1") && pInfFile->title != _T("Pak2"))
		return FALSE;
	if (lstrcmpi(PathFindExtension(pInfFile->name), _T(".nps")) != 0)
		return FALSE;

	// �o�̓t�@�C������
	pclArc->OpenScriptFile();

	if (pInfFile->format == _T("zlib")) {
		// zlib���k�f�[�^

		// �o�b�t�@�m��
		YCMemory<BYTE> z_buf(pInfFile->sizeCmp);
		YCMemory<BYTE> buf(pInfFile->sizeOrg);

		// �ǂݍ���
		pclArc->Read(&z_buf[0], pInfFile->sizeCmp);

		// ��
		CZlib zlib;
		zlib.Decompress(&buf[0], &pInfFile->sizeOrg, &z_buf[0], pInfFile->sizeCmp);

		// �o��
		pclArc->WriteFile(&buf[0], pInfFile->sizeOrg);
	}
	else {
		// �����k�f�[�^
		pclArc->ReadWrite();
	}

	return TRUE;
}

BOOL CNitro::DecodePak3(CArcFile* pclArc)
{
	SFileInfo* pInfFile = pclArc->GetOpenFileInfo();

	if (pInfFile->title != _T("Pak3"))
		return FALSE;

	if (pInfFile->format == _T("BMP")) {
		// �ǂݍ���
		YCMemory<BYTE> buf(pInfFile->sizeCmp);
		pclArc->Read(&buf[0], pInfFile->sizeCmp);
		DecryptPak3(&buf[0], pInfFile->sizeCmp, 0, pInfFile);

		CImage image;
		image.Init(pclArc, &buf[0]);
		image.Write(pInfFile->sizeCmp);
	}
	else {
		// �o�̓t�@�C������
		if (lstrcmpi(PathFindExtension(pInfFile->name), _T(".nps")) == 0)
			pclArc->OpenScriptFile();
		else
			pclArc->OpenFile();

		// zlib���k�f�[�^
		if (pInfFile->format == _T("zlib")) {
			// �o�b�t�@�m��
			YCMemory<BYTE> z_buf(pInfFile->sizeCmp);
			YCMemory<BYTE> buf(pInfFile->sizeOrg);

			// �ǂݍ���
			pclArc->Read(&z_buf[0], pInfFile->sizeCmp);

			// ��
			CZlib zlib;
			zlib.Decompress(&buf[0], &pInfFile->sizeOrg, &z_buf[0], pInfFile->sizeCmp);

			// �o��
			DecryptPak3(&buf[0], pInfFile->sizeOrg, 0, pInfFile);
			pclArc->WriteFile(&buf[0], pInfFile->sizeOrg);
		}
		else {
			// �o�b�t�@�m��
			DWORD BufSize = pclArc->GetBufSize();
			YCMemory<BYTE> buf(BufSize);

			for (DWORD WriteSize = 0; WriteSize != pInfFile->sizeOrg; WriteSize += BufSize) {
				// �o�b�t�@����
				pclArc->SetBufSize(&BufSize, WriteSize);

				// �o��
				pclArc->Read(&buf[0], BufSize);
				DecryptPak3(&buf[0], BufSize, WriteSize, pInfFile);
				pclArc->WriteFile(&buf[0], BufSize);
			}
		}
	}

	return TRUE;
}

BOOL CNitro::DecodePak4(CArcFile* pclArc)
{
	SFileInfo* pInfFile = pclArc->GetOpenFileInfo();

	if (pInfFile->title != _T("Pak4"))
		return FALSE;

	if (pInfFile->format == _T("BMP")) {
		// �ǂݍ���
		YCMemory<BYTE> buf(pInfFile->sizeCmp);
		pclArc->Read(&buf[0], pInfFile->sizeCmp);
		DecryptPak4(&buf[0], pInfFile->sizeCmp, 0, pInfFile);

		CImage image;
		image.Init(pclArc, &buf[0]);
		image.Write(pInfFile->sizeCmp);
	}
	else {
		// �o�̓t�@�C������
		if (lstrcmpi(PathFindExtension(pInfFile->name), _T(".nps")) == 0)
			pclArc->OpenScriptFile();
		else
			pclArc->OpenFile();

		// zlib���k�f�[�^
		if (pInfFile->format == _T("zlib")) {
			// �o�b�t�@�m��
			YCMemory<BYTE> z_buf(pInfFile->sizeCmp);
			YCMemory<BYTE> buf(pInfFile->sizeOrg);

			// �ǂݍ���
			pclArc->Read(&z_buf[0], pInfFile->sizeCmp);

			// ��
			CZlib zlib;
			zlib.Decompress(&buf[0], &pInfFile->sizeOrg, &z_buf[0], pInfFile->sizeCmp);

			// �o��
			DecryptPak4(&buf[0], pInfFile->sizeOrg, 0, pInfFile);
			pclArc->WriteFile(&buf[0], pInfFile->sizeOrg);
		}
		else {
			// �o�b�t�@�m��
			DWORD BufSize = pclArc->GetBufSize();
			YCMemory<BYTE> buf(BufSize);

			for (DWORD WriteSize = 0; WriteSize != pInfFile->sizeOrg; WriteSize += BufSize) {
				// �o�b�t�@����
				pclArc->SetBufSize(&BufSize, WriteSize);

				// �o��
				pclArc->Read(&buf[0], BufSize);
				DecryptPak4(&buf[0], BufSize, WriteSize, pInfFile);
				pclArc->WriteFile(&buf[0], BufSize);
			}
		}
	}

	return TRUE;
}

BOOL CNitro::DecodePK2(CArcFile* pclArc)
{
	if ((pclArc->GetArcExten() != _T(".PK2")) || (memcmp(pclArc->GetHed(), "ARCV", 4) != 0))
		return FALSE;

	SFileInfo* pInfFile = pclArc->GetOpenFileInfo();
	YCString sFileExt = PathFindExtension(pInfFile->name);

	if (sFileExt != _T(".nps") && sFileExt != _T(".ini") && sFileExt != _T(".h") && sFileExt != _T(".txt"))
		return FALSE;

	// �t�@�C���T�C�Y�擾
	DWORD dwSizeOrg;
	pclArc->Read(&dwSizeOrg, 4);
	dwSizeOrg &= 0x00FFFFFF;

	// �o�b�t�@�m��
	YCMemory<BYTE> vbyBufCmp(pInfFile->sizeCmp);
	YCMemory<BYTE> vbyBufOrg(dwSizeOrg);

	// �ǂݍ���
	pclArc->Read(&vbyBufCmp[0], pInfFile->sizeCmp);

	// ��
	CZlib zlib;
	zlib.Decompress(&vbyBufOrg[0], dwSizeOrg, &vbyBufCmp[0], pInfFile->sizeCmp);

	if (sFileExt == _T(".nps")) {
		// �g���q��txt�ɕϊ�
		pclArc->OpenScriptFile();
	}
	else
		pclArc->OpenFile();

	pclArc->WriteFile(&vbyBufOrg[0], dwSizeOrg);

	return TRUE;
}

BOOL CNitro::DecodeN3Pk(CArcFile* pclArc)
{
	if ((pclArc->GetArcExten() != _T(".pak")) || (memcmp(pclArc->GetHed(), "N3Pk", 4) != 0))
		return FALSE;

	SFileInfo* pInfFile = pclArc->GetOpenFileInfo();
	YCString sFileExt = PathFindExtension(pInfFile->name);

	// �o�b�t�@�m��
	DWORD dwBufSize = pclArc->GetBufSize();
	YCMemory<BYTE> vbyBuf(dwBufSize);

	BYTE byKey = (BYTE)pInfFile->key;

	if (sFileExt == _T(".nps"))
		pclArc->OpenScriptFile();
	else
		pclArc->OpenFile();

	for (DWORD dwWriteSize = 0; dwWriteSize != pInfFile->sizeOrg; dwWriteSize += dwBufSize) {
		// �o�b�t�@����
		pclArc->SetBufSize(&dwBufSize, dwWriteSize);

		// �o��
		pclArc->Read(&vbyBuf[0], dwBufSize);
		DecryptN3Pk(&vbyBuf[0], dwBufSize, dwWriteSize, pInfFile, byKey);
		pclArc->WriteFile(&vbyBuf[0], dwBufSize);
	}

	return TRUE;
}

BOOL CNitro::DecodeNpa(CArcFile* pclArc)
{
	if ((pclArc->GetArcExten() != _T(".npa")) || (memcmp(pclArc->GetHed(), "NPA\x01", 4) != 0))
		return FALSE;

	SFileInfo* pInfFile = pclArc->GetOpenFileInfo();
	YCString sFileExt = PathFindExtension(pInfFile->name);

	if (sFileExt == _T(".nss"))
		pclArc->OpenScriptFile();
	else
		pclArc->OpenFile();

	if (pInfFile->format == _T("zlib")) {
		// �o�b�t�@�m��
		YCMemory<BYTE> vbyBufCmp(pInfFile->sizeCmp);
		YCMemory<BYTE> vbyBufOrg(pInfFile->sizeOrg);

		// �ǂݍ���
		pclArc->Read(&vbyBufCmp[0], pInfFile->sizeCmp);

		// ����
		DecryptNpa(&vbyBufCmp[0], pInfFile->sizeCmp, 0, pInfFile);

		// ��
		CZlib zlib;
		zlib.Decompress(&vbyBufOrg[0], pInfFile->sizeOrg, &vbyBufCmp[0], pInfFile->sizeCmp);

		// �o��
		pclArc->WriteFile(&vbyBufOrg[0], pInfFile->sizeOrg);
	}
	else {
		// �o�b�t�@�m��
		DWORD dwBufSize = pclArc->GetBufSize();
		YCMemory<BYTE> vbyBuf(dwBufSize);

		for (DWORD dwWriteSize = 0; dwWriteSize != pInfFile->sizeOrg; dwWriteSize += dwBufSize) {
			// �o�b�t�@����
			pclArc->SetBufSize(&dwBufSize, dwWriteSize);

			// �o��
			pclArc->Read(&vbyBuf[0], dwBufSize);
			DecryptNpa(&vbyBuf[0], dwBufSize, dwWriteSize, pInfFile);
			pclArc->WriteFile(&vbyBuf[0], dwBufSize);
		}
	}

	return TRUE;
}

void CNitro::DecryptPak3(LPBYTE data, DWORD size, DWORD offset, SFileInfo* pInfFile)
{
	YCString			clsFileExt = PathFindExtension( pInfFile->name );

	if( pInfFile->format == _T("zlib") )
	{
		// �������Ȃ�

		return;
	}

	// �ő�Ő擪16�o�C�g�𕜍�����

	size = (size < 16) ? size : 16;

	// ����

	DWORD				dwTargetPtr = 0;

	for( DWORD i = offset ; i < (size / 4) ; i++ )
	{
		*(DWORD*) &data[dwTargetPtr] ^= pInfFile->key;

		dwTargetPtr += 4;
	}

	for( DWORD i = offset ; i < (size & 3) ; i++ )
	{
		data[dwTargetPtr++] ^= (pInfFile->key >> (i * 8)) & 0xFF;
	}
}

void CNitro::DecryptPak4(LPBYTE data, DWORD size, DWORD offset, SFileInfo* pInfFile)
{
	YCString			clsFileExt = PathFindExtension( pInfFile->name );

	if( pInfFile->format != _T("zlib") )
	{
		// �擪����1024�o�C�g�܂ŕ���

		size = (size < 1024) ? size : 1024;
	}

	// ����

	DWORD				dwTargetPtr = 0;

	for( DWORD i = offset ; i < (size / 4) ; i++ )
	{
		*(DWORD*) &data[dwTargetPtr] ^= pInfFile->key;

		dwTargetPtr += 4;
	}

	for( DWORD i = offset ; i < (size & 3) ; i++ )
	{
		data[dwTargetPtr++] ^= (pInfFile->key >> (i * 8)) & 0xFF;
	}
}

void CNitro::DecryptN3Pk(LPBYTE data, DWORD size, DWORD offset, SFileInfo* pInfFile, BYTE& byKey)
{
	static const BYTE	abtKey[1024] =
	{
		0xAA, 0x00, 0x00, 0x00, 0x96, 0x30, 0x07, 0x77, 0x2C, 0x61, 0x0E, 0xEE, 0xBA, 0x51, 0x09, 0x99, 0x19, 0xC4, 0x6D, 0x07, 0x8F, 0xF4, 0x6A, 0x70, 0x35, 0xA5, 0x63, 0xE9, 0xA3, 0x95, 0x64, 0x9E,
		0x32, 0x88, 0xDB, 0x0E, 0xA4, 0xB8, 0xDC, 0x79, 0x1E, 0xE9, 0xD5, 0xE0, 0x88, 0xD9, 0xD2, 0x97, 0x2B, 0x4C, 0xB6, 0x09, 0xBD, 0x7C, 0xB1, 0x7E, 0x07, 0x2D, 0xB8, 0xE7, 0x91, 0x1D, 0xBF, 0x90,
		0x64, 0x10, 0xB7, 0x1D, 0xF2, 0x20, 0xB0, 0x6A, 0x48, 0x71, 0xB9, 0xF3, 0xDE, 0x41, 0xBE, 0x84, 0x7D, 0xD4, 0xDA, 0x1A, 0xEB, 0xE4, 0xDD, 0x6D, 0x51, 0xB5, 0xD4, 0xF4, 0xC7, 0x85, 0xD3, 0x83,
		0x56, 0x98, 0x6C, 0x13, 0xC0, 0xA8, 0x6B, 0x64, 0x7A, 0xF9, 0x62, 0xFD, 0xEC, 0xC9, 0x65, 0x8A, 0x4F, 0x5C, 0x01, 0x14, 0xD9, 0x6C, 0x06, 0x63, 0x63, 0x3D, 0x0F, 0xFA, 0xF5, 0x0D, 0x08, 0x8D,
		0xC8, 0x20, 0x6E, 0x3B, 0x5E, 0x10, 0x69, 0x4C, 0xE4, 0x41, 0x60, 0xD5, 0x72, 0x71, 0x67, 0xA2, 0xD1, 0xE4, 0x03, 0x3C, 0x47, 0xD4, 0x04, 0x4B, 0xFD, 0x85, 0x0D, 0xD2, 0x6B, 0xB5, 0x0A, 0xA5,
		0xFA, 0xA8, 0xB5, 0x35, 0x6C, 0x98, 0xB2, 0x42, 0xD6, 0xC9, 0xBB, 0xDB, 0x40, 0xF9, 0xBC, 0xAC, 0xE3, 0x6C, 0xD8, 0x32, 0x75, 0x5C, 0xDF, 0x45, 0xCF, 0x0D, 0xD6, 0xDC, 0x59, 0x3D, 0xD1, 0xAB,
		0xAC, 0x30, 0xD9, 0x26, 0x3A, 0x00, 0xDE, 0x51, 0x80, 0x51, 0xD7, 0xC8, 0x16, 0x61, 0xD0, 0xBF, 0xB5, 0xF4, 0xB4, 0x21, 0x23, 0xC4, 0xB3, 0x56, 0x99, 0x95, 0xBA, 0xCF, 0x0F, 0xA5, 0xBD, 0xB8,
		0x9E, 0xB8, 0x02, 0x28, 0x08, 0x88, 0x05, 0x5F, 0xB2, 0xD9, 0x0C, 0xC6, 0x24, 0xE9, 0x0B, 0xB1, 0x87, 0x7C, 0x6F, 0x2F, 0x11, 0x4C, 0x68, 0x58, 0xAB, 0x1D, 0x61, 0xC1, 0x3D, 0x2D, 0x66, 0xB6,
		0x90, 0x41, 0xDC, 0x76, 0x06, 0x71, 0xDB, 0x01, 0xBC, 0x20, 0xD2, 0x98, 0x2A, 0x10, 0xD5, 0xEF, 0x89, 0x85, 0xB1, 0x71, 0x1F, 0xB5, 0xB6, 0x06, 0xA5, 0xE4, 0xBF, 0x9F, 0x33, 0xD4, 0xB8, 0xE8,
		0xA2, 0xC9, 0x07, 0x78, 0x34, 0xF9, 0x00, 0x0F, 0x8E, 0xA8, 0x09, 0x96, 0x18, 0x98, 0x0E, 0xE1, 0xBB, 0x0D, 0x6A, 0x7F, 0x2D, 0x3D, 0x6D, 0x08, 0x97, 0x6C, 0x64, 0x91, 0x01, 0x5C, 0x63, 0xE6,
		0xF4, 0x51, 0x6B, 0x6B, 0x62, 0x61, 0x6C, 0x1C, 0xD8, 0x30, 0x65, 0x85, 0x4E, 0x00, 0x62, 0xF2, 0xED, 0x95, 0x06, 0x6C, 0x7B, 0xA5, 0x01, 0x1B, 0xC1, 0xF4, 0x08, 0x82, 0x57, 0xC4, 0x0F, 0xF5,
		0xC6, 0xD9, 0xB0, 0x65, 0x50, 0xE9, 0xB7, 0x12, 0xEA, 0xB8, 0xBE, 0x8B, 0x7C, 0x88, 0xB9, 0xFC, 0xDF, 0x1D, 0xDD, 0x62, 0x49, 0x2D, 0xDA, 0x15, 0xF3, 0x7C, 0xD3, 0x8C, 0x65, 0x4C, 0xD4, 0xFB,
		0x58, 0x61, 0xB2, 0x4D, 0xCE, 0x51, 0xB5, 0x3A, 0x74, 0x00, 0xBC, 0xA3, 0xE2, 0x30, 0xBB, 0xD4, 0x41, 0xA5, 0xDF, 0x4A, 0xD7, 0x95, 0xD8, 0x3D, 0x6D, 0xC4, 0xD1, 0xA4, 0xFB, 0xF4, 0xD6, 0xD3,
		0x6A, 0xE9, 0x69, 0x43, 0xFC, 0xD9, 0x6E, 0x34, 0x46, 0x88, 0x67, 0xAD, 0xD0, 0xB8, 0x60, 0xDA, 0x73, 0x2D, 0x04, 0x44, 0xE5, 0x1D, 0x03, 0x33, 0x5F, 0x4C, 0x0A, 0xAA, 0xC9, 0x7C, 0x0D, 0xDD,
		0x3C, 0x71, 0x05, 0x50, 0xAA, 0x41, 0x02, 0x27, 0x10, 0x10, 0x0B, 0xBE, 0x86, 0x20, 0x0C, 0xC9, 0x25, 0xB5, 0x68, 0x57, 0xB3, 0x85, 0x6F, 0x20, 0x09, 0xD4, 0x66, 0xB9, 0x9F, 0xE4, 0x61, 0xCE,
		0x0E, 0xF9, 0xDE, 0x5E, 0x98, 0xC9, 0xD9, 0x29, 0x22, 0x98, 0xD0, 0xB0, 0xB4, 0xA8, 0xD7, 0xC7, 0x17, 0x3D, 0xB3, 0x59, 0x81, 0x0D, 0xB4, 0x2E, 0x3B, 0x5C, 0xBD, 0xB7, 0xAD, 0x6C, 0xBA, 0xC0,
		0x20, 0x83, 0xB8, 0xED, 0xB6, 0xB3, 0xBF, 0x9A, 0x0C, 0xE2, 0xB6, 0x03, 0x9A, 0xD2, 0xB1, 0x74, 0x39, 0x47, 0xD5, 0xEA, 0xAF, 0x77, 0xD2, 0x9D, 0x15, 0x26, 0xDB, 0x04, 0x83, 0x16, 0xDC, 0x73,
		0x12, 0x0B, 0x63, 0xE3, 0x84, 0x3B, 0x64, 0x94, 0x3E, 0x6A, 0x6D, 0x0D, 0xA8, 0x5A, 0x6A, 0x7A, 0x0B, 0xCF, 0x0E, 0xE4, 0x9D, 0xFF, 0x09, 0x93, 0x27, 0xAE, 0x00, 0x0A, 0xB1, 0x9E, 0x07, 0x7D,
		0x44, 0x93, 0x0F, 0xF0, 0xD2, 0xA3, 0x08, 0x87, 0x68, 0xF2, 0x01, 0x1E, 0xFE, 0xC2, 0x06, 0x69, 0x5D, 0x57, 0x62, 0xF7, 0xCB, 0x67, 0x65, 0x80, 0x71, 0x36, 0x6C, 0x19, 0xE7, 0x06, 0x6B, 0x6E,
		0x76, 0x1B, 0xD4, 0xFE, 0xE0, 0x2B, 0xD3, 0x89, 0x5A, 0x7A, 0xDA, 0x10, 0xCC, 0x4A, 0xDD, 0x67, 0x6F, 0xDF, 0xB9, 0xF9, 0xF9, 0xEF, 0xBE, 0x8E, 0x43, 0xBE, 0xB7, 0x17, 0xD5, 0x8E, 0xB0, 0x60,
		0xE8, 0xA3, 0xD6, 0xD6, 0x7E, 0x93, 0xD1, 0xA1, 0xC4, 0xC2, 0xD8, 0x38, 0x52, 0xF2, 0xDF, 0x4F, 0xF1, 0x67, 0xBB, 0xD1, 0x67, 0x57, 0xBC, 0xA6, 0xDD, 0x06, 0xB5, 0x3F, 0x4B, 0x36, 0xB2, 0x48,
		0xDA, 0x2B, 0x0D, 0xD8, 0x4C, 0x1B, 0x0A, 0xAF, 0xF6, 0x4A, 0x03, 0x36, 0x60, 0x7A, 0x04, 0x41, 0xC3, 0xEF, 0x60, 0xDF, 0x55, 0xDF, 0x67, 0xA8, 0xEF, 0x8E, 0x6E, 0x31, 0x79, 0xBE, 0x69, 0x46,
		0x8C, 0xB3, 0x61, 0xCB, 0x1A, 0x83, 0x66, 0xBC, 0xA0, 0xD2, 0x6F, 0x25, 0x36, 0xE2, 0x68, 0x52, 0x95, 0x77, 0x0C, 0xCC, 0x03, 0x47, 0x0B, 0xBB, 0xB9, 0x16, 0x02, 0x22, 0x2F, 0x26, 0x05, 0x55,
		0xBE, 0x3B, 0xBA, 0xC5, 0x28, 0x0B, 0xBD, 0xB2, 0x92, 0x5A, 0xB4, 0x2B, 0x04, 0x6A, 0xB3, 0x5C, 0xA7, 0xFF, 0xD7, 0xC2, 0x31, 0xCF, 0xD0, 0xB5, 0x8B, 0x9E, 0xD9, 0x2C, 0x1D, 0xAE, 0xDE, 0x5B,
		0xB0, 0xC2, 0x64, 0x9B, 0x26, 0xF2, 0x63, 0xEC, 0x9C, 0xA3, 0x6A, 0x75, 0x0A, 0x93, 0x6D, 0x02, 0xA9, 0x06, 0x09, 0x9C, 0x3F, 0x36, 0x0E, 0xEB, 0x85, 0x67, 0x07, 0x72, 0x13, 0x57, 0x00, 0x05,
		0x82, 0x4A, 0xBF, 0x95, 0x14, 0x7A, 0xB8, 0xE2, 0xAE, 0x2B, 0xB1, 0x7B, 0x38, 0x1B, 0xB6, 0x0C, 0x9B, 0x8E, 0xD2, 0x92, 0x0D, 0xBE, 0xD5, 0xE5, 0xB7, 0xEF, 0xDC, 0x7C, 0x21, 0xDF, 0xDB, 0x0B,
		0xD4, 0xD2, 0xD3, 0x86, 0x42, 0xE2, 0xD4, 0xF1, 0xF8, 0xB3, 0xDD, 0x68, 0x6E, 0x83, 0xDA, 0x1F, 0xCD, 0x16, 0xBE, 0x81, 0x5B, 0x26, 0xB9, 0xF6, 0xE1, 0x77, 0xB0, 0x6F, 0x77, 0x47, 0xB7, 0x18,
		0xE6, 0x5A, 0x08, 0x88, 0x70, 0x6A, 0x0F, 0xFF, 0xCA, 0x3B, 0x06, 0x66, 0x5C, 0x0B, 0x01, 0x11, 0xFF, 0x9E, 0x65, 0x8F, 0x69, 0xAE, 0x62, 0xF8, 0xD3, 0xFF, 0x6B, 0x61, 0x45, 0xCF, 0x6C, 0x16,
		0x78, 0xE2, 0x0A, 0xA0, 0xEE, 0xD2, 0x0D, 0xD7, 0x54, 0x83, 0x04, 0x4E, 0xC2, 0xB3, 0x03, 0x39, 0x61, 0x26, 0x67, 0xA7, 0xF7, 0x16, 0x60, 0xD0, 0x4D, 0x47, 0x69, 0x49, 0xDB, 0x77, 0x6E, 0x3E,
		0x4A, 0x6A, 0xD1, 0xAE, 0xDC, 0x5A, 0xD6, 0xD9, 0x66, 0x0B, 0xDF, 0x40, 0xF0, 0x3B, 0xD8, 0x37, 0x53, 0xAE, 0xBC, 0xA9, 0xC5, 0x9E, 0xBB, 0xDE, 0x7F, 0xCF, 0xB2, 0x47, 0xE9, 0xFF, 0xB5, 0x30,
		0x1C, 0xF2, 0xBD, 0xBD, 0x8A, 0xC2, 0xBA, 0xCA, 0x30, 0x93, 0xB3, 0x53, 0xA6, 0xA3, 0xB4, 0x24, 0x05, 0x36, 0xD0, 0xBA, 0x93, 0x06, 0xD7, 0xCD, 0x29, 0x57, 0xDE, 0x54, 0xBF, 0x67, 0xD9, 0x23,
		0x2E, 0x7A, 0x66, 0xB3, 0xB8, 0x4A, 0x61, 0xC4, 0x02, 0x1B, 0x68, 0x5D, 0x94, 0x2B, 0x6F, 0x2A, 0x37, 0xBE, 0x0B, 0xB4, 0xA1, 0x8E, 0x0C, 0xC3, 0x1B, 0xDF, 0x05, 0x5A, 0x8D, 0xEF, 0x02, 0x2D
	};

	switch( pInfFile->type )
	{
	case	1:
		// �S�T�C�Y����

		for( DWORD i = 0 ; i < size ; i++ )
		{
			data[i] ^= abtKey[byKey++];
		}

		break;

	case	2:
		// �擪����1024�o�C�g�܂ŕ���

		size = (size < 1024) ? size : 1024;

		for( DWORD i = offset, j = 0 ; i < size ; i++, j++ )
		{
			data[j] ^= abtKey[byKey++];
		}

		break;
	}
}

void CNitro::DecryptNpa(LPBYTE data, DWORD size, DWORD offset, SFileInfo* pInfFile)
{
	static const BYTE	abtKey[256] =
	{
		0xDF, 0x5F, 0x6E, 0xF7, 0xF5, 0xEF, 0x52, 0x5B, 0x7E, 0x25, 0xD7, 0x46, 0xBC, 0x92, 0x02, 0x2E, 0x51, 0x7C, 0x39, 0x16, 0x2A, 0x18, 0x08, 0xEB, 0x0C, 0x97, 0x3A, 0xC7, 0xAC, 0xC6, 0xB0, 0x17,
		0x80, 0xD6, 0x86, 0x3C, 0xFB, 0xF9, 0xB1, 0x01, 0xA9, 0x79, 0x9E, 0xB3, 0x37, 0xDE, 0x19, 0xE7, 0x2B, 0xC2, 0x28, 0x1E, 0x5D, 0x67, 0x22, 0x8E, 0x58, 0x1A, 0xCC, 0xEC, 0x44, 0x9D, 0xA7, 0x24,
		0x55, 0x0F, 0x64, 0x56, 0x4C, 0x6C, 0xC4, 0x77, 0x11, 0x09, 0xEA, 0xC8, 0x20, 0x63, 0x1C, 0x6D, 0x3D, 0x72, 0xB8, 0x8A, 0x54, 0x95, 0x3F, 0x98, 0xE9, 0xB2, 0x7F, 0x06, 0x50, 0x8C, 0xC5, 0xB4,
		0x9B, 0x31, 0x81, 0x2D, 0x7B, 0xA3, 0x42, 0x3B, 0xF6, 0x74, 0xE0, 0x83, 0xA0, 0x41, 0xD2, 0xEE, 0x49, 0xC9, 0x57, 0x73, 0x61, 0x85, 0x6A, 0x78, 0x89, 0x68, 0x2C, 0xB5, 0xB9, 0xA8, 0x40, 0x05,
		0xE8, 0xBF, 0xBB, 0x26, 0xA4, 0x66, 0x94, 0x87, 0x2F, 0xFE, 0xDC, 0x33, 0xCE, 0xB7, 0xE4, 0x07, 0x43, 0x48, 0xCB, 0xC3, 0x76, 0x30, 0xDD, 0x6B, 0x21, 0xD4, 0x82, 0xAD, 0x84, 0xD0, 0x75, 0xF1,
		0xF0, 0xE2, 0xDB, 0x03, 0x6F, 0x65, 0xCD, 0x0D, 0x4A, 0x62, 0x0A, 0x88, 0x8F, 0x8D, 0x14, 0xAB, 0x70, 0xED, 0x0B, 0x45, 0x23, 0xC0, 0xAA, 0x91, 0x0E, 0x3E, 0x29, 0xD1, 0x59, 0xFD, 0xFA, 0xE1,
		0xA2, 0xAF, 0xF4, 0x4F, 0x4D, 0xA5, 0x9C, 0xE3, 0x8B, 0x00, 0x12, 0x90, 0x32, 0x15, 0xD5, 0xBE, 0xD8, 0x4E, 0x69, 0xF2, 0xE6, 0x9A, 0xCA, 0x99, 0x04, 0x7A, 0x35, 0x10, 0x38, 0x1D, 0x13, 0x4B,
		0xA6, 0xA1, 0x47, 0x5A, 0x1F, 0x36, 0xC1, 0x53, 0xB6, 0xCF, 0xAE, 0x7D, 0xFF, 0x93, 0x71, 0x34, 0xD3, 0xFC, 0x9F, 0xF8, 0x5E, 0x1B, 0xD9, 0x60, 0xBA, 0xBD, 0x5C, 0xE5, 0xF3, 0x27, 0xDA, 0x96
	};

	BYTE				btKey = (BYTE) (pInfFile->key & 0xFF);

	// �擪����4096�o�C�g�܂ŕ���

	size = (size < 4096) ? size : 4096;

	// ����

	for( DWORD i = offset, j = 0 ; i < size ; i++, j++ )
	{
		data[j] = abtKey[data[j]] - btKey;
	}
}
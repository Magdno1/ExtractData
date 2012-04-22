
#include	"stdafx.h"
#include	"../ExtractBase.h"
#include	"../Image.h"
#include	"Ethornell.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	�}�E���g

BOOL	CEthornell::Mount(
	CArcFile*			pclArc							// �A�[�J�C�u
	)
{
	if( pclArc->GetArcExten() != _T(".arc") )
	{
		return	FALSE;
	}

	if( memcmp( pclArc->GetHed(), "PackFile    ", 12 ) != 0 )
	{
		return	FALSE;
	}

	pclArc->SeekHed( 12 );

	// �t�@�C�����擾

	DWORD				dwFiles;

	pclArc->Read( &dwFiles, 4 );

	// �C���f�b�N�X�T�C�Y�擾

	DWORD				dwIndexSize = (32 * dwFiles);

	// �C���f�b�N�X�擾

	YCMemory<BYTE>		clmIndex( dwIndexSize );

	pclArc->Read( &clmIndex[0], dwIndexSize );

	// �I�t�Z�b�g�擾

	DWORD				dwOffset = 16 + dwIndexSize;

	// �t�@�C�����̎擾

	for( DWORD i = 0 ; i < dwIndexSize ; i += 32 )
	{
		SFileInfo			stFileInfo;

		stFileInfo.name.Copy( (char*) &clmIndex[i], 16 );
		stFileInfo.start = *(DWORD*) &clmIndex[i + 16] + dwOffset;
		stFileInfo.sizeCmp = *(DWORD*) &clmIndex[i + 20];
		stFileInfo.sizeOrg = stFileInfo.sizeCmp;
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;

		pclArc->AddFileInfo( stFileInfo );
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	�f�R�[�h

BOOL	CEthornell::Decode(
	CArcFile*			pclArc							// �A�[�J�C�u
	)
{
	BYTE				abtHeader[16];

	pclArc->Read( abtHeader, sizeof(abtHeader) );
	pclArc->SeekCur( -(int)sizeof(abtHeader) );

	if( memcmp( abtHeader, "DSC FORMAT 1.00\0", 16 ) == 0 )
	{
		// DSC

		return	DecodeDSC( pclArc );
	}
	else if( memcmp( abtHeader, "CompressedBG___\0", 16 ) == 0 )
	{
		// CompressedBG

		return	DecodeCBG( pclArc );
	}

	// ���̑�

	return	DecodeStd( pclArc );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	DSC�̃f�R�[�h

BOOL	CEthornell::DecodeDSC(
	CArcFile*			pclArc							// �A�[�J�C�u
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	// �ǂݍ���

	DWORD				dwSrcSize = pstFileInfo->sizeCmp;

	YCMemory<BYTE>		clmbtSrc( dwSrcSize );

	pclArc->Read( &clmbtSrc[0], dwSrcSize );

	// �o�͗p�o�b�t�@�m��

	DWORD				dwDstSize = *(DWORD*) &clmbtSrc[20];

	YCMemory<BYTE>		clmbtDst( dwDstSize );

	// DSC��

	DecompDSC( &clmbtDst[0], dwDstSize, &clmbtSrc[0], dwSrcSize );

	// �摜���擾

	long				lWidth = *(WORD*) &clmbtDst[0];
	long				lHeight = *(WORD*) &clmbtDst[2];
	WORD				wBpp = *(WORD*) &clmbtDst[4];
	WORD				wFlags = *(WORD*) &clmbtDst[6];

	DWORD				dwOffset = *(DWORD*) &clmbtDst[0];

	// �o��

	if( ((wBpp == 8) || (wBpp == 24) || (wBpp == 32)) && (memcmp( &clmbtDst[8], "\0\0\0\0\0\0\0\0", 8 ) == 0) )
	{
		// �摜

		CImage				clImage;
		YCMemory<BYTE>		clmbtDst2;
		DWORD				dwDstSize2;

		switch( wFlags )
		{
		case	0:
			// �ʏ�

			clImage.Init( pclArc, lWidth, lHeight, wBpp );
			clImage.WriteReverse( &clmbtDst[16], (dwDstSize - 16) );

			break;

		case	1:
			// �Í��^�C�v1

			dwDstSize2 = (dwDstSize - 16);

			clmbtDst2.resize( dwDstSize2 );

			// ������

			DecryptBGType1( &clmbtDst2[0], &clmbtDst[16], lWidth, lHeight, wBpp );

			// �o��

			clImage.Init( pclArc, lWidth, lHeight, wBpp );
			clImage.WriteReverse( &clmbtDst2[0], dwDstSize2 );

			break;

		default:
			// ���m�̌`��

			pclArc->OpenFile();
			pclArc->WriteFile( &clmbtDst[0], dwDstSize, dwSrcSize );
		}
	}
	else if( (dwOffset < (dwDstSize - 4)) && (memcmp( &clmbtDst[dwOffset], "OggS", 4 ) == 0) )
	{
		// Ogg Vorbis

		pclArc->OpenFile( _T(".ogg") );
		pclArc->WriteFile( &clmbtDst[dwOffset], (dwDstSize - dwOffset), dwSrcSize );
	}
	else
	{
		// ���̑�

		pclArc->OpenFile();
		pclArc->WriteFile( &clmbtDst[0], dwDstSize, dwSrcSize );
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	CompressedBG�̃f�R�[�h

BOOL	CEthornell::DecodeCBG(
	CArcFile*			pclArc							// �A�[�J�C�u
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	// CompressedBG�ǂݍ���

	DWORD				dwSrcSize = pstFileInfo->sizeCmp;

	YCMemory<BYTE>		clmbtSrc( dwSrcSize );

	pclArc->Read( &clmbtSrc[0], dwSrcSize );

	// ���A�����A�F���擾

	long				lWidth = *(WORD*) &clmbtSrc[16];
	long				lHeight = *(WORD*) &clmbtSrc[18];
	WORD				wBpp = *(WORD*) &clmbtSrc[20];

	// �o�͗p�o�b�t�@�m��

	DWORD				dwDstSize = lWidth * lHeight * (wBpp >> 3);

	YCMemory<BYTE>		clmbtDst( dwDstSize );

	// CompressedBG��

	DecompCBG( &clmbtDst[0], &clmbtSrc[0] );

	// �摜�o��

	CImage				clImage;

	clImage.Init( pclArc, lWidth, lHeight, wBpp );
	clImage.WriteReverse( &clmbtDst[0], dwDstSize );

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	���̑��t�@�C���̃f�R�[�h

BOOL	CEthornell::DecodeStd(
	CArcFile*			pclArc							// �A�[�J�C�u
	)
{
	// �I�t�Z�b�g�擾

	DWORD				dwOffset;

	pclArc->Read( &dwOffset, 4 );

	// �t�@�C���w�b�_�`�F�b�N

	BYTE				abtHeader[4];

	if( (pclArc->GetArcPointer() + dwOffset) < pclArc->GetArcSize() )
	{
		// �t�@�C���V�[�N�\�ȃI�t�Z�b�g�l

		pclArc->SeekCur( dwOffset - 4 );
		pclArc->Read( abtHeader, sizeof(abtHeader) );
	}
	else
	{
		// �t�@�C���V�[�N�s�\�ȃI�t�Z�b�g�l(�I�t�Z�b�g�l�ł͂Ȃ��ƒf��)

		dwOffset = 0;

		ZeroMemory( abtHeader, sizeof(abtHeader) );
	}

	// �o��

	if( memcmp( abtHeader, "OggS", 4 ) == 0 )
	{
		// Ogg Vorbis

		pclArc->SeekHed( pclArc->GetOpenFileInfo()->start + dwOffset );

		pclArc->OpenFile( _T(".ogg") );
	}
	else
	{
		// ���̑�

		dwOffset = 0;

		pclArc->SeekHed( pclArc->GetOpenFileInfo()->start );

		pclArc->OpenFile();
	}

	pclArc->ReadWrite( pclArc->GetOpenFileInfo()->sizeOrg - dwOffset );

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	�L�[�̎擾

DWORD	CEthornell::GetKey(
	DWORD*				pdwKey							// �������L�[
	)
{
	DWORD				dwWork1 = 20021 * (*pdwKey & 0xFFFF);
	DWORD				dwWork2 = 20021 * (*pdwKey >> 16);
	DWORD				dwWork  = 346 * (*pdwKey) + dwWork2 + (dwWork1 >> 16);

	*pdwKey = (dwWork << 16) + (dwWork1 & 0xFFFF) + 1;

	return	(dwWork & 0x7FFF);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	�ϒ��f�[�^�̎擾

DWORD	CEthornell::GetVariableData(
	const BYTE*			pbtSrc,							// ���̓f�[�^
	DWORD*				pdwDstOfReadLength				// �ǂݍ��񂾒����̊i�[��
	)
{
	DWORD				dwData = 0;
	DWORD				dwSrcPtr = 0;
	DWORD				dwShift = 0;
	BYTE				btCurrentSrc;

	do
	{
		btCurrentSrc = pbtSrc[dwSrcPtr++];

		dwData |= (btCurrentSrc & 0x7F) << dwShift;

		dwShift += 7;
	}
	while( btCurrentSrc & 0x80 );

	if( pdwDstOfReadLength != NULL )
	{
		*pdwDstOfReadLength = dwSrcPtr;
	}

	return	dwData;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	DSC�̉�

void	CEthornell::DecompDSC(
	BYTE*				pbtDst,							// �i�[��
	DWORD				dwDstSize,						// �i�[��T�C�Y
	const BYTE*			pbtSrc,							// ���k�f�[�^
	DWORD				dwSrcSize						// ���k�f�[�^�T�C�Y
	)
{
	DWORD				dwSrcPtr = 32;
	DWORD				dwDstPtr = 0;

	DWORD				adwBuffer[512 + 1];
	DWORD				adwBuffer2[1024];
	BYTE				abtBuffer3[0x3FF0];

	DWORD				dwCount;
	DWORD				dwWork;
	BYTE				btWork;

	ZeroMemory( adwBuffer, sizeof(adwBuffer) );
	ZeroMemory( adwBuffer2, sizeof(adwBuffer2) );
	ZeroMemory( abtBuffer3, sizeof(abtBuffer3) );

	// 

	DWORD				dwKey = *(DWORD*) &pbtSrc[16];
	DWORD				dwBufferSize = 0;

	for( DWORD i = 0 ; i < 512 ; i++ )
	{
		btWork = pbtSrc[dwSrcPtr] - (BYTE) GetKey( &dwKey );

		if( btWork != 0 )
		{
			adwBuffer[dwBufferSize++] = (btWork << 16) + i;
		}

		dwSrcPtr++;
	}

	adwBuffer[dwBufferSize] = 0;

	// �\�[�g

	for( DWORD i = 0 ; i < (dwBufferSize - 1) ; i++ )
	{
		for( DWORD j = (i + 1) ; j < dwBufferSize ; j++ )
		{
			if( adwBuffer[i] > adwBuffer[j] )
			{
				std::swap( adwBuffer[i], adwBuffer[j] );
			}
		}
	}

	// 

	int					nMin = 0;
	int					nMax = 1;
	DWORD				dwBufferPtr = 0;
	DWORD				dwBufferPtr2 = 0;
	DWORD				dwBufferPtrPrev2 = 0x200;
	DWORD				dwCode = 1;
	DWORD				dwIndex;

	for( dwIndex = 0 ; dwBufferPtr < dwBufferSize ; dwIndex++ )
	{
		if( dwIndex & 1 )
		{
			dwBufferPtr2 = 0;
			dwBufferPtrPrev2 = 512;
		}
		else
		{
			dwBufferPtr2 = 512;
			dwBufferPtrPrev2 = 0;
		}

		nMin = 0;

		while( (adwBuffer[dwBufferPtr] >> 16) == dwIndex )
		{
			DWORD*				pdwBuffer3 = (DWORD*) &abtBuffer3[adwBuffer2[dwBufferPtrPrev2] << 4];

			pdwBuffer3[0] = 0;
			pdwBuffer3[1] = adwBuffer[dwBufferPtr] & 0x1FF;

			dwBufferPtr++;
			dwBufferPtrPrev2++;

			nMin++;
		}

		for( int i = nMin ; i < nMax ; i++ )
		{
			DWORD*				pdwBuffer3 = (DWORD*) &abtBuffer3[adwBuffer2[dwBufferPtrPrev2] << 4];
			DWORD				dwBufferPtr3 = 0;

			pdwBuffer3[dwBufferPtr3] = 1;
			dwBufferPtr3 += 2;

			adwBuffer2[dwBufferPtr2] = dwCode;
			pdwBuffer3[dwBufferPtr3] = dwCode;
			dwBufferPtr2++;
			dwBufferPtr3++;
			dwCode++;

			adwBuffer2[dwBufferPtr2] = dwCode;
			pdwBuffer3[dwBufferPtr3] = dwCode;
			dwBufferPtr2++;
			dwBufferPtr3++;
			dwCode++;

			dwBufferPtrPrev2++;
		}

		nMax = (nMax - nMin) * 2;
	}

	//

	DWORD				dwSize = *(DWORD*) &pbtSrc[24];
	DWORD				dwSrc;

	dwCount = 0;

	for( DWORD i = 0 ; (i < dwSize) && (dwSrcPtr < dwSrcSize) && (dwDstPtr < dwDstSize) ; i++ )
	{
		dwIndex = 0;

		do
		{
			if( dwCount == 0 )
			{
				dwSrc = pbtSrc[dwSrcPtr++];
				dwCount = 8;
			}

			dwIndex = dwIndex * 4 + ((dwSrc & 0xFF) >> 7);
			dwSrc <<= 1;
			dwCount--;

			dwIndex = *(DWORD*) &abtBuffer3[4 * dwIndex + 8];
		}
		while( *(DWORD*) &abtBuffer3[dwIndex << 4] != 0 );

		dwIndex <<= 4;
		btWork = abtBuffer3[dwIndex + 4];

		if( abtBuffer3[dwIndex + 5] == 1 )
		{
			DWORD				dwBitBuffer = (dwSrc & 0xFF) >> (8 - dwCount);
			DWORD				dwBitCount = dwCount;

			if( dwCount < 12 )
			{
				dwWork = (19 - dwCount) >> 3;
				dwBitCount = dwCount + 8 * dwWork;

				for( DWORD j = 0 ; j < dwWork ; j++ )
				{
					dwBitBuffer = (dwBitBuffer << 8) + pbtSrc[dwSrcPtr++];
				}
			}

			DWORD				dwBack;
			DWORD				dwLength;

			dwCount = dwBitCount - 12;
			dwSrc = (dwBitBuffer << (8 - dwCount));
			dwBack = ((dwBitBuffer >> dwCount) & 0xFFFF) + 2;
			dwLength = btWork + 2;

			if( (dwBack > dwDstPtr) || (dwDstPtr >= dwDstSize) )
			{
				break;
			}

			for( DWORD j = 0 ; j < dwLength ; j++ )
			{
				pbtDst[dwDstPtr + j] = pbtDst[dwDstPtr + j - dwBack];
			}

			dwDstPtr += dwLength;
		}
		else
		{
			pbtDst[dwDstPtr++] = btWork;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
//	CompressedBG�̉�

void	CEthornell::DecompCBG(
	BYTE*				pbtDst,							// �i�[��
	BYTE*				pbtSrc							// ���k�f�[�^
	)
{
	DWORD				dwSrcPtr = 48;
	DWORD				dwDstPtr = 0;

	long				lWidth = *(WORD*) &pbtSrc[16];
	long				lHeight = *(WORD*) &pbtSrc[18];
	WORD				wBpp = *(WORD*) &pbtSrc[20];

	DWORD				dwDstSizeOfHuffman = *(DWORD*) &pbtSrc[32];
	DWORD				dwKey = *(DWORD*) &pbtSrc[36];
	DWORD				dwDecryptSize = *(DWORD*) &pbtSrc[40];

	DWORD				dwWork;
	BYTE				btWork;

	// ����

	for( DWORD i = 0 ; i < dwDecryptSize ; i++ )
	{
		pbtSrc[dwSrcPtr + i] -= (BYTE) GetKey( &dwKey );
	}

	// �p�x�e�[�u���̎擾

	DWORD				adwFreq[256];

	for( DWORD i = 0 ; i < 256 ; i++ )
	{
		adwFreq[i] = GetVariableData( &pbtSrc[dwSrcPtr], &dwWork );

		dwSrcPtr += dwWork;
	}

	// �t�m�[�h�̓o�^

	SNodeInfo			astNodeInfo[511];
	DWORD				dwFreqTotal = 0;

	for( DWORD i = 0 ; i < 256 ; i++ )
	{
		astNodeInfo[i].bValidity = (adwFreq[i] > 0);
		astNodeInfo[i].dwFreq = adwFreq[i];
		astNodeInfo[i].dwLeft = i;
		astNodeInfo[i].dwRight = i;

		dwFreqTotal += adwFreq[i];
	}

	// �}�m�[�h�̏�����

	for( DWORD i = 256 ; i < 511 ; i++ )
	{
		astNodeInfo[i].bValidity = FALSE;
		astNodeInfo[i].dwFreq = 0;
		astNodeInfo[i].dwLeft = (DWORD) -1;
		astNodeInfo[i].dwRight = (DWORD) -1;
	}

	// �}�m�[�h�̓o�^

	DWORD				dwNodes;

	for( dwNodes = 256 ; dwNodes < 511 ; dwNodes++ )
	{
		// �ŏ��̓�l�𓾂�

		DWORD				dwMin;
		DWORD				dwFreq = 0;
		DWORD				adwChild[2];

		for( DWORD i = 0 ; i < 2 ; i++ )
		{
			dwMin = 0xFFFFFFFF;
			adwChild[i] = (DWORD) -1;

			for( DWORD j = 0 ; j < dwNodes ; j++ )
			{
				if( astNodeInfo[j].bValidity && (astNodeInfo[j].dwFreq < dwMin) )
				{
					dwMin = astNodeInfo[j].dwFreq;
					adwChild[i] = j;
				}
			}

			if( adwChild[i] != (DWORD) -1 )
			{
				astNodeInfo[adwChild[i]].bValidity = FALSE;

				dwFreq += astNodeInfo[adwChild[i]].dwFreq;
			}
		}

		// �}�m�[�h�̓o�^

		astNodeInfo[dwNodes].bValidity = TRUE;
		astNodeInfo[dwNodes].dwFreq = dwFreq;
		astNodeInfo[dwNodes].dwLeft = adwChild[0];
		astNodeInfo[dwNodes].dwRight = adwChild[1];

		if( dwFreq == dwFreqTotal )
		{
			// �I��

			break;
		}
	}

	// �n�t�}���̉�

	DWORD				dwRoot = dwNodes;
	DWORD				dwMask = 0x80;

	YCMemory<BYTE>		clmbtDstOfHuffman( dwDstSizeOfHuffman );

	for( DWORD i = 0 ; i < dwDstSizeOfHuffman ; i++ )
	{
		DWORD				dwNode = dwRoot;

		while( dwNode >= 256 )
		{
			if( pbtSrc[dwSrcPtr] & dwMask )
			{
				dwNode = astNodeInfo[dwNode].dwRight;
			}
			else
			{
				dwNode = astNodeInfo[dwNode].dwLeft;
			}

			dwMask >>= 1;

			if( dwMask == 0 )
			{
				dwSrcPtr++;
				dwMask = 0x80;
			}
		}

		clmbtDstOfHuffman[i] = (BYTE) dwNode;
	}

	// RLE�̉�

	DWORD				dwDstPtrOfHuffman = 0;
	BYTE				btZeroFlag = 0;

	while( dwDstPtrOfHuffman < dwDstSizeOfHuffman )
	{
		DWORD				dwLength = GetVariableData( &clmbtDstOfHuffman[dwDstPtrOfHuffman], &dwWork );

		dwDstPtrOfHuffman += dwWork;

		if( btZeroFlag )
		{
			ZeroMemory( &pbtDst[dwDstPtr], dwLength );

			dwDstPtr += dwLength;
		}
		else
		{
			memcpy( &pbtDst[dwDstPtr], &clmbtDstOfHuffman[dwDstPtrOfHuffman], dwLength );

			dwDstPtr += dwLength;
			dwDstPtrOfHuffman += dwLength;
		}

		btZeroFlag ^= 1;
	}

	// 

	WORD				wColors = (wBpp >> 3);
	long				lLine = lWidth * wColors;

	dwDstPtr = 0;

	for( long lY = 0 ; lY < lHeight ; lY++ )
	{
		for( long lX = 0 ; lX < lWidth ; lX++ )
		{
			for( WORD i = 0 ; i < wColors ; i++ )
			{
				if( (lY == 0) && (lX == 0) )
				{
					// �����

					btWork = 0;
				}
				else if( lY == 0 )
				{
					// ���
					// ���̃s�N�Z�����擾

					btWork = pbtDst[dwDstPtr - wColors];
				}
				else if( lX == 0 )
				{
					// ����
					// ��̃s�N�Z�����擾

					btWork = pbtDst[dwDstPtr - lLine];
				}
				else
				{
					// ���̑�
					// ���Ə�̃s�N�Z���̕��ς��擾

					btWork = (pbtDst[dwDstPtr - wColors] + pbtDst[dwDstPtr - lLine]) >> 1;
				}

				pbtDst[dwDstPtr++] += btWork;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//	�摜�̕�����

void	CEthornell::DecryptBGType1(
	BYTE*				pbtDst,							// �����������摜�̊i�[��
	BYTE*				pbtSrc,							// �Í������ꂽ�摜�f�[�^
	long				lWidth,							// �摜�̕�
	long				lHeight,						// �摜�̍���
	WORD				wBpp							// �r�b�g�[�x
	)
{
	WORD				wColors = (wBpp >> 3);

	// �e�F�����ւ̃|�C���^���擾

	BYTE*				apbtSrc[4];

	for( WORD i = 0 ; i < wColors ; i++ )
	{
		apbtSrc[i] = &pbtSrc[lWidth * lHeight * i];
	}

	// �ϐ��̏�����

	BYTE				abtPrev[4];

	ZeroMemory( abtPrev, sizeof( abtPrev ) );

	// ������

	BYTE*				pbtDst2 = pbtDst;

	for( long i = 0 ; i < lHeight ; i++ )
	{
		if( i & 0x01 )
		{
			pbtDst2 += (lWidth * wColors);

			for( long j = 0 ; j < lWidth ; j++ )
			{
				pbtDst2 -= wColors;

				for( WORD k = 0 ; k < wColors ; k++ )
				{
					pbtDst2[k] = *apbtSrc[k]++ + abtPrev[k];
					abtPrev[k] = pbtDst2[k];
				}
			}

			pbtDst2 += (lWidth * wColors);
		}
		else
		{
			for( long j = 0 ; j < lWidth ; j++ )
			{
				for( WORD k = 0 ; k < wColors ; k++ )
				{
					*pbtDst2 = *apbtSrc[k]++ + abtPrev[k];
					abtPrev[k] = *pbtDst2++;
				}
			}
		}
	}
}
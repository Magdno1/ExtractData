#pragma once

#include "../SearchBase.h"

//----------------------------------------------------------------------------------------
//-- MIDI Search Class -------------------------------------------------------------------
//----------------------------------------------------------------------------------------

class CMidSearch : public CSearchBase
{
public:

		 CMidSearch();

	void Mount( CArcFile* pclArc );
};

#pragma once

#include "Krkr.h"

class CHachukano final : public CKrkr
{
public:
	bool OnCheckDecrypt(CArcFile* archive) override;
	u32 OnInitDecrypt(CArcFile* archive) override;
	size_t OnDecrypt(u8* target, size_t target_size, size_t offset, u32 decrypt_key) override;

private:
	u32 m_change_decrypt_key = 0;
};

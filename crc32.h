#pragma once

#include <stdint.h>
#include <sstream>

class CRC32
{
	/* Table of CRCs of all 8-bit messages. */
	static uint32_t crc_table[256];

	uint32_t update_crc(uint32_t crc, unsigned char* buf, int len);

	uint32_t update_crc(uint32_t crc, std::stringstream& stream);
protected:
	/* Make the table for a fast CRC. */
	static void make_crc_table();
public:
	uint32_t crc(std::stringstream& stream);
};
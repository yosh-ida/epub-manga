#include "crc32.h"

using namespace std;

uint32_t CRC32::crc_table[256];

/* Make the table for a fast CRC. */
void CRC32::make_crc_table()
{
	static bool crc_table_computed = false;
	if (crc_table_computed)
		return;
	crc_table_computed = true;

	uint32_t c;
	int n, k;
	for (n = 0; n < 256; n++) {
		c = (uint32_t)n;
		for (k = 0; k < 8; k++) {
			if (c & 1) {
				c = 0xedb88320L ^ (c >> 1);
			}
			else {
				c = c >> 1;
			}
		}
		crc_table[n] = c;
	}
	crc_table_computed = true;
}

uint32_t CRC32::update_crc(uint32_t crc, unsigned char* buf, int len)
{
	uint32_t c = crc ^ 0xffffffffL;
	int n;

	make_crc_table();
	for (n = 0; n < len; n++) {
		c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
	}
	return c ^ 0xffffffffL;
}

uint32_t CRC32::update_crc(uint32_t crc, stringstream& stream)
{
	uint32_t c = crc ^ 0xffffffffL;
	uint32_t n;

	stream.seekg(0, ios_base::end);
	uint32_t len = stream.tellg();
	stream.seekg(0, ios_base::beg);

	make_crc_table();

	for (n = 0; n < len; n++) {
		c = crc_table[(c ^ (uint8_t)stream.get()) & 0xff] ^ (c >> 8);
	}
	return c ^ 0xffffffffL;
}

uint32_t CRC32::crc(stringstream& stream)
{
	return update_crc(0L, stream);
}
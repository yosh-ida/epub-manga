/**
 * @file zip.h
 * @brief zip
 * @author yosh-ida
 * @date 01/01/2022
 */

#pragma once

#include "crc32.h"
#include <streambuf>

class zip
{
	std::string root = "";
	std::string central_directory_header = "";
	std::string archive_decryption_header = "";
	std::string archive_extra_data_record = "";
	std::ostream& out;

	uint16_t entries;
	CRC32 crc32;
	char* buf;
	const size_t buffersize;

	char crc_str[4];
	char compressed_size_str[4];
	char uncompressed_size_str[4];
	char file_name_length_str[2];
	char extra_field_length_str[2];
	char file_comment_length_str[2];
	char relative_offset_of_local_header[4];

	//	system info
	const char const version_needed_to_extract[2] = { 0x14, 0x00 };	//	version 2.0
	const char const general_purpose_bit_flag[2] = { 0x00, 0x00 };	//	no encrypt 
	const char const compression_method[2] = { 0x00, 0x00 };	//	no compress
	const char const compression_method_deflate[2] = { 0x08, 0x00 };	//	deflate
	const char const version_made_by[2] = { 0x14, 0x00 };	//	version 2.0 / MS-DOS and OS/2 (FAT / VFAT / FAT32 file systems)

	//	signatures
	const char const local_file_header_signature[4] = { 0x50, 0x4b, 0x03, 0x04 };
	const char const central_file_header_signature[4] = { 0x50, 0x4b, 0x01, 0x02 };
	const char const end_of_central_dir_signature[4] = { 0x50, 0x4b, 0x05, 0x06 };

	//	last modified time
	const char const last_mod_file_time[2] = { (char)0x9B, 0x01 };	//	random time
	const char const last_mod_file_date[2] = { 0x0B, 0x51 };	//	random time

	std::stringstream compress(const std::stringstream& data, const size_t size, const int level) const;
public:
	zip(std::ostream& stream);

	zip(std::ostream& stream, const size_t buffersize);

	~zip();

	operator bool() const {
		return true;
	}

	/**
	 * @fn
	 * @brief Add file into zip.
	 * @param (s) file path
	 * @param (in) file buffer
	 * @return ñﬂÇËílÇÃê‡ñæ
	 * @detail è⁄ç◊Ç»ê‡ñæ
	*/
	bool add(const char* s, const std::istream& in);

	void write();
};
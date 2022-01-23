#define ZLIB_WINAPI

#include "zip.h"
#include <sstream>
#include <zlib.h>

using namespace std;

#define ZLIB_COMPRESS	false

zip::zip(ostream& stream) : out(stream), buffersize(64)
{
	entries = 0;
	buf = (char*)malloc(buffersize);
}

zip::zip(ostream& stream, const size_t bs) : out(stream), buffersize(bs)
{
	entries = 0;
	buf = (char*)malloc(buffersize);
}

zip::~zip() {
	free(buf);
}

bool zip::add(const char* s, const std::istream& in)
{
	if (in.fail())
		return false;
	entries++;
	uint32_t start = out.tellp();
	relative_offset_of_local_header[0] = (start & 0xff);
	relative_offset_of_local_header[1] = ((start & 0xff00) >> 8);
	relative_offset_of_local_header[2] = ((start & 0xff0000) >> 16);
	relative_offset_of_local_header[3] = ((start & 0xff000000) >> 24);

	////////////////////////////////////////////
	//	local file header
	////////////////////////////////////////////

	out.write(local_file_header_signature, 4);
	out.write(version_needed_to_extract, 2);
	out.write(general_purpose_bit_flag, 2);
	out.write(ZLIB_COMPRESS ? compression_method_deflate : compression_method, 2);
	out.write(last_mod_file_time, 2);
	out.write(last_mod_file_date, 2);

	stringstream ss;
	ss << in.rdbuf();

	uint32_t crc = crc32.crc(ss);
	//	CRC-32
	crc_str[0] = (crc & 0xff);
	crc_str[1] = ((crc & 0xff00) >> 8);
	crc_str[2] = ((crc & 0xff0000) >> 16);
	crc_str[3] = ((crc & 0xff000000) >> 24);
	out.write(crc_str, 4);

	//	deflate
	ss.seekg(0, ios_base::end);
	uint32_t uclen = ss.tellg();
	ss.seekg(0, ios_base::beg);
	uint32_t clen = uclen;

	if (ZLIB_COMPRESS)
	{
		ss = this->compress(ss, uclen, Z_DEFAULT_COMPRESSION);
		ss.seekg(0, ios_base::end);
		clen = ss.tellg();
		ss.seekg(0, ios_base::beg);
	}

	//	compressed size
	compressed_size_str[0] = (clen & 0xff);
	compressed_size_str[1] = ((clen & 0xff00) >> 8);
	compressed_size_str[2] = ((clen & 0xff0000) >> 16);
	compressed_size_str[3] = ((clen & 0xff000000) >> 24);
	out.write(compressed_size_str, 4);
	//	uncompressed size
	uncompressed_size_str[0] = (uclen & 0xff);
	uncompressed_size_str[1] = ((uclen & 0xff00) >> 8);
	uncompressed_size_str[2] = ((uclen & 0xff0000) >> 16);
	uncompressed_size_str[3] = ((uclen & 0xff000000) >> 24);
	out.write(uncompressed_size_str, 4);

	uint16_t nlen = strnlen_s(s, 1024);
	//	file name length
	file_name_length_str[0] = (nlen & 0xff);
	file_name_length_str[1] = ((nlen & 0xff00) >> 8);
	out.write(file_name_length_str, 2);

	//	extra field length
	extra_field_length_str[0] = 0x00;
	extra_field_length_str[1] = 0x00;
	out.write(extra_field_length_str, 2);

	//	filename
	out.write(s, nlen);

	//	extra field
	//	do nothing

	////////////////////////////////////////////
	//	filedata
	////////////////////////////////////////////	
	//auto sb = in.rdbuf();
	//sb->pubseekoff(0, ios_base::beg, ios_base::in);
	auto size = ss.readsome(buf, buffersize);
	//auto size = sb->sgetn(buf, buffersize);
	for (; size > 0; size = ss.readsome(buf, buffersize))
		out.write(buf, size);

	////////////////////////////////////////////
	//	data descriptor
	//This descriptor MUST exist if bit 3 of the general purpose bit flag is set.
	////////////////////////////////////////////

	//	crc-32
	//out.write(crc_str, 4);
	//	compressed size
	//out.write(compressed_size_str, 4);
	//	uncompressed size
	//out.write(compressed_size_str, 4);

	////////////////////////////////////////////
	//	archive decryption header
	//	The Archive Decryption Header is introduced in version 6.2 of the ZIP format specification.
	////////////////////////////////////////////


	////////////////////////////////////////////
	//	archive extra data record
	//	The Archive Extra Data Record is introduced in version 6.2 of the ZIP format specification.
	////////////////////////////////////////////


	////////////////////////////////////////////
	//	central directory header
	////////////////////////////////////////////
	for (int i = 0; i < 4; i++)
		central_directory_header += central_file_header_signature[i];
	for (int i = 0; i < 2; i++)
		central_directory_header += version_made_by[i];
	for (int i = 0; i < 2; i++)
		central_directory_header += version_needed_to_extract[i];
	for (int i = 0; i < 2; i++)
		central_directory_header += general_purpose_bit_flag[i];
	for (int i = 0; i < 2; i++)
		if (ZLIB_COMPRESS)
			central_directory_header += compression_method_deflate[i];
		else
			central_directory_header += compression_method[i];
	for (int i = 0; i < 2; i++)
		central_directory_header += last_mod_file_time[i];
	for (int i = 0; i < 2; i++)
		central_directory_header += last_mod_file_date[i];

	//	crc-32
	for (int i = 0; i < 4; i++)
		central_directory_header += crc_str[i];
	//	compressed size
	for (int i = 0; i < 4; i++)
		central_directory_header += compressed_size_str[i];
	//	uncompressed size
	for (int i = 0; i < 4; i++)
		central_directory_header += uncompressed_size_str[i];
	//	file name length
	for (int i = 0; i < 2; i++)
		central_directory_header += file_name_length_str[i];
	//	extra field length
	for (int i = 0; i < 2; i++)
		central_directory_header += extra_field_length_str[i];
	//	file comment length
	for (int i = 0; i < 2; i++)
		central_directory_header += file_comment_length_str[i];

	//	disk number start
	//	only one disk used
	central_directory_header += (char)0x00;
	central_directory_header += (char)0x00;
	//	internal file attributes
	central_directory_header += (char)0x00;
	central_directory_header += (char)0x00;
	//	external file attributes
	central_directory_header += (char)0x00;
	central_directory_header += (char)0x00;
	central_directory_header += (char)0x00;
	central_directory_header += (char)0x00;
	//	relative offset of local header
	for (int i = 0; i < 4; i++)
		central_directory_header += relative_offset_of_local_header[i];
	//	file name
	central_directory_header += s;
	//	extra field

	//	file comment


	////////////////////////////////////////////
	//	digital signature
	//	after version 6.2
	////////////////////////////////////////////

	return true;
}

void zip::write()
{
	////////////////////////////////////////////
	//	archive decryption header
	//	after version 6.2
	////////////////////////////////////////////

	////////////////////////////////////////////
	//	archive extra data record
	//	The Archive Extra Data Record is introduced in version 6.2 of the ZIP format specification.
	////////////////////////////////////////////

	////////////////////////////////////////////
	//	central directory header
	////////////////////////////////////////////

	uint32_t cen_offset = out.tellp();
	out.write(central_directory_header.c_str(), central_directory_header.length());

	////////////////////////////////////////////
	//	zip64 end of central directory record
	//	not use
	////////////////////////////////////////////

	////////////////////////////////////////////
	//	zip64 end of central directory locator
	//	not use
	////////////////////////////////////////////

	////////////////////////////////////////////
	//	end of central directory record
	////////////////////////////////////////////

	//	end of central dir signature
	out.write(end_of_central_dir_signature, 4);
	//	number of this disk
	out.put(0x00);
	out.put(0x00);
	//	number of the disk with the start of the central directory	
	out.put(0x00);
	out.put(0x00);
	//	total number of entries in the central directory on this disk	
	out.put(entries & 0xff);
	out.put((entries & 0xff00) >> 8);
	//	total number of entries in the central directory
	out.put(entries & 0xff);
	out.put((entries & 0xff00) >> 8);
	//	size of the central directory
	uint32_t cen_size = central_directory_header.length();
	out.put(cen_size & 0xff);
	out.put((cen_size & 0xff00) >> 8);
	out.put((cen_size & 0xff0000) >> 16);
	out.put((cen_size & 0xff000000) >> 24);
	//	offset of start of central directory ...
	out.put(cen_offset & 0xff);
	out.put((cen_offset & 0xff00) >> 8);
	out.put((cen_offset & 0xff0000) >> 16);
	out.put((cen_offset & 0xff000000) >> 24);
	//	.ZIP file comment length
	out.put(0x00);
	out.put(0x00);
	//	.ZIP file comment
}

stringstream zip::compress(const stringstream& data, const size_t size, const int level) const
{
	static size_t limit = (size_t)1 << 31;
	//	less than 2GB 
	if (size >= limit)
		throw std::runtime_error("size may use more memory than intended when decompressing");

	z_stream deflate_s;
	deflate_s.zalloc = Z_NULL;
	deflate_s.zfree = Z_NULL;
	deflate_s.opaque = Z_NULL;
	deflate_s.avail_in = 0;
	deflate_s.next_in = Z_NULL;
	deflate_s.data_type = Z_BINARY;

	constexpr int window_bits = -15;	//	no header / windowbits=15
	constexpr int mem_level = MAX_MEM_LEVEL;

	if (deflateInit2(&deflate_s, level, Z_DEFLATED, window_bits, mem_level, Z_DEFAULT_STRATEGY) != Z_OK)
		throw std::runtime_error("deflate init failed");
	deflate_s.next_in = (z_const Bytef*)(data.str().c_str());
	deflate_s.avail_in = size;

	size_t size_compressed = 0;
	string output;
	
	do
	{
		size_t increase = size / 2 + 1024;
		if (size < (size_compressed + increase))
		{
			output.resize(size_compressed + increase);
		}
		deflate_s.avail_out = static_cast<unsigned int>(increase);
		deflate_s.next_out = reinterpret_cast<Bytef*>((&output[0] + size_compressed));
		deflate(&deflate_s, Z_FINISH);
		size_compressed += (increase - deflate_s.avail_out);
	} while (deflate_s.avail_out == 0);

	deflateEnd(&deflate_s);
	output.resize(size_compressed);
	return stringstream(output);
}
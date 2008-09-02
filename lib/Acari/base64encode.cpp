#include "urlencode.h"
#include <cassert>

namespace Acari
{
	const char base64_characters__[64+1] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	const char decode_characters__[256] = {
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
		52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
		-1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
		15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
		-1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
		41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
	};

	ACARI_API std::string base64encode(const void * data, size_t length)
	{
		// Calculate the len of the destination buffer
		// 2^6 = 64 . With 3 characters, 4 base64 characters are generated
		size_t base64_length((length / 3) * 4);

		const unsigned char * char_data = static_cast<const unsigned char *>(data);

		if (length % 3)
			base64_length+=4;

		//Base64 string are terminated with '=' characters
		std::string base64_buffer(base64_length, '=');

		// Handle groups of 3 characters
		base64_buffer[base64_length] = '\0';
		unsigned int pos, dest_pos=0;
		for (pos = 0; pos+3 <= length; pos+=3)
		{
			unsigned int curr(char_data[pos] << 16 | char_data[pos+1] << 8 | char_data[pos+2]);
			base64_buffer[dest_pos++] = base64_characters__[(curr & 0xFC0000) >> 18];
			base64_buffer[dest_pos++] = base64_characters__[(curr & 0x3F000) >> 12];
			base64_buffer[dest_pos++] = base64_characters__[(curr & 0xFC0) >> 6];
			base64_buffer[dest_pos++] = base64_characters__[curr & 0x3F];
		}

		// Handle remaining characters
		if (pos != length)
		{
			// Does the remaining len is two
			if ((length-pos) == 2)
			{
				unsigned int curr(char_data[pos] << 16 | char_data[pos+1] << 8);
				base64_buffer[dest_pos++] = base64_characters__[(curr & 0xFC0000)>>18];
				base64_buffer[dest_pos++] = base64_characters__[(curr & 0x3F000)>>12];
				base64_buffer[dest_pos++] = base64_characters__[(curr & 0xFC0)>>6];
			}
			else
			{
				// Remain only one character
				unsigned int curr(char_data[pos] << 16);
				base64_buffer[dest_pos++] = base64_characters__[(curr & 0xFC0000)>>18];
				base64_buffer[dest_pos++] = base64_characters__[(curr & 0x3F000)>>12];
			}
		}
		return base64_buffer;
	}

	ACARI_API std::string base64encode(const std::string & data)
	{
		return base64encode(data.c_str(), data.size());
	}

	ACARI_API std::string base64decode(const char * base64_data, size_t length)
	{
		const unsigned char * base64_unsigned(reinterpret_cast< const unsigned char * >(base64_data));

		while (length > 0 && base64_unsigned[length-1] == '=')
			length--;

		size_t decode_length = (length/4)*3;
		size_t remainder = length % 4;
		if (remainder == 1)
			return "";
		else if (remainder == 2)
			decode_length +=1;
		else if (remainder == 3)
			decode_length +=2;

		std::string decoded_data(decode_length, '-');
		
		unsigned int destination_index(0);
		unsigned int index(0);
		for (; index + 4 <= length; index += 4)
		{
			int value(decode_characters__[base64_unsigned[index]] << 18);
			value |= decode_characters__[base64_unsigned[index+1]] << 12;
			value |= decode_characters__[base64_unsigned[index+2]] << 6;
			value |= decode_characters__[base64_unsigned[index+3]];
			if (value < 0)
				return "";
			decoded_data[destination_index++] = static_cast<char>((value & 0xFF0000) >> 16);
			decoded_data[destination_index++] = static_cast<char>((value & 0xFF00) >> 8);
			decoded_data[destination_index++] = static_cast<char>((value & 0xFF));
		}
		if (remainder == 2)
		{
			// Two base64 characters contains one real caracter
			int value(decode_characters__[base64_unsigned[index++]] << 18);
			value |= decode_characters__[base64_unsigned[index++]] << 12;
			if (value < 0)
				return "";
			decoded_data[destination_index++] = static_cast<char>((value & 0xFF0000) >> 16);
		}
		else if (remainder == 3)
		{
			// Two base64 characters contains two real caracters
			int value(decode_characters__[base64_unsigned[index++]] << 18);
			value |= decode_characters__[base64_unsigned[index++]] << 12;
			value |= decode_characters__[base64_unsigned[index++]] << 6;
			if (value < 0)
				return "";
			decoded_data[destination_index++] = static_cast<char>((value & 0xFF0000) >> 16);
			decoded_data[destination_index++] = static_cast<char>((value & 0xFF00) >> 8);
		}

		return decoded_data;
	}

	ACARI_API std::string base64decode(const std::string & data)
	{
		return base64decode(data.c_str(), data.size());
	}
}


#include <openssl/err.h>

#define SCORPION_PRIVATE_OPENSSL_EXEC(command, post_condition, exception_type)													\
	command;																												\
	if (!(post_condition))																									\
	{																														\
		const char * filename(0);																							\
		int line(0);																										\
		const char * error_line_data(0);																					\
		int error_line_data_flags(0);																						\
		unsigned long error_code(::ERR_get_error_line_data(&filename, &line, &error_line_data, &error_line_data_flags));	\
		throw exception_type(error_code, filename, line, error_line_data, error_line_data_flags);							\
	}


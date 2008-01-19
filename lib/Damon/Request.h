#ifndef _damon_request_h
#define _damon_request_h

#include "Details/prologue.h"
#include <string>
#include "Response.h"
#include "Details/Header.h"

namespace Damon
{
	class Request;

	class DAMON_API Request
	{
	public :
		enum Method { options__, get__, head__, post__, put__, delete__, trace__, connect__ };

		Request(const std::string & url, Method method = get__);
		~Request();

		/** Add a header to the response. */
		void addHeader(const std::string & name, const std::string & value) { header_fields_.push_back(Details::Header(name, value)); }
		Request & operator()(const std::string & name, const std::string & value) { addHeader(name, value); return *this; }

		/** Set the body of the response */
		void setBody(const std::vector< char > & body) { body_ = body; }
		Request & operator()(const std::vector< char > & body) { setBody(body); return *this; }
		Request & operator()(const std::string & body) { setBody(std::vector< char >(body.begin(), body.end())); return *this; }
		Request & operator()(const char * body) { return (*this)(std::string(body)); }
		template < typename T >
		Request & operator()(const T & body) { return (*this)(serialize(body)); }

	private :
		std::string url_;
		Method method_;
		Details::HeaderFields header_fields_;		///< all header fields found while parsing the request
		std::vector< char > body_;

		friend DAMON_API Response send(const Request &/* request*/);
		friend DAMON_API std::string serialize(const Request &/* request*/);
	};

	DAMON_API Response send(const Request & request);
	DAMON_API std::string serialize(const Request & request);
}

#endif

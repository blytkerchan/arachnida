#ifndef _damon_request_h
#define _damon_request_h

#include "Details/prologue.h"
#include <string>
#include "Response.h"
#include "Details/Header.h"

namespace Damon
{
	class Request;
	class Session;

	/** A request to be sent to a server.
	 * To construct a complex request, try this
	 * \code
	 * Request req("url");
	 * req("header-name", "value")
	 *    ("another-header-name", "another value")
	 * 	(some_serializable_object)
	 * 	;
	 * \endcode */
	class DAMON_API Request
	{
	public :
		//! Type of the method to be used in the request
		enum Method {
			//! send the request with the OPTIONS method
			options__,
			//! send the request with the GET method
			get__,
			//! send the request with the HEAD method
			head__,
			//! send the request with the POST method
			post__,
			//! send the request with the PUT method
			put__,
			//! send the request with the DELETE method
			delete__,
			//! send the request with the TRACE method
			trace__,
			//! send the request with the CONNECT method
			connect__
		};

		/** Construct a request.
		 * \param url the URL of the resource you're requesting. Should normally 
		 *        at least include http:// or https://, and the server name. The 
		 *        URL will be parsed to extract the server name and port number,
		 *        as well as whether or not HTTPS should be used to secure the 
		 *        connection. 
		 * \param method the method that should be used with the request, GET by
		 *        default.*/
		Request(const std::string & url, Method method = get__);
		~Request();

		/** Add a header to the response.
		 * \param name name of the header to be added
		 * \param value value of the header to be added */
		void addHeader(const std::string & name, const std::string & value) { header_fields_.push_back(Details::Header(name, value)); }
		/** Add a header to the response.
		 * \param name name of the header to be added
		 * \param value value of the header to be added */
		Request & operator()(const std::string & name, const std::string & value) { addHeader(name, value); return *this; }

		//! Set the body of the response
		void setBody(const std::vector< char > & body) { body_ = body; }
		//! Set the body of the response
		Request & operator()(const std::vector< char > & body) { setBody(body); return *this; }
		//! Set the body of the response
		Request & operator()(const std::string & body) { setBody(std::vector< char >(body.begin(), body.end())); return *this; }
		//! Set the body of the response
		Request & operator()(const char * body) { return (*this)(std::string(body)); }
		//! Set the body of the response
		template < typename T >
		Request & operator()(const T & body) { return (*this)(serialize(body)); }

	private :
		std::string url_;
		Method method_;
		Details::HeaderFields header_fields_;		///< all header fields found while parsing the request
		std::vector< char > body_;

		friend DAMON_API Response send(Session & /*session*/, const Request &/* request*/);
		friend DAMON_API std::vector< Response > send(Session & /*session*/, const std::vector< Request > & requests);
		friend DAMON_API std::string serialize(Request /* request*/);
	};

	DAMON_API Response send(Session & session, const Request & request);
	DAMON_API Response send(const Request & request);
	DAMON_API std::vector< Response > send(Session & session, const std::vector< Request > & requests);
	DAMON_API std::vector< Response > send(const std::vector< Request > & requests);
	DAMON_API std::string serialize(Request request);
}

#endif

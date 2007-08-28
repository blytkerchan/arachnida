#ifndef _spin_private_grammar_h
#define _spin_private_grammar_h

#include <boost/spirit.hpp>

namespace Spin
{
	namespace Private
	{
		struct Grammar : boost::spirit::grammar< Grammar >
		{
			struct HexAdapter
			{
				HexAdapter(std::size_t & s)
					: p_(&s)
				{ /* no-op */ }

				template < typename InputIterator >
				HexAdapter(InputIterator begin, InputIterator end)
					: p_(0),
					  s_(begin, end)
				{ /* no-op */ }

				HexAdapter & operator=(const HexAdapter & rhs)
				{
					if (this != &rhs)
					{
						if (p_ && !rhs.p_)
							*p_ = dehex(rhs.s_);
						else
						{
							HexAdapter a(rhs);
							return swap(a);
						}
					}
					else
					{ /* no-op */ }

					return *this;
				}

				HexAdapter & swap(HexAdapter & rhs)
				{
					using std::swap;
					swap(p_, rhs.p_);
					swap(s_, rhs.s_);
					return *this;
				}

				static std::size_t dehex(const std::string & s)
				{
					std::size_t retval(0);
					static const char charset[] = "0123456789abcdefABCDEF";
					static const char * const end = charset + 22;
					for (std::string::const_iterator curr(s.begin()); curr != s.end(); ++curr)
					{
						const char * where(std::find(charset, end, *curr));
						if (where == end)
							throw std::runtime_error("Invalid hex character");
						else
						{ /* all is well */ }
						if (std::distance(charset, where) > 16)
							std::advance(where, -6);
						else
						{ /* all is well */ }
						retval *= 16;
						retval += std::distance(charset, where);
					}
					return retval;
				}

				std::size_t * p_;
				std::string s_;
			};

			template < typename Scanner >
			struct definition
			{
				definition(const Grammar & self)
				{
					using namespace boost::spirit;
					OCTET_			= range_p(0, 255)
									;
					CHAR_			= range_p(0, 127)
									;
					UPALPHA_		= range_p('A', 'Z')
									;
					LOALPHA_		= range_p('a', 'z')
									;
					ALPHA_			= UPALPHA_ | LOALPHA_
									;
					DIGIT_			= range_p('0', '9')
									;
					CTL_			= range_p(0, 31) | ch_p(127)
									;
					CR_				= ch_p(13)
									;
					LF_				= ch_p(10)
									;
					SP_				= ch_p(32)
									;
					HT_				= ch_p(9)
									;
					DOUBLE_QUOTE_	= ch_p('"')
									;
					CRLF_			= CR_ >> LF_
									;
					LWS_			= !CRLF_ >> +( SP_ | HT_ )
									;
					TEXT_			= (OCTET_ - CTL_) | LWS_
									;
					HEX_			= range_p('a', 'f') 
									| range_p('A', 'F')
									| DIGIT_
									;
					separators_		= ch_p('(')
									| ')' | '<' | '>' | '@'
									| ',' | ';' | ':' | '\\'
									| '"' | '/' | '[' | ']'
									| '?' | '=' | '{' | '}'
									| SP_ | HT_
									;
					token_			= +(CHAR_ - CTL_ - separators_)
									;
					comment_		= '(' >> *(ctext_ | quoted_pair_ | comment_) >> ')'
									;
					ctext_			= TEXT_ - '(' - ')'
									;
					quoted_string_	= ch_p('"') >> *(qdtext_ | quoted_pair_) >> ch_p('"')
									;
					qdtext_			= TEXT_ - '"'
									;
					quoted_pair_	= ch_p('\\') >> CHAR_
									;
					HTTP_version_	= str_p("HTTP") >> ch_p('/') >> +DIGIT_ >> '.' >> +DIGIT_
									;
					//http_URL_		= str_p("http:") >> "//" >> host_ >> !(':' >> port_) >> !(abs_path_ >> !('?' >> query_))
					//				;
					HTTP_date_		= rfc1123_date_
									| rfc850_date_
									| asctime_date_
									;
					rfc1123_date_	= wkday_ >> ',' >> SP_ >> date1_ >> SP_ >> time_ >> SP_ >> "GMT"
									;
					rfc850_date_	= weekday_ >> ',' >> SP_ >> date2_ >> SP_ >> time_ >> SP_ >> "GMT"
									;
					asctime_date_	= wkday_ >> SP_ >> date3_ >> SP_ >> time_ >> SP_ >> repeat_p(4)[DIGIT_]
									;
					date1_			= repeat_p(2)[DIGIT_] >> SP_ >> month_ >> SP_ >> repeat_p(4)[DIGIT_]
									;
					date2_			= repeat_p(2)[DIGIT_] >> '-' >> month_ >> '-' >> repeat_p(2)[DIGIT_]
									;
					date3_			= month_ >> SP_ >> ( repeat_p(2)[DIGIT_] | ( SP_ >> DIGIT_ ))
									;
					time_			= repeat_p(2)[DIGIT_] >> ':' >> repeat_p(2)[DIGIT_] >> ':' >> repeat_p(2)[DIGIT_]
									;
					wkday_			= str_p("Mon") | "Tue" | "Wed"
									| "Thu" | "Fri" | "Sat" | "Sun"
									;
					weekday_		= str_p("Monday") | "Tuesday" | "Wednesday"
									| "Thursday" | "Friday" | "Saturday" | "Sunday"
									;
					month_			= str_p("Jan") | "Feb" | "Mar" | "Apr"
									| "May" | "Jun" | "Jul" | "Aug"
									| "Sep" | "Oct" | "Nov" | "Dec"
									;
					delta_seconds_	= +DIGIT_
									;
					charset_		= token_
									;
					content_coding_	= token_
									;
					transfer_coding_= str_p("chunked") | transfer_extension_
									;
					transfer_extension_
									= token_ >> *(ch_p(';') >> parameter_);
					parameter_		= attribute_ >> '=' >> value_
									;
					attribute_		= token_
									;
					value_			= token_ | quoted_string_
									;
					Chunked_Body_	= *chunk_ >> last_chunk_ >> trailer_ >> CRLF_
									;
					chunk_			= chunk_size_p_ >> !chunk_extension_ >> CRLF_ >> chunk_data_ >> CRLF_
									;
					chunk_size_p_	= (+HEX_)[assign_a(HexAdapter(chunk_size_))]
									;
					last_chunk_		= +ch_p('0') >> !chunk_extension_ >> CRLF_
									;
					chunk_extension_= *( ch_p(';') >> chunk_ext_name_ >> !(ch_p('=') >> chunk_ext_val_) )
									;
					chunk_ext_name_	= token_
									;
					chunk_ext_val_	= token_ | quoted_string_
									;
					chunk_data_		= repeat_p(boost::ref(chunk_size_))[OCTET_]
									;
					//trailer_		= *(entity_header_ >> CRLF_)
					//				;
					media_type_		= type_ >> '/' >> subtype >> *(';' >> parameter_)
									;
					type_			= token_
									;
					subtype_		= token_
									;
					product_		= token_ >> !('/' >> product_version_)
									;
					product_version_= token_
									;
					qvalue_			= (ch_p('1') >> !repeat_p(0,3)[DIGIT_])
									| (ch_p('1') >> !repeat_p(0,3)['0'])
									;
					language_tag_	= primary_tag_ >> *('-' >> subtag_)
									;
					primary_tag_	= repeat_p(1,8)[ALPHA_]
									;
					subtag_			= repeat_p(1,8)[ALPHA_]
									;
					entity_tag_		= !weak_ >> opaque_tag_
									;
					weak_			= str_p("W/")
									;
					opaque_tag_		= quoted_string_
									;
					range_unit_		= bytes_unit_ 
									| other_range_unit_
									;
					bytes_unit_		= str_p("bytes")
									;
					other_range_unit_
									= token_
									;
					HTTP_message_	= Request_ | Response_
									;
					// The standard says that certain buggy cliens emit extra CRLFs after the body of a POST message, so we catch that here as well
					generic_message_= start_line_ >> *(message_header_ >> CRLF_) >> CRLF_ >> !message_body_ >> *CRLF_
									;
					// Note the slight modification here. The standard says that HTTP/1.1 implementations should be robust and ignore any leading CRLF before the start line. The only way to do that is to incorporate them in the grammar.
					start_line_		= *CRLF_ >> Request_Line_ | Status_Line_
									;
					message_header_	= field_name_ >> ':' >> !field_value_
									;
					field_name_		= token_
									;
					field_value_	= *(field_content_ | LWS_)
									;
					field_content_	= *(token_ | separators_ | quoted_string_)
									| *TEXT_
									;
					message_body_	= entity_body_
//									| +OCTET_ // <entity-body encoded as per Transfer-Encoding>
									;
				}

				const boost::spirit::rule< Scanner > & start() const
				{
					return target_;
				}

				//////////////////////////////////////////////////////////////////////////
				// the following rules are defined in RFC 2616, section 2.2 "Basic Rules"
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The following rules are used throughout this specification
				 * to describe basic parsing constructs. The US-ASCII coded
				 * character set is defined by ANSI X3.4-1986
				 */
				boost::spirit::rule< Scanner > OCTET_;
				boost::spirit::rule< Scanner > CHAR_;
				boost::spirit::rule< Scanner > UPALPHA_;
				boost::spirit::rule< Scanner > LOALPHA_;
				boost::spirit::rule< Scanner > ALPHA_;
				boost::spirit::rule< Scanner > DIGIT_;
				boost::spirit::rule< Scanner > CTL_;
				boost::spirit::rule< Scanner > CR_;
				boost::spirit::rule< Scanner > LF_;
				boost::spirit::rule< Scanner > SP_;
				boost::spirit::rule< Scanner > HT_;
				boost::spirit::rule< Scanner > DOUBLE_QUOTE_;
				/*
				 * HTTP/1.1 defines the sequence CR LF as the end-of-line 
				 * marker for all protocol elements except the entity-body
				 * (see appendix 19.3 for tolerant applications). The 
				 * end-of-line marker within an entity-body is defined by its
				 * associated media type, as described in section 3.7.
				 */
				boost::spirit::rule< Scanner > CRLF_;
				/*
				 * HTTP/1.1 header field values can be folded onto multiple 
				 * lines if the continuation line begins with a space or
				 * horizontal tab. All linear white space, including folding,
				 * has the same semantics as SP. A recipient MAY replace any
				 * linear white space with a single SP before interpreting 
				 * the field value or forwarding the message downstream.
				 */
				boost::spirit::rule< Scanner > LWS_;
				/*
				 * The TEXT rule is only used for descriptive field contents 
				 * and values that are not intended to be interpreted by the
				 * message parser. Words of *TEXT MAY contain characters from
				 * character sets other than ISO-8859-1 [22] only
				 * when encoded according to the rules of RFC 2047 [14].
				 */
				boost::spirit::rule< Scanner > TEXT_;
				/*
				 * A CRLF is allowed in the definition of TEXT only as part 
				 * of a header field continuation. It is expected that the
				 * folding LWS will be replaced with a single SP before 
				 * interpretation of the TEXT value. 
				 */
				/*
				 * Hexadecimal numeric characters are used in several 
				 * protocol elements.
				 */
				boost::spirit::rule< Scanner > HEX_;
				/*
				 * Many HTTP/1.1 header field values consist of words separated
				 * by LWS or special characters. These special characters MUST
				 * be in a quoted string to be used within a parameter value
				 * (as defined in section 3.6).
				 */
				boost::spirit::rule< Scanner > token_;
				boost::spirit::rule< Scanner > separators_;
				/* Comments can be included in some HTTP header fields by 
				 * surrounding the comment text with parentheses.
				 *
				 * Comments are only allowed in fields containing “comment” 
				 * as part of their field value definition. In all other 
				 * fields, parentheses are considered part of the field value.
				 */
				boost::spirit::rule< Scanner > comment_;
				boost::spirit::rule< Scanner > ctext_;
				/*
				 * A string of text is parsed as a single word if it is quoted 
				 * using double-quote marks.
				 */
				boost::spirit::rule< Scanner > quoted_string_;
				boost::spirit::rule< Scanner > qdtext_;
				/*
				 * The backslash character (“\”) MAY be used as a single-character 
				 * quoting mechanism only within quoted-string and comment 
				 * constructs.
				 */
				boost::spirit::rule< Scanner > quoted_pair_;
				//////////////////////////////////////////////////////////////////////////
				// Section 3.0 of the RFC
				//////////////////////////////////////////////////////////////////////////
				/*
				 * HTTP uses a “<major>.<minor>” numbering scheme to indicate versions of 
				 * the protocol. The protocol versioning policy is intended to allow the 
				 * sender to indicate the format of a message and its capacity for 
				 * understanding further HTTP communication, rather than the features 
				 * obtained via that communication. No change is made to the version
				 * number for the addition of message components which do not affect 
				 * communication behavior or which only add to extensible field values. 
				 * The <minor> number is incremented when the changes made to the protocol 
				 * add features which do not change the general message parsing algorithm,
				 * but which may add to the message semantics and imply additional 
				 * capabilities of the sender. The <major> number is incremented when the
				 * format of a message within the protocol is changed. See RFC 2145 [36]
				 * for a fuller explanation.
				 *
				 * The version of an HTTP message is indicated by an HTTP-Version field 
				 * in the first line of the message.
				 */
				boost::spirit::rule< Scanner > HTTP_version_;
				/*
				 * Note that the major and minor numbers MUST be treated as separate
				 * integers and that each MAY be incremented higher than a single
				 * digit. Thus, HTTP/2.4 is a lower version than HTTP/2.13, which in
				 * turn is lower than HTTP/12.3. Leading zeros MUST be ignored by
				 * recipients and MUST NOT be sent.
				 *
				 * An application that sends a request or response message that includes 
				 * HTTP-Version of “HTTP/1.1” MUST be at least conditionally compliant 
				 * with this specification. Applications that are at least conditionally
				 * compliant with this specification SHOULD use an HTTP-Version of
				 * “HTTP/1.1” in their messages, and MUST do so for any message
				 * that is not compatible with HTTP/1.0. For more details on when to 
				 * send specific HTTP-Version values, see RFC 2145 [36].
				 *
				 * The HTTP version of an application is the highest HTTP version for 
				 * which the application is at least conditionally compliant.
				 *
				 * Proxy and gateway applications need to be careful when forwarding 
				 * messages in protocol versions different from that of the application.
				 * Since the protocol version indicates the protocol capability of the 
				 * sender, a proxy/gateway MUST NOT send a message with a version 
				 * indicator which is greater than its actual version. If a higher 
				 * version request is received, the proxy/gateway MUST either downgrade 
				 * the request version, or respond with an error, or switch to tunnel 
				 * behavior.
				 * 
				 * Due to interoperability problems with HTTP/1.0 proxies discovered 
				 * since the publication of RFC 2068[33], caching proxies MUST, 
				 * gateways MAY, and tunnels MUST NOT upgrade the request to the highest 
				 * version they support.
				 *
				 * The proxy/gateway’s response to that request MUST be in the same major 
				 * version as the request.
				 *
				 * Note: Converting between versions of HTTP may involve modification of 
				 * header fields required or forbidden by the versions involved.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 3.2.2
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The “http” scheme is used to locate network resources via the HTTP 
				 * protocol. This section defines the schemespecific syntax and semantics for 
				 * http URLs.
				 */
				boost::spirit::rule< Scanner > http_URL_;
				/*
				 * If the port is empty or not given, port 80 is assumed. The semantics are 
				 * that the identified resource is located at the server listening for TCP
				 * connections on that port of that host, and the Request-URI for the resource
				 * is abs_path (section 5.1.2). The use of IP addresses in URLs SHOULD be
				 * avoided whenever possible (see RFC 1900 [24]). If the abs_path is not
				 * present in the URL, it MUST be given as “/” when used as a Request-URI
				 * for a resource (section 5.1.2). If a proxy receives a host name which
				 * is not a fully qualified domain name, it MAY add its domain to the host
				 * name it received. If a proxy receives a fully qualified domain name, the
				 * proxy MUST NOT change the host name.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 3.3.1
				//////////////////////////////////////////////////////////////////////////
				/*
				 * HTTP applications have historically allowed three different formats for 
				 * the representation of date/time stamps:
				 *     Sun, 06 Nov 1994 08:49:37 GMT ; RFC 822, updated by RFC 1123
				 *     Sunday, 06-Nov-94 08:49:37 GMT ; RFC 850, obsoleted by RFC 1036
				 *     Sun Nov 6 08:49:37 1994 ; ANSI C's asctime() format
				 * The first format is preferred as an Internet standard and represents a
				 * fixed-length subset of that defined by RFC 1123 [8] (an update to 
				 * RFC 822 [9]). The second format is in common use, but is based on the 
				 * obsolete RFC 850 [12] date format and lacks a four-digit year. HTTP/1.1
				 * clients and servers that parse the date value MUST accept all three
				 * formats (for compatibility with HTTP/1.0), though they MUST only generate 
				 * the RFC 1123 format for representing HTTP-date values in header fields.
				 * See section 19.3 for further information.
				 *
				 * Note: Recipients of date values are encouraged to be robust in accepting 
				 * date values that may have been sent by non-HTTP applications, as is 
				 * sometimes the case when retrieving or posting messages via proxies/gateways
				 * to SMTP or NNTP.
				 *
				 * All HTTP date/time stamps MUST be represented in Greenwich Mean Time (GMT),
				 * without exception. For the purposes of HTTP, GMT is exactly equal to UTC
				 * (Coordinated Universal Time). This is indicated in the first two formats by
				 * the inclusion of “GMT” as the three-letter abbreviation for time zone, and
				 * MUST be assumed when reading the asctime format. HTTP-date is case sensitive
				 * and MUST NOT include additional LWS beyond that specifically included as SP
				 * in the grammar.*/
				boost::spirit::rule< Scanner > HTTP_date_;
				boost::spirit::rule< Scanner > rfc1123_date_;
				boost::spirit::rule< Scanner > rfc850_date_;
				boost::spirit::rule< Scanner > asctime_date_;
				boost::spirit::rule< Scanner > date1_;
				boost::spirit::rule< Scanner > date2_;
				boost::spirit::rule< Scanner > date3_;
				boost::spirit::rule< Scanner > time_;
				boost::spirit::rule< Scanner > wkday_;
				boost::spirit::rule< Scanner > weekday_;
				boost::spirit::rule< Scanner > month_;
				/*
				 * Note: HTTP requirements for the date/time stamp format apply only to 
				 * their usage within the protocol stream. Clients and servers are not
				 * required to use these formats for user presentation, request logging,
				 * etc.
				 */
				/*
				 * Some HTTP header fields allow a time value to be specified as an 
				 * integer number of seconds, represented in decimal, after the time 
				 * that the message was received.
				 */
				boost::spirit::rule< Scanner > delta_seconds_;
				//////////////////////////////////////////////////////////////////////////
				// Section 3.4
				//////////////////////////////////////////////////////////////////////////
				/*
				 * HTTP character sets are identified by case-insensitive tokens. The 
				 * complete set of tokens is defined by the IANA Character Set 
				 * registry [19].
				 */
				boost::spirit::rule< Scanner > charset_;
				/*
				 * Although HTTP allows an arbitrary token to be used as a charset value,
				 * any token that has a predefined value within the IANA Character Set
				 * registry [19] MUST represent the character set defined by that registry.
				 * Applications SHOULD limit their use of character sets to those defined
				 * by the IANA registry.
				 * 
				 * Implementors should be aware of IETF character set requirements [38] [41].
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 3.5
				//////////////////////////////////////////////////////////////////////////
				/* 
				 * Content coding values indicate an encoding transformation that has been
				 * or can be applied to an entity. Content codings are primarily used to
				 * allow a document to be compressed or otherwise usefully transformed
				 * without losing the identity of its underlying media type and without
				 * loss of information. Frequently, the entity is stored in coded form,
				 * transmitted directly, and only decoded by the recipient.
				 */
				boost::spirit::rule< Scanner > content_coding_;
				/*
				 * All content-coding values are case-insensitive. HTTP/1.1 uses
				 * content-coding values in the Accept-Encoding (section 14.3) and
				 * Content-Encoding (section 14.11) header fields. Although the value
				 * describes the content-coding, what is more important is that it
				 * indicates what decoding mechanism will be required to remove the
				 * encoding.
				 *
				 * The Internet Assigned Numbers Authority (IANA) acts as a registry
				 * for content-coding value tokens. Initially, the registry contains
				 * the following tokens:
				 *
				 * gzip
				 *     An encoding format produced by the file compression program
				 *     “gzip” (GNU zip) as described in RFC 1952 [25]. This format
				 *     is a Lempel-Ziv coding (LZ77) with a 32 bit CRC.
				 * compress
				 *     The encoding format produced by the common UNIX file
				 *     compression program “compress”. This format is an adaptive
				 *     Lempel-Ziv-Welch coding (LZW).
				 *
				 *     Use of program names for the identification of encoding formats
				 *     is not desirable and is discouraged for future encodings. Their
				 *     use here is representative of historical practice, not good
				 *     design. For compatibility with previous implementations of HTTP,
				 *     applications SHOULD consider “x-gzip” and “x-compress” to be 
				 *     equivalent to “gzip” and “compress” respectively.
				 * deflate
				 *     The “zlib” format defined in RFC 1950 [31] in combination with
				 *     the “deflate” compression mechanism described in RFC 1951 [29].
				 * identity
				 *     The default (identity) encoding; the use of no transformation
				 *     whatsoever. This content-coding is used only in the Accept-Encoding
				 *     header, and SHOULD NOT be used in the Content-Encoding header.
				 *
				 * New content-coding value tokens SHOULD be registered; to allow 
				 * interoperability between clients and servers, specifications of the
				 * content coding algorithms needed to implement a new value SHOULD be
				 * publicly available and adequate for independent implementation, and
				 * conform to the purpose of content coding defined in this section.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 3.6
				//////////////////////////////////////////////////////////////////////////
				/*
				 * Transfer-coding values are used to indicate an encoding transformation
				 * that has been, can be, or may need to be applied to an entity-body in
				 * order to ensure “safe transport” through the network. This differs from
				 * a content coding in that the transfer-coding is a property of the message,
				 * not of the original entity.
				 */
				boost::spirit::rule< Scanner > transfer_coding_;
				boost::spirit::rule< Scanner > transfer_extension_;
				/* Parameters are in the form of attribute/value pairs. */
				boost::spirit::rule< Scanner > parameter_;
				boost::spirit::rule< Scanner > attribute_;
				boost::spirit::rule< Scanner > value_;
				/*
				 * All transfer-coding values are case-insensitive. HTTP/1.1 uses
				 * transfer-coding values in the TE header field (section 14.39) and in
				 * the Transfer-Encoding header field (section 14.41).
				 * 
				 * Whenever a transfer-coding is applied to a message-body, the set
				 * of transfer-codings MUST include “chunked”, unless the message is
				 * terminated by closing the connection. When the “chunked” transfer-coding
				 * is used, it MUST be the last transfer-coding applied to the message-body.
				 * The “chunked” transfer-coding MUST NOT be applied more than once to a
				 * message-body. These rules allow the recipient to determine the
				 * transfer-length of the message (section 4.4).
				 *
				 * Transfer-codings are analogous to the Content-Transfer-Encoding values of
				 * MIME [7], which were designed to enable safe transport of binary data over
				 * a 7-bit transport service. However, safe transport has a different focus
				 * for an 8bit-clean transfer protocol. In HTTP, the only unsafe characteristic
				 * of message-bodies is the difficulty in determining the exact body length
				 * (section 7.2.2), or the desire to encrypt data over a shared transport.
				 *
				 * The Internet Assigned Numbers Authority (IANA) acts as a registry for
				 * transfer-coding value tokens. Initially, the registry contains the
				 * following tokens: “chunked” (section 3.6.1), “identity” (section 3.6.2),
				 * “gzip” (section 3.5), “compress” (section 3.5), and “deflate” (section 3.5).
				 * 
				 * New transfer-coding value tokens SHOULD be registered in the same way as
				 * new content-coding value tokens (section 3.5).
				 * 
				 * A server which receives an entity-body with a transfer-coding it does not
				 * understand SHOULD return 501 (Unimplemented), and close the connection.
				 * A server MUST NOT send transfer-codings to an HTTP/1.0 client.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 3.6.1
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The chunked encoding modifies the body of a message in order to transfer
				 * it as a series of chunks, each with its own size indicator, followed by
				 * an OPTIONAL trailer containing entity-header fields. This allows dynamically
				 * produced content to be transferred along with the information necessary for
				 * the recipient to verify that it has received the full message.
				 */
				boost::spirit::rule< Scanner > Chunked_Body_;
				boost::spirit::rule< Scanner > chunk_;
				boost::spirit::rule< Scanner > chunk_size_p_;
				std::size_t chunk_size_;
				boost::spirit::rule< Scanner > last_chunk_;
				boost::spirit::rule< Scanner > chunk_extension_;
				boost::spirit::rule< Scanner > chunk_ext_name_;
				boost::spirit::rule< Scanner > chunk_ext_val_;
				boost::spirit::rule< Scanner > chunk_data_;
				boost::spirit::rule< Scanner > trailer_;
				/*
				 * The chunk-size field is a string of hex digits indicating the 
				 * size of the chunk. The chunked encoding is ended by any chunk 
				 * whose size is zero, followed by the trailer, which is terminated
				 * by an empty line.
				 *
				 * The trailer allows the sender to include additional HTTP header 
				 * fields at the end of the message. The Trailer header field can 
				 * be used to indicate which header fields are included in a trailer
				 * (see section 14.40).
				 * 
				 * A server using chunked transfer-coding in a response MUST NOT use
				 * the trailer for any header fields unless at least one of the
				 * following is true:
				 * a) the request included a TE header field that indicates “trailers”
				 *    is acceptable in the transfer-coding of the response, as described
				 *    in section 14.39; or,
				 * b) the server is the origin server for the response, the trailer
				 *    fields consist entirely of optional metadata, and the recipient
				 *    could use the message (in a manner acceptable to the origin
				 *    server) without receiving this metadata. In other words, the
				 *    origin server is willing to accept the possibility that the trailer
				 *    fields might be silently discarded along the path to the client.
				 * This requirement prevents an interoperability failure when the message
				 * is being received by an HTTP/1.1 (or later) proxy and forwarded to an
				 * HTTP/1.0 recipient. It avoids a situation where compliance with the
				 * protocol would have necessitated a possibly infinite buffer on the
				 * proxy.
				 *
				 * An example process for decoding a Chunked-Body is presented in appendix
				 * 19.4.6.
				 *
				 * All HTTP/1.1 applications MUST be able to receive and decode the
				 * “chunked” transfer-coding, and MUST ignore chunk-extension extensions
				 * they do not understand.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 3.7
				//////////////////////////////////////////////////////////////////////////
				/* 
				 * HTTP uses Internet Media Types [17] in the Content-Type (section 14.17) 
				 * and Accept (section 14.1) header fields in order to provide open and 
				 * extensible data typing and type negotiation. 
				 */
				boost::spirit::rule< Scanner > media_type_;
				boost::spirit::rule< Scanner > type_;
				boost::spirit::rule< Scanner > subtype_;
				/*
				 * Parameters MAY follow the type/subtype in the form of attribute/value 
				 * pairs (as defined in section 3.6).
				 *
				 * The type, subtype, and parameter attribute names are case-insensitive. 
				 * Parameter values might or might not be casesensitive, depending on the 
				 * semantics of the parameter name. Linear white space (LWS) MUST NOT be 
				 * used between the type and subtype, nor between an attribute and its 
				 * value. The presence or absence of a parameter might be significant to 
				 * the processing of a media-type, depending on its definition within the 
				 * media type registry.
				 *
				 * Note that some older HTTP applications do not recognize media type 
				 * parameters. When sending data to older HTTP applications, implementations 
				 * SHOULD only use media type parameters when they are required by that 
				 * type/subtype definition.
				 *
				 * Media-type values are registered with the Internet Assigned Number 
				 * Authority (IANA [19]). The media type registration process is outlined 
				 * in RFC 1590 [17]. Use of non-registered media types is discouraged.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 3.8
				//////////////////////////////////////////////////////////////////////////
				/*
				 * Product tokens are used to allow communicating applications to identify 
				 * themselves by software name and version.
				 *
				 * Most fields using product tokens also allow sub-products which form a 
				 * significant part of the application to be listed, separated by white 
				 * space. By convention, the products are listed in order of their 
				 * significance for identifying the application.
				 */
				boost::spirit::rule< Scanner > product_;
				boost::spirit::rule< Scanner > product_version_;
				/*
				 * Examples:
				 *     User-Agent: CERN-LineMode/2.15 libwww/2.17b3
				 *     Server: Apache/0.8.4
				 * Product tokens SHOULD be short and to the point. They MUST NOT be used for 
				 * advertising or other non-essential information. Although any token character 
				 * MAY appear in a product-version, this token SHOULD only be used for a version 
				 * identifier (i.e., successive versions of the same product SHOULD only differ 
				 * in the productversion portion of the product value).
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 3.9
				//////////////////////////////////////////////////////////////////////////
				/*
				 * HTTP content negotiation (section 12) uses short “floating point” numbers 
				 * to indicate the relative importance (“weight”) of various negotiable 
				 * parameters. A weight is normalized to a real number in the range 0 
				 * through 1, where 0 is the minimum and 1 the maximum value. If a parameter 
				 * has a quality value of 0, then content with this parameter is ‘not acceptable’
				 * for the client. HTTP/1.1 applications MUST NOT generate more than three 
				 * digits after the decimal point. User configuration of these values SHOULD 
				 * also be limited in this fashion.
				 */
				boost::spirit::rule< Scanner > qvalue_;
				/*
				 * “Quality values” is a misnomer, since these values merely represent relative 
				 * degradation in desired quality.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 3.10
				//////////////////////////////////////////////////////////////////////////
				/*
				 * A language tag identifies a natural language spoken, written, or otherwise 
				 * conveyed by human beings for communication of information to other human 
				 * beings. Computer languages are explicitly excluded. HTTP uses language 
				 * tags within the Accept-Language and Content-Language fields.
				 *
				 * The syntax and registry of HTTP language tags is the same as that defined 
				 * by RFC 1766 [1]. In summary, a language tag is composed of 1 or more 
				 * parts: A primary language tag and a possibly empty series of subtags:
				 */
				boost::spirit::rule< Scanner > language_tag_;
				boost::spirit::rule< Scanner > primary_tag_;
				boost::spirit::rule< Scanner > subtag_;
				/*
				 * White space is not allowed within the tag and all tags are 
				 * case-insensitive. The name space of language tags is administered by the 
				 * IANA. Example tags include:
				 *     en, en-US, en-cockney, i-cherokee, x-pig-latin
				 * where any two-letter primary-tag is an ISO-639 language abbreviation and 
				 * any two-letter initial subtag is an ISO-3166 country code. (The last 
				 * three tags above are not registered tags; all but the last are examples 
				 * of tags which could be registered in future.)
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 3.11
				//////////////////////////////////////////////////////////////////////////
				/*
				 * Entity tags are used for comparing two or more entities from the same 
				 * requested resource. HTTP/1.1 uses entity tags in the ETag (section 
				 * 14.19), If-Match (section 14.24), If-None-Match (section 14.26), and 
				 * If-Range (section 14.27) header fields. The definition of how they are 
				 * used and compared as cache validators is in section 13.3.3. An entity 
				 * tag consists of an opaque quoted string, possibly prefixed by a weakness 
				 * indicator.
				 */
				boost::spirit::rule< Scanner > entity_tag_;
				boost::spirit::rule< Scanner > weak_;
				boost::spirit::rule< Scanner > opaque_tag_;
				/*
				 * A “strong entity tag” MAY be shared by two entities of a resource only 
				 * if they are equivalent by octet equality.
				 *
				 * A “weak entity tag,” indicated by the "W/" prefix, MAY be shared by two 
				 * entities of a resource only if the entities are equivalent and could be 
				 * substituted for each other with no significant change in semantics. A 
				 * weak entity tag can only be used for weak comparison.
				 *
				 * An entity tag MUST be unique across all versions of all entities associated 
				 * with a particular resource. A given entity tag value MAY be used for 
				 * entities obtained by requests on different URIs. The use of the same 
				 * entity tag value in conjunction with entities obtained by requests on 
				 * different URIs does not imply the equivalence of those entities.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 3.12
				//////////////////////////////////////////////////////////////////////////
				/*
				 * HTTP/1.1 allows a client to request that only part (a range of) the 
				 * response entity be included within the response.
				 *
				 * HTTP/1.1 uses range units in the Range (section 14.35) and Content-Range 
				 * (section 14.16) header fields. An entity can be broken down into subranges 
				 * according to various structural units.
				 */
				boost::spirit::rule< Scanner > range_unit_;
				boost::spirit::rule< Scanner > bytes_unit_;
				boost::spirit::rule< Scanner > other_range_unit_;
				/*
				 * The only range unit defined by HTTP/1.1 is “bytes”. HTTP/1.1 implementations 
				 * MAY ignore ranges specified using other units. HTTP/1.1 has been 
				 * designed to allow implementations of applications that do not depend 
				 * on knowledge of ranges.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 4.1
				//////////////////////////////////////////////////////////////////////////
				/*
				 * HTTP messages consist of requests from client to server and responses 
				 * from server to client.
				 */
				boost::spirit::rule< Scanner > HTTP_message_;
				/*
				 * Request (section 5) and Response (section 6) messages use the generic 
				 * message format of RFC 822 [9] for transferring entities (the payload 
				 * of the message). Both types of message consist of a start-line, zero 
				 * or more header fields (also known as “headers”), an empty line (i.e.,
				 * a line with nothing preceding the CRLF) indicating the end of the header
				 * fields, and possibly a message-body.
				 */
				boost::spirit::rule< Scanner > generic_message_;
				boost::spirit::rule< Scanner > start_line_;
				/*
				 * In the interest of robustness, servers SHOULD ignore any empty line(s) 
				 * received where a Request-Line is expected. In other words, if the server 
				 * is reading the protocol stream at the beginning of a message and receives a
				 * CRLF first, it should ignore the CRLF.
				 *
				 * Certain buggy HTTP/1.0 client implementations generate extra CRLF’s after 
				 * a POST request. To restate what is explicitly forbidden by the BNF, an 
				 * HTTP/1.1 client MUST NOT preface or follow a request with an extra CRLF. 
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 4.2
				//////////////////////////////////////////////////////////////////////////
				/*
				 * HTTP header fields, which include general-header (section 4.5), 
				 * request-header (section 5.3), response-header (section 6.2), and 
				 * entity-header (section 7.1) fields, follow the same generic format as 
				 * that given in Section 3.1 of RFC 822 [9]. Each header field consists of 
				 * a name followed by a colon (“:”) and the field value. Field names are 
				 * case-insensitive. The field value MAY be preceded by any amount of LWS, 
				 * though a single SP is preferred. Header fields can be extended over 
				 * multiple lines by preceding each extra line with at least one SP or HT. 
				 * Applications ought to follow “common form”, where one is known or 
				 * indicated, when generating HTTP constructs, since there might exist 
				 * some implementations that fail to accept anything beyond the common 
				 * forms.
				 */
				boost::spirit::rule< Scanner > message_header_;
				boost::spirit::rule< Scanner > field_name_;
				boost::spirit::rule< Scanner > field_value_;
				boost::spirit::rule< Scanner > field_content_;
				/*
				 * The field-content does not include any leading or trailing LWS: linear 
				 * white space occurring before the first non-whitespace character of the 
				 * field-value or after the last non-whitespace character of the field-value.
				 * Such leading or trailing LWS MAY be removed without changing the 
				 * semantics of the field value. Any LWS that occurs between field-content 
				 * MAY be replaced with a single SP before interpreting the field value or
				 * forwarding the message downstream.
				 *
				 * The order in which header fields with differing field names are received 
				 * is not significant. However, it is “good practice” to send general-header 
				 * fields first, followed by request-header or response-header fields, and 
				 * ending with the entity-header fields.
				 *
				 * Multiple message-header fields with the same field-name MAY be present 
				 * in a message if and only if the entire field-value for that header field 
				 * is defined as a comma-separated list [i.e., #(values)]. It MUST be possible
				 * to combine the multiple header fields into one “field-name: field-value” 
				 * pair, without changing the semantics of the message, by appending each 
				 * subsequent field-value to the first, each separated by a comma. The order 
				 * in which header fields with the same field-name are received is therefore 
				 * significant to the interpretation of the combined field value, and thus a 
				 * proxy MUST NOT change the order of these field values when a message is 
				 * forwarded. (This means we should use an associative vector for the headers, 
				 * and not a map /rlc)
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 4.3
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The message-body (if any) of an HTTP message is used to carry the 
				 * entity-body associated with the request or response. The message-body 
				 * differs from the entity-body only when a transfer-coding has been 
				 * applied, as indicated by the Transfer-Encoding header field (section 
				 * 14.41).
				 */
				boost::spirit::rule< Scanner > message_body_;
				/*
				 * Transfer-Encoding MUST be used to indicate any transfer-codings applied 
				 * by an application to ensure safe and proper transfer of the message. 
				 * Transfer-Encoding is a property of the message, not of the entity, and 
				 * thus MAY be added or removed by any application along the request/response 
				 * chain. (However, section 3.6 places restrictions on when certain 
				 * transfer-codings may be used.)
				 */
				boost::spirit::rule< Scanner > target_;
			};
		};
	}
}

#endif


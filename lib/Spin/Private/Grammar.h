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
					general_header_	= Cache_Control_		// Section 14.9
									| Connection_			// Section 14.10
									| Date_					// Section 14.18
									| Pragma_				// Section 14.32
									| Trailer_				// Section 14.40
									| Transfer_Encoding_	// Section 14.41
									| Upgrade_				// Section 14.42
									| Via_					// Section 14.45
									| Warning_				// Section 14.46
									;
					Request_		= Request_Line_ >> *((general_header_ | request_header_ | entity_header_) >> CRLF_) >> CRLF_ >> !message_body_
									;
					Request_Line_	= Method_ >> SP_ >> Request_URI_ >> SP_ >> HTTP_version_ >> CRLF_
									;
					Method_			= str_p("OPTIONS")		// Section 9.2
									| "GET"					// Section 9.3
									| "HEAD"				// Section 9.4
									| "POST"				// Section 9.5
									| "PUT"					// Section 9.6
									| "DELETE"				// Section 9.7
									| "TRACE"				// Section 9.8
									| "CONNECT"				// Section 9.9
									| extension_method_
									;
					extension_method_
									= token_
									; 
					Request_URI_	= '*'
									| absoluteURI_
									| abs_path_
									: authority_
									;
					request_header_	= Accept_				// Section 14.1
									| Accept_Charset_		// Section 14.2
									| Accept_Encoding_		// Section 14.3
									| Accept_Language_		// Section 14.4
									| Authorization_		// Section 14.8
									| Expect_				// Section 14.20
									| From_					// Section 14.22
									| Host_					// Section 14.23
									| If_Match_				// Section 14.24
									| If_Modified_Since_	// Section 14.25
									| If_None_Match_		// Section 14.26
									| If_Range_				// Section 14.27
									| If_Unmodified_Since_	// Section 14.28
									| Max_Forwards_			// Section 14.31
									| Proxy_Authorization_	// Section 14.34
									| Range_				// Section 14.35
									| Referer_				// Section 14.36
									| TE_					// Section 14.39
									| User_Agent_			// Section 14.43
									;
					Response_		= Status_Line_ >> *((general_header_ | response_header_ | entity_header_) >> CRLF_) >> CRLF_ >> !message_body_
									;
					Status_Line_	= HTTP_version_ >> SP_ >> Status_Code_ >> SP_ >> Reason_Phrase_ >> CRLF_
									;
					Status_Code_	= str_p("100")			// Section 10.1.1: Continue
									| "101"					// Section 10.1.2: Switching Protocols
									| "200"					// Section 10.2.1: OK
									| "201"					// Section 10.2.2: Created
									| "202"					// Section 10.2.3: Accepted
									| "203"					// Section 10.2.4: Non-Authoritative Information
									| "204"					// Section 10.2.5: No Content
									| "205"					// Section 10.2.6: Reset Content
									| "206"					// Section 10.2.7: Partial Content
									| "300"					// Section 10.3.1: Multiple Choices
									| "301"					// Section 10.3.2: Moved Permanently
									| "302"					// Section 10.3.3: Found
									| "303"					// Section 10.3.4: See Other
									| "304"					// Section 10.3.5: Not Modified
									| "305"					// Section 10.3.6: Use Proxy
									| "307"					// Section 10.3.8: Temporary Redirect
									| "400"					// Section 10.4.1: Bad Request
									| "401"					// Section 10.4.2: Unauthorized
									| "402"					// Section 10.4.3: Payment Required
									| "403"					// Section 10.4.4: Forbidden
									| "404"					// Section 10.4.5: Not Found
									| "405"					// Section 10.4.6: Method Not Allowed
									| "406"					// Section 10.4.7: Not Acceptable
									| "407"					// Section 10.4.8: Proxy Authentication Required
									| "408"					// Section 10.4.9: Request Time-out
									| "409"					// Section 10.4.10: Conflict
									| "410"					// Section 10.4.11: Gone
									| "411"					// Section 10.4.12: Length Required
									| "412"					// Section 10.4.13: Precondition Failed
									| "413"					// Section 10.4.14: Request Entity Too Large
									| "414"					// Section 10.4.15: Request-URI Too Large
									| "415"					// Section 10.4.16: Unsupported Media Type
									| "416"					// Section 10.4.17: Requested range not satisfiable
									| "417"					// Section 10.4.18: Expectation Failed
									| "500"					// Section 10.5.1: Internal Server Error
									| "501"					// Section 10.5.2: Not Implemented
									| "502"					// Section 10.5.3: Bad Gateway
									| "503"					// Section 10.5.4: Service Unavailable
									| "504"					// Section 10.5.5: Gateway Time-out
									| "505"					// Section 10.5.6: HTTP Version not supported
									| extension_code_
					extension_code_	= repeat_p(3)[DIGIT_]
									;
					Reason_Phrase_	= *(TEXT_ - CR_ - LF_)
									;
					response_header_
									= Accept_Ranges_		// Section 14.5
									| Age_					// Section 14.6
									| ETag_					// Section 14.19
									| Location_				// Section 14.30
									| Proxy_Authenticate_	// Section 14.33
									| Retry_After_			// Section 14.37
									| Server_				// Section 14.38
									| Vary_					// Section 14.44
									| WWW_Authenticate_		// Section 14.47
									;
					entity_header_	= Allow_				// Section 14.7
									| Content_Encoding_		// Section 14.11
									| Content_Language_		// Section 14.12
									| Content_Length_		// Section 14.13
									| Content_Location_		// Section 14.14
									| Content_MD5_			// Section 14.15
									| Content_Range_		// Section 14.16
									| Content_Type_			// Section 14.17
									| Expires_				// Section 14.21
									| Last_Modified_		// Section 14.29
									| extension_header_
									;
					extension_header_
									= message_header_
									;
					Accept_			= "Accept" >> ':' >> !((*LWS_ >> media_range_ >> *LWS_ >> !accept_params_ >> *LWS_) % ',')
									;
					media_range_	= ( "*/*" 
									  | (type_ >> '/' >> '*')
									  | (type_ >> '/' >> subtype_)
									  ) >> *(';' >> parameter_)
									;
					accept_params_	= ch_p(';') >> 'q' >> '=' >> qvalue_ >> *accept_extension_
									;
					accept_extension_
									= ';' >> token_ >> !('=' >> (token_ | quoted_string_))
									;
					Accept_Charset_	= str_p("Accept-Charset") >> ':' >> ((*LWS_ >> (charset_ | '*') >. *LWS_ >> !(*LWS_ >. ch_p(';') >> *LWS_ >> 'q' >> *LWS_ >> '=' >> *LWS_ >> qvalue_ >> *LWS_)) % ',')
									;
					Accept_Encoding_
									= str_p("Accept-Encoding") >> ':' >> ((*LWS_ >> codings_ >> *LWS_ >> !(*LWS_ >> ch_p(';') >> *LWS_ >> 'q' >> *LWS_ >> '=' >> *LWS_ >> qvalue_ >> *LWS_)) % ',')
									;
					codings_		= content_coding_ | '*'
									;
					Accept_Language_= str_p("Accept-Language") >> ':' >> ((*LWS_ >> language_range_ >> *LWS_ >> !(*LWS_ >> ch_p(';') >> *LWS_ >> 'q' >> *LWS_ >> '=' >> *LWS_ >> qvalue_ >> *LWS_)) % ',')
									;
					language_range_	= repeat_p(1,8)[ALPHA_] >> *('-' >> repeat_p(1,8)[ALPHA_])
									| '*'
									;
					Accept_Ranges_	= str_p("Accept-Ranges") >> ':' >> acceptable_ranges_
									;
					acceptable_ranges_
									= ((*LWS_ >> range_unit_ >> *LWS_) % ',') 
									| (*LWS_ >> "none" >> *LWS_)
									;
					Age_			= str_p("Age") >> ':' >> age_value_
									;
					age_value_		= delta_seconds_
									;
					Allow_			= str_p("Allow") >> ':' >> ((*LWS_ >> Method_ *LWS_) % ',')
									;
					Authorization_	= str_p("Authorization") >> ':' >> credentials_
									;
					Cache_Control_	= str_p("Cache-Control") >> ':' ((*LWS_ >> cache_directive_ >> *LWS_) % ',')
									;
					cache_directive_= cache_request_directive_
									| cache_response_directive_
									;
					cache_request_directive_
									= str_p("no-cache")											// Section 14.9.1
									| "no-store"												// Section 14.9.2
									| (str_p("max-age") >> '=' >> delta_seconds_)				// Section 14.9.3, 14.9.4
									| (str_p("max-stale") >> !(ch_p('=') >> delta_seconds_))	// Section 14.9.3
									| (str_p("min-fresh") >> !(ch_p('=') >> delta_seconds_))	// Section 14.9.3
									| "no-transform"											// Section 14.9.5
									| "only-if-cached"											// Section 14.9.4
									| cache_extension_											// Section 14.9.6
									;
					cache_response_directive_
									= str_p("public")											// Section 14.9.1
									| (str_p("private") !( '=' >> ch_p('"') ((*LWS_ >> field_name_ >> *LWS_) % ',') ch_p('"') ))	// Section 14.9.1
									| ("no-cache" [ "=" <"> 1#field-name <"> ])					// Section 14.9.1
									| "no-store"												// Section 14.9.2
									| "no-transform"											// Section 14.9.5
									| "must-revalidate"											// Section 14.9.4
									| "proxy-revalidate"										// Section 14.9.4
									| (str_p("max-age") >> '=' >> delta_seconds_)				// Section 14.9.3
									| (str_p("s-maxage") >> '=' >> delta_seconds_)				// Section 14.9.3
									| cache_extension_											// Section 14.9.6
									;
					cache_extension_= token_ >> !( '=' >> ( token_ | quoted_string_ ) )
									;
					Connection_		= str_p("Connection") >> ':' >> ((*LWS_ >> connection_token_ >> *LWS_) % ',')
									;
					connection_token_
									= token_
									;
					Content_Encoding_
									= str_p("Content-Encoding") >> ':' >> ((*LWS_ >> content_coding_ >> *LWS_) % ',')
									;
					Content_Language_
									= str_p("Content-Language") >> ':' >> ((*LWS_ >> language_tag_ >> *LWS_) % ',')
									;
					Content_Length_	= str_p("Content-Length") >> ';' >> +DIGIT_
									;
					Content_Location_
									= str_p("Content-Location") >> ':' >> (absoluteURI | relativeURI)
									;
					Content_MD5_	= str_p("Content-MD5") >> ';' >> md5_digest_
									;
					md5_digest_		= repeat_p(32)[HEX_]
									;
					Content_Range_	= str_p("Content-Range") >> ':' >> content_range_spec_
									;
					content_range_spec_
									= byte_content_range_spec_
									;
					byte_content_range_spec_
									= bytes_unit_ >> SP_ >> byte_range_resp_spec_ >> '/' >> (instance_length_ | '*')
									;
					byte_range_resp_spec_
									= (first_byte_pos_ >> '-' >> last_byte_pos_)
									| '*'
									;
					instance_length_= +DIGIT_
									;
					Content_Type_	= str_p("Content-Type") >> ':' >> media_type_
									;
					Date_			= str_p("Date") >> ':' >> HTTP_date_
									;
					ETag_			= str_p("ETag") >> ':' >> entity_tag_
									;
					Expect_			= ((*LWS_ >> expectation_ >> *LWS_) % ',')
									;
					expectation_	= "100-continue"
									| expectation_extension_
									;
					expectation_extension_
									= token >> !('=' >> (token_ | quoted_string_) >> *expect_params_)
									;
					expect_params_	= ';' >> token_ >. !('=' >> (token_ | quoted_string_))
									;
					Expires_		= str_p("Expires") >> ':' >> HTTP_date_
									;
					From_			= str_p("From") >> ':' >> mailbox_
									;
					Host_			= str_p("Host") >> ':' >> host_ >> !(':' >> port_)
									;
					If_Match_		= str_p("If-Match") >> ':' >> ('*' | ((*LWS_ >> entity_tag_ >> *LWS_) % ','))
									;
					If_Modified_Since_
									= str_p("If-Modified-Since") >> ':' >> HTTP_date_
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
				 *
				 * The rules for when a message-body is allowed in a message differ for 
				 * requests and responses.
				 *
				 * The presence of a message-body in a request is signaled by the inclusion 
				 * of a Content-Length or Transfer-Encoding header field in the request’s 
				 * message-headers. A message-body MUST NOT be included in a request if
				 * the specification of the request method (section 5.1.1) does not allow 
				 * sending an entity-body in requests. A server SHOULD read and forward a 
				 * message-body on any request; if the request method does not include defined
				 * semantics for an entity-body, then the message-body SHOULD be ignored 
				 * when handling the request.
				 *
				 * For response messages, whether or not a message-body is included with a 
				 * message is dependent on both the request method and the response status 
				 * code (section 6.1.1). All responses to the HEAD request method MUST NOT
				 * include a message-body, even though the presence of entity-header fields 
				 * might lead one to believe they do. All 1xx (informational), 204 (no content), 
				 * and 304 (not modified) responses MUST NOT include a message-body. All other
				 * responses do include a message-body, although it MAY be of zero length.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 4.5
				//////////////////////////////////////////////////////////////////////////
				/*
				 * There are a few header fields which have general applicability for both 
				 * request and response messages, but which do not apply to the entity 
				 * being transferred. These header fields apply only to the message being 
				 * transmitted.
				 */
				boost::spirit::rule< Scanner > general_header_;
				/*
				 * General-header field names can be extended reliably only in combination 
				 * with a change in the protocol version. However, new or experimental 
				 * header fields may be given the semantics of general header fields if all 
				 * parties in the communication recognize them to be general-header fields.
				 * Unrecognized header fields are treated as entity-header fields.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 5
				//////////////////////////////////////////////////////////////////////////
				/*
				 * A request message from a client to a server includes, within the first 
				 * line of that message, the method to be applied to the resource, the
				 * identifier of the resource, and the protocol version in use.
				 */
				boost::spirit::rule< Scanner > Request_;
				//////////////////////////////////////////////////////////////////////////
				// Section 5.1
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Request-Line begins with a method token, followed by the Request-URI 
				 * and the protocol version, and ending with CRLF. The elements are 
				 * separated by SP characters. No CR or LF is allowed except in the final 
				 * CRLF sequence.
				 */
				boost::spirit::rule< Scanner > Request_Line_;
				//////////////////////////////////////////////////////////////////////////
				// Section 5.1.1
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Method token indicates the method to be performed on the resource 
				 * identified by the Request-URI. The method is case-sensitive.
				 */
				boost::spirit::rule< Scanner > Method_;
				boost::spirit::rule< Scanner > extension_method_;
				/*
				 * The list of methods allowed by a resource can be specified in an Allow 
				 * header field (section 14.7). The return code of the response always
				 * notifies the client whether a method is currently allowed on a resource,
				 * since the set of allowed methods can change dynamically. An origin 
				 * server SHOULD return the status code 405 (Method Not Allowed) if the
				 * method is known by the origin server but not allowed for the requested
				 * resource, and 501 (Not Implemented) if the method is unrecognized or
				 * not implemented by the origin server. The methods GET and HEAD MUST be
				 * supported by all general-purpose servers. All other methods are
				 * OPTIONAL; however, if the above methods are implemented, they MUST be
				 * implemented with the same semantics as those specified in section 9.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 5.1.2
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Request-URI is a Uniform Resource Identifier (section 3.2) and
				 * identifies the resource upon which to apply the request.
				 */
				boost::spirit::rule< Scanner > Request_URI_;
				/*
				 * The four options for Request-URI are dependent on the nature of the 
				 * request. The asterisk “*” means that the request does not apply to a
				 * particular resource, but to the server itself, and is only allowed
				 * when the method used does not necessarily apply to a resource. One
				 * example would be
				 *     OPTIONS * HTTP/1.1
				 * The absoluteURI form is REQUIRED when the request is being made to 
				 * a proxy. The proxy is requested to forward the request or service 
				 * it from a valid cache, and return the response. Note that the proxy
				 * MAY forward the request on to another proxy or directly to the server
				 * specified by the absoluteURI. In order to avoid request loops, a
				 * proxy MUST be able to recognize all of its server names, including
				 * any aliases, local variations, and the numeric IP address. An example
				 * Request-Line would be:
				 *     GET http://www.w3.org/pub/WWW/TheProject.html HTTP/1.1
				 * To allow for transition to absoluteURIs in all requests in future 
				 * versions of HTTP, all HTTP/1.1 servers MUST accept the absoluteURI
				 * form in requests, even though HTTP/1.1 clients will only generate 
				 * them in requests to proxies.
				 *
				 * The authority form is only used by the CONNECT method (section 9.9).
				 *
				 * The most common form of Request-URI is that used to identify a 
				 * resource on an origin server or gateway. In this case the absolute 
				 * path of the URI MUST be transmitted (see section 3.2.1, abs_path) 
				 * as the Request-URI, and the network location of the URI (authority) 
				 * MUST be transmitted in a Host header field. For example, a client
				 * wishing to retrieve the resource above directly from the origin 
				 * server would create a TCP connection to port 80 of the host 
				 * “www.w3.org” and send the lines: 
				 *     GET /pub/WWW/TheProject.html HTTP/1.1
				 *     Host: www.w3.org
				 * followed by the remainder of the Request. Note that the absolute path 
				 * cannot be empty; if none is present in the original URI, it MUST 
				 * be given as “/” (the server root).
				 *
				 * The Request-URI is transmitted in the format specified in section 
				 * 3.2.1. If the Request-URI is encoded using the “% HEX HEX” encoding
				 * [42], the origin server MUST decode the Request-URI in order to 
				 * properly interpret the request. Servers SHOULD respond to invalid 
				 * Request-URIs with an appropriate status code.
				 *
				 * A transparent proxy MUST NOT rewrite the “abs_path” part of the 
				 * received Request-URI when forwarding it to the next inbound server,
				 * except as noted above to replace a null abs_path with “/”.
				 * 
				 *     Note: The “no rewrite” rule prevents the proxy from changing the 
				 *           meaning of the request when the origin server is improperly 
				 *           using a non-reserved URI character for a reserved purpose.
				 *           Implementors should be aware that some pre-HTTP/1.1 proxies 
				 *           have been known to rewrite the Request-URI.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 5.3
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The request-header fields allow the client to pass additional information 
				 * about the request, and about the client itself, to the server. These 
				 * fields act as request modifiers, with semantics equivalent to the 
				 * parameters on a programming language method invocation.
				 */
				boost::spirit::rule< Scanner > request_header_;
				/* 
				 * Request-header field names can be extended reliably only in combination 
				 * with a change in the protocol version. However, new or experimental 
				 * header fields MAY be given the semantics of request-header fields if 
				 * all parties in the communication recognize them to be request-header 
				 * fields. Unrecognized header fields are treated as entity-header fields.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 6
				//////////////////////////////////////////////////////////////////////////
				/*
				 * After receiving and interpreting a request message, a server responds 
				 * with an HTTP response message.
				 */
				boost::spirit::rule< Scanner > Response_;
				//////////////////////////////////////////////////////////////////////////
				// Section 6.1
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The first line of a Response message is the Status-Line, consisting of 
				 * the protocol version followed by a numeric status code and its 
				 * associated textual phrase, with each element separated by SP characters.
				 * No CR or LF is allowed except in the final CRLF sequence.
				 */
				boost::spirit::rule< Scanner > Status_Line_;
				//////////////////////////////////////////////////////////////////////////
				// Section 6.1.1
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Status-Code element is a 3-digit integer result code of the attempt 
				 * to understand and satisfy the request. These codes are fully defined in 
				 * section 10. The Reason-Phrase is intended to give a short textual 
				 * description of the Status-Code. The Status-Code is intended for use by 
				 * automata and the Reason-Phrase is intended for the human user. The client
				 * is not required to examine or display the Reason-Phrase.
				 *
				 * The first digit of the Status-Code defines the class of response. The 
				 * last two digits do not have any categorization role. There are 5 values 
				 * for the first digit:
				 * · 1xx: Informational - Request received, continuing process
				 * · 2xx: Success - The action was successfully received, understood, and accepted
				 * · 3xx: Redirection - Further action must be taken in order to complete the request
				 * · 4xx: Client Error - The request contains bad syntax or cannot be fulfilled
				 * · 5xx: Server Error - The server failed to fulfill an apparently valid request
				 * The individual values of the numeric status codes defined for HTTP/1.1,
				 * and an example set of corresponding Reason-Phrase’s, are presented below.
				 * The reason phrases listed here are only recommendations -- they MAY be 
				 * replaced by local equivalents without affecting the protocol.
				 */
				boost::spirit::rule< Scanner > Status_Code_;
				boost::spirit::rule< Scanner > extension_code_;
				boost::spirit::rule< Scanner > Reason_Phrase_;
				/*
				 * HTTP status codes are extensible. HTTP applications are not required to 
				 * understand the meaning of all registered status codes, though such 
				 * understanding is obviously desirable. However, applications MUST understand 
				 * the class of any status code, as indicated by the first digit, and 
				 * treat any unrecognized response as being equivalent to the x00 status 
				 * code of that class, with the exception that an unrecognized response MUST 
				 * NOT be cached. For example, if an unrecognized status code of 431 is 
				 * received by the client, it can safely assume that there was something 
				 * wrong with its request and treat the response as if it had received a
				 * 400 status code. In such cases, user agents SHOULD present to the user 
				 * the entity returned with the response, since that entity is likely to
				 * include human-readable information which will explain the unusual status.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 6.2
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The response-header fields allow the server to pass additional 
				 * information about the response which cannot be placed in the Status-Line.
				 * These header fields give information about the server and about further
				 * access to the resource identified by the Request-URI.
				 */
				boost::spirit::rule< Scanner > response_header_;
				/*
				 * Response-header field names can be extended reliably only in combination 
				 * with a change in the protocol version. However, new or experimental header
				 * fields MAY be given the semantics of response-header fields if all
				 * parties in the communication recognize them to be response-header fields.
				 * Unrecognized header fields are treated as entityheader fields.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 7.1
				//////////////////////////////////////////////////////////////////////////
				/*
				 * Entity-header fields define metainformation about the entity-body or,
				 * if no body is present, about the resource identified by the request.
				 * Some of this metainformation is OPTIONAL; some might be REQUIRED by
				 * portions of this specification.
				 */
				boost::spirit::rule< Scanner > entity_header_;
				boost::spirit::rule< Scanner > extension_header_;
				/*
				 * The extension-header mechanism allows additional entity-header
				 * fields to be defined without changing the protocol, but these fields
				 * cannot be assumed to be recognizable by the recipient. Unrecognized
				 * header fields SHOULD be ignored by the recipient and MUST be forwarded
				 * by transparent proxies.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14
				//////////////////////////////////////////////////////////////////////////
				/*
				 * This section defines the syntax and semantics of all standard HTTP/1.1
				 * header fields. For entity-header fields, both sender and recipient 
				 * refer to either the client or the server, depending on who sends and 
				 * who receives the entity.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.1
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Accept request-header field can be used to specify certain media 
				 * types which are acceptable for the response. Accept headers can be used
				 * to indicate that the request is specifically limited to a small set of
				 * desired types, as in the case of a request for an in-line image.
				 */
				boost::spirit::rule< Scanner > Accept_;
				boost::spirit::rule< Scanner > media_range_;
				boost::spirit::rule< Scanner > accept_params_;
				boost::spirit::rule< Scanner > accept_extension_;
				/*
				 * The asterisk “*” character is used to group media types into ranges, 
				 * with “* /*” (space inserted to avoid the comment from being interrupted 
				 * /rlc) indicating all media types and “type/*” indicating all subtypes of 
				 * that type. The media-range MAY include media type parameters that are 
				 * applicable to that range.
				 *
				 * Each media-range MAY be followed by one or more accept-params, beginning 
				 * with the “q” parameter for indicating a relative quality factor. The 
				 * first “q” parameter (if any) separates the media-range parameter(s) from 
				 * the accept-params. Quality factors allow the user or user agent to 
				 * indicate the relative degree of preference for that media-range, using 
				 * the qvalue scale from 0 to 1 (section 3.9). The default value is q=1.
				 *
				 * Note: Use of the “q” parameter name to separate media type parameters from 
				 *       Accept extension parameters is due to historical practice. Although 
				 *       this prevents any media type parameter named “q” from being used with 
				 *       a media range, such an event is believed to be unlikely given the lack 
				 *       of any “q” parameters in the IANA media type registry and the rare 
				 *       usage of any media type parameters in Accept. Future media types are 
				 *       discouraged from registering any parameter named “q”.
				 *
				 * The example
				 *     Accept: audio/*; q=0.2, audio/basic
				 * SHOULD be interpreted as “I prefer audio/basic, but send me any audio type 
				 * if it is the best available after an 80% mark-down in quality.”
				 *
				 * If no Accept header field is present, then it is assumed that the client 
				 * accepts all media types. If an Accept header field is present, and if the 
				 * server cannot send a response which is acceptable according to the combined 
				 * Accept field value, then the server SHOULD send a 406 (not acceptable) 
				 * response.
				 *
				 * A more elaborate example is
				 *     Accept: text/plain; q=0.5, text/html,
				 *             text/x-dvi; q=0.8, text/x-c
				 * Verbally, this would be interpreted as “text/html and text/x-c are the 
				 * preferred media types, but if they do not exist, then send the text/x-dvi 
				 * entity, and if that does not exist, send the text/plain entity.”
				 *
				 * Media ranges can be overridden by more specific media ranges or specific 
				 * media types. If more than one media range applies to a given type, the 
				 * most specific reference has precedence. For example,
				 *     Accept: text/*, text/html, text/html;level=1, * /*
				 * (space inserted /rlc) have the following precedence:
				 *     1) text/html;level=1
				 *     2) text/html
				 *     3) text/*
				 *     4) * /*
				 * (space insered again) The media type quality factor associated with a given 
				 * type is determined by finding the media range with the highest precedence 
				 * which matches that type. For example,
				 *     Accept: text/*;q=0.3, text/html;q=0.7, text/html;level=1,
				 *             text/html;level=2;q=0.4, * /*;q=0.5
				 * (space inserted /rlc) would cause the following values to be associated:
				 *     text/html;level=1 = 1
				 *     text/html = 0.7
				 *     text/plain = 0.3
				 *     image/jpeg = 0.5
				 *     text/html;level=2 = 0.4
				 *     text/html;level=3 = 0.7
				 * Note: A user agent might be provided with a default set of quality values 
				 *       for certain media ranges. However, unless the user agent is a closed 
				 *       system which cannot interact with other rendering agents, this
				 *       default set ought to be configurable by the user.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.2
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Accept-Charset request-header field can be used to indicate what 
				 * character sets are acceptable for the response. This field allows 
				 * clients capable of understanding more comprehensive or special-purpose 
				 * character sets to signal that capability to a server which is capable 
				 * of representing documents in those character sets.
				 */
				boost::spirit::rule< Scanner > Accept_Charset_;
				/* 
				 * Character set values are described in section 3.4. Each charset MAY be 
				 * given an associated quality value which represents the user’s preference 
				 * for that charset. The default value is q=1. An example is
				 *     Accept-Charset: iso-8859-5, unicode-1-1;q=0.8
				 * The special value “*”, if present in the Accept-Charset field, matches 
				 * every character set (including ISO-8859-1) which is not mentioned 
				 * elsewhere in the Accept-Charset field. If no “*” is present in an 
				 * Accept-Charset field, then all character sets not explicitly mentioned 
				 * get a quality value of 0, except for ISO-8859-1, which gets a quality 
				 * value of 1 if not explicitly mentioned.
				 *
				 * If no Accept-Charset header is present, the default is that any 
				 * character set is acceptable. If an Accept-Charset header is present, 
				 * and if the server cannot send a response which is acceptable according 
				 * to the Accept-Charset header, then the server SHOULD send an error 
				 * response with the 406 (not acceptable) status code, though the sending 
				 * of an unacceptable response is also allowed.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.3
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Accept-Encoding request-header field is similar to Accept, but 
				 * restricts the content-codings (section 3.5) that are acceptable in the 
				 * response.
				 */
				boost::spirit::rule< Scanner > Accept_Encoding_;
				boost::spirit::rule< Scanner > codings_;
				/*
				 * Examples of its use are:
				 *     Accept-Encoding: compress, gzip
				 *     Accept-Encoding:
				 *     Accept-Encoding: *
				 *     Accept-Encoding: compress;q=0.5, gzip;q=1.0
				 *     Accept-Encoding: gzip;q=1.0, identity; q=0.5, *;q=0
				 * (in my opinion, this is a defect in the specification: the line 
				 * "Accept-Encoding:" without anything behind should not be valid, according 
				 * to the BNF
				 *
				 * A server tests whether a content-coding is acceptable, according to an 
				 * Accept-Encoding field, using these rules:
				 * 1. If the content-coding is one of the content-codings listed in the 
				 *    Accept-Encoding field, then it is acceptable, unless it is accompanied 
				 *    by a qvalue of 0. (As defined in section 3.9, a qvalue of 0 means “not
				 *    acceptable.”)
				 * 2. The special “*” symbol in an Accept-Encoding field matches any available 
				 *    content-coding not explicitly listed in the header field.
				 * 3. If multiple content-codings are acceptable, then the acceptable 
				 *    content-coding with the highest non-zero qvalue is preferred.
				 * 4. The “identity” content-coding is always acceptable, unless specifically 
				 *    refused because the Accept-Encoding field includes “identity;q=0”, or 
				 *    because the field includes “*;q=0” and does not explicitly include the 
				 *    “identity” content-coding. If the Accept-Encoding field-value is empty,
				 *    then only the “identity” encoding is acceptable.
				 *
				 * If an Accept-Encoding field is present in a request, and if the server 
				 * cannot send a response which is acceptable according to the Accept-Encoding 
				 * header, then the server SHOULD send an error response with the 406 (Not
				 * Acceptable) status code.
				 *
				 * If no Accept-Encoding field is present in a request, the server MAY assume 
				 * that the client will accept any content coding. In this case, if “identity” 
				 * is one of the available content-codings, then the server SHOULD use the 
				 * “identity” content-coding, unless it has additional information that a 
				 * different content-coding is meaningful to the client.
				 *
				 * Note: If the request does not include an Accept-Encoding field, and if the
				 *       “identity” contentcoding is unavailable, then content-codings commonly 
				 *       understood by HTTP/1.0 clients (i.e., “gzip” and “compress”) are 
				 *       preferred; some older clients improperly display messages sent with 
				 *       other contentcodings.
				 *
				 *       The server might also make this decision based on information about 
				 *       the particular user-agent or client.
				 *
				 * Note: Most HTTP/1.0 applications do not recognize or obey qvalues associated 
				 *       with content-codings. This means that qvalues will not work and are 
				 *       not permitted with x-gzip or x-compress.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.4
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Accept-Language request-header field is similar to Accept, but restricts 
				 * the set of natural languages that are preferred as a response to the 
				 * request. Language tags are defined in section 3.10.
				 */
				boost::spirit::rule< Scanner > Accept_Language_;
				boost::spirit::rule< Scanner > language_range_;
				/*
				 * Each language-range MAY be given an associated quality value which 
				 * represents an estimate of the user’s preference for the languages 
				 * specified by that range. The quality value defaults to “q=1”. For 
				 * example,
				 *     Accept-Language: da, en-gb;q=0.8, en;q=0.7
				 * would mean: “I prefer Danish, but will accept British English and 
				 * other types of English.” A language-range matches a language-tag if 
				 * it exactly equals the tag, or if it exactly equals a prefix of the 
				 * tag such that the first tag character following the prefix is “-”. 
				 * The special range “*”, if present in the Accept-Language field, matches
				 * every tag not matched by any other range present in the 
				 * Accept-Language field.
				 *
				 * Note: This use of a prefix matching rule does not imply that language 
				 *       tags are assigned to languages in such a way that it is always 
				 *       true that if a user understands a language with a certain tag, 
				 *       then this user will also understand all languages with tags for 
				 *       which this tag is a prefix. The prefix rule simply allows the 
				 *       use of prefix tags if this is the case.
				 *
				 * The language quality factor assigned to a language-tag by the Accept-Language 
				 * field is the quality value of the longest language-range in the field that 
				 * matches the language-tag. If no language-range in the field matches the 
				 * tag, the language quality factor assigned is 0. If no Accept-Language 
				 * header is present in the request, the server SHOULD assume that all 
				 * languages are equally acceptable. If an Accept-Language header is present, 
				 * then all languages which are assigned a quality factor greater than 0 
				 * are acceptable.
				 *
				 * It might be contrary to the privacy expectations of the user to send an 
				 * Accept-Language header with the complete linguistic preferences of the 
				 * user in every request. For a discussion of this issue, see section 15.1.4.
				 *
				 * As intelligibility is highly dependent on the individual user, it is 
				 * recommended that client applications make the choice of linguistic 
				 * preference available to the user. If the choice is not made available, 
				 * then the Accept-Language header field MUST NOT be given in the request.
				 *
				 * Note: When making the choice of linguistic preference available to the 
				 *       user, we remind implementors of the fact that users are not familiar 
				 *       with the details of language matching as described above, and should
				 *       provide appropriate guidance. As an example, users might assume 
				 *       that on selecting “en-gb”, they will be served any kind of English 
				 *       document if British English is not available. A user agent might 
				 *       suggest in such a case to add “en” to get the best matching behavior.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.5
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Accept-Ranges response-header field allows the server to indicate 
				 * its acceptance of range requests for a resource:
				 */
				boost::spirit::rule< Scanner > Accept_Ranges_;
				boost::spirit::rule< Scanner > acceptable_ranges_;
				/*
				 * Origin servers that accept byte-range requests MAY send
				 *     Accept-Ranges: bytes
				 * but are not required to do so. Clients MAY generate byte-range requests 
				 * without having received this header for the resource involved. Range 
				 * units are defined in section 3.12.
				 *
				 * Servers that do not accept any kind of range request for a resource 
				 * MAY send
				 *     Accept-Ranges: none
				 * to advise the client not to attempt a range request.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.6
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Age response-header field conveys the sender's estimate of the 
				 * amount of time since the response (or its revalidation) was generated 
				 * at the origin server. A cached response is “fresh” if its age does not 
				 * exceed its freshness lifetime. Age values are calculated as specified 
				 * in section 13.2.3.
				 */
				boost::spirit::rule< Scanner > Age_;
				boost::spirit::rule< Scanner > age_value_;
				/*
				 * Age values are non-negative decimal integers, representing time in 
				 * seconds.
				 * 
				 * If a cache receives a value larger than the largest positive integer 
				 * it can represent, or if any of its age calculations overflows, it 
				 * MUST transmit an Age header with a value of 2147483648 (2^31). An 
				 * HTTP/1.1 server that includes a cache MUST include an Age header 
				 * field in every response generated from its own cache. Caches SHOULD 
				 * use an arithmetic type of at least 31 bits of range.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.7
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Allow entity-header field lists the set of methods supported by the 
				 * resource identified by the Request-URI.
				 * 
				 * The purpose of this field is strictly to inform the recipient of valid 
				 * methods associated with the resource. An Allow header field MUST be 
				 * present in a 405 (Method Not Allowed) response.
				 */
				boost::spirit::rule< Scanner > Allow_;
				/*
				 * Example of use:
				 *     Allow: GET, HEAD, PUT
				 * This field cannot prevent a client from trying other methods. However, 
				 * the indications given by the Allow header field value SHOULD be followed.
				 * The actual set of allowed methods is defined by the origin server at the 
				 * time of each request.
				 *
				 * The Allow header field MAY be provided with a PUT request to recommend 
				 * the methods to be supported by the new or modified resource. The server 
				 * is not required to support these methods and SHOULD include an Allow 
				 * header in the response giving the actual supported methods.
				 *
				 * A proxy MUST NOT modify the Allow header field even if it does not 
				 * understand all the methods specified, since the user agent might have 
				 * other means of communicating with the origin server.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.8
				//////////////////////////////////////////////////////////////////////////
				/*
				 * A user agent that wishes to authenticate itself with a server--usually,
				 * but not necessarily, after receiving a 401 response--does so by 
				 * including an Authorization request-header field with the request. The 
				 * Authorization field value consists of credentials containing the 
				 * authentication information of the user agent for the realm of
				 * the resource being requested.
				 */
				boost::spirit::rule< Scanner > Authorization_;
				/*
				 * HTTP access authentication is described in “HTTP Authentication: Basic 
				 * and Digest Access Authentication” [43]. If a request is authenticated 
				 * and a realm specified, the same credentials SHOULD be valid for all other 
				 * requests within this realm (assuming that the authentication scheme 
				 * itself does not require otherwise, such as credentials that vary 
				 * according to a challenge value or using synchronized clocks).
				 *
				 * When a shared cache (see section 13.7) receives a request containing 
				 * an Authorization field, it MUST NOT return the corresponding response 
				 * as a reply to any other request, unless one of the following specific 
				 * exceptions holds:
				 * 1. If the response includes the “s-maxage” cache-control directive, 
				 *    the cache MAY use that response in replying to a subsequent request. 
				 *    But (if the specified maximum age has passed) a proxy cache MUST 
				 *    first revalidate it with the origin server, using the request-headers 
				 *    from the new request to allow the origin server to authenticate the 
				 *    new request. (This is the defined behavior for s-maxage.) If the 
				 *    response includes “smaxage=0”, the proxy MUST always revalidate it 
				 *    before re-using it.
				 * 2. If the response includes the “must-revalidate” cache-control directive, 
				 *    the cache MAY use that response in replying to a subsequent request. 
				 *    But if the response is stale, all caches MUST first revalidate it
				 *    with the origin server, using the request-headers from the new 
				 *    request to allow the origin server to authenticate the new request.
				 * 3. If the response includes the “public” cache-control directive, it 
				 *    MAY be returned in reply to any subsequent request.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.9
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Cache-Control general-header field is used to specify directives 
				 * that MUST be obeyed by all caching mechanisms along the request/response 
				 * chain. The directives specify behavior intended to prevent caches from
				 * adversely interfering with the request or response. These directives 
				 * typically override the default caching algorithms. Cache directives 
				 * are unidirectional in that the presence of a directive in a request 
				 * does not imply that the same directive is to be given in the response.
				 * 
				 * Note that HTTP/1.0 caches might not implement Cache-Control and might 
				 *      only implement Pragma: no-cache (see section 14.32).
				 *
				 * Cache directives MUST be passed through by a proxy or gateway application, 
				 * regardless of their significance to that application, since the 
				 * directives might be applicable to all recipients along the request/response 
				 * chain. It is not possible to specify a cache-directive for a specific 
				 * cache.
				 */
				boost::spirit::rule< Scanner > Cache_Control_;
				boost::spirit::rule< Scanner > cache_directive_;
				boost::spirit::rule< Scanner > cache_request_directive_;
				boost::spirit::rule< Scanner > cache_response_directive_;
				boost::spirit::rule< Scanner > cache_extension_;
				/* 
				 * When a directive appears without any 1#field-name parameter, the 
				 * directive applies to the entire request or response. When such 
				 * a directive appears with a 1#field-name parameter, it applies only 
				 * to the named field or fields, and not to the rest of the request or 
				 * response. This mechanism supports extensibility; implementations of
				 * future versions of the HTTP protocol might apply these directives 
				 * to header fields not defined in HTTP/1.1.
				 *
				 * The cache-control directives can be broken down into these general 
				 * categories:
				 * · Restrictions on what are cacheable; these may only be imposed by 
				 *   the origin server.
				 * · Restrictions on what may be stored by a cache; these may be 
				 *   imposed by either the origin server or the user agent.
				 * · Modifications of the basic expiration mechanism; these may be 
				 *   imposed by either the origin server or the user agent.
				 * · Controls over cache revalidation and reload; these may only be 
				 *   imposed by a user agent.
				 * · Control over transformation of entities.
				 * · Extensions to the caching system.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.10
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Connection general-header field allows the sender to specify options 
				 * that are desired for that particular connection and MUST NOT be 
				 * communicated by proxies over further connections.
				 */
				boost::spirit::rule< Scanner > Connection_;
				boost::spirit::rule< Scanner > connection_token_;
				/* 
				 * HTTP/1.1 proxies MUST parse the Connection header field before a message
				 * is forwarded and, for each connection-token in this field, remove any 
				 * header field(s) from the message with the same name as the connection 
				 * token. Connection options are signaled by the presence of a 
				 * connection-token in the Connection header field, not by any corresponding 
				 * additional header field(s), since the additional header field may not be 
				 * sent if there are no parameters associated with that connection option.
				 *
				 * Message headers listed in the Connection header MUST NOT include 
				 * end-to-end headers, such as Cache-Control.
				 *
				 * HTTP/1.1 defines the “close” connection option for the sender to 
				 * signal that the connection will be closed after completion of the 
				 * response. For example,
				 *     Connection: close
				 * in either the request or the response header fields indicates that the 
				 * connection SHOULD NOT be considered ‘persistent’ (section 8.1) after 
				 * the current request/response is complete.
				 *
				 * HTTP/1.1 applications that do not support persistent connections 
				 * MUST include the “close” connection option in every message.
				 *
				 * A system receiving an HTTP/1.0 (or lower-version) message that 
				 * includes a Connection header MUST, for each connection-token in this 
				 * field, remove and ignore any header field(s) from the message with the 
				 * same name as the connection-token. This protects against mistaken 
				 * forwarding of such header fields by pre-HTTP/1.1 proxies. See section 
				 * 19.6.2.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.11
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Content-Encoding entity-header field is used as a modifier to the 
				 * media-type. When present, its value indicates what additional content 
				 * codings have been applied to the entity-body, and thus what decoding 
				 * mechanisms must be applied in order to obtain the media-type referenced 
				 * by the Content-Type header field. Content-Encoding is primarily used to 
				 * allow a document to be compressed without losing the identity of its
				 * underlying media type.
				 */
				boost::spirit::rule< Scanner > Content_Encoding_;
				/*
				 * Content codings are defined in section 3.5. An example of its use is
				 *     Content-Encoding: gzip
				 * The content-coding is a characteristic of the entity identified by 
				 * the Request-URI. Typically, the entity-body is stored with this 
				 * encoding and is only decoded before rendering or analogous usage. 
				 * However, a non-transparent proxy MAY modify the content-coding if 
				 * the new coding is known to be acceptable to the recipient, unless 
				 * the “notransform” cache-control directive is present in the message.
				 *
				 * If the content-coding of an entity is not “identity”, then the 
				 * response MUST include a Content-Encoding entity-header (section 
				 * 14.11) that lists the non-identity content-coding(s) used.
				 *
				 * If the content-coding of an entity in a request message is not 
				 * acceptable to the origin server, the server SHOULD respond with 
				 * a status code of 415 (Unsupported Media Type).
				 *
				 * If multiple encodings have been applied to an entity, the content 
				 * codings MUST be listed in the order in which they were applied. 
				 * Additional information about the encoding parameters MAY be 
				 * provided by other entity-header fields not defined by this 
				 * specification.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.12
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Content-Language entity-header field describes the natural 
				 * language(s) of the intended audience for the enclosed entity. Note 
				 * that this might not be equivalent to all the languages used within
				 * the entity-body.
				 */
				boost::spirit::rule< Scanner > Content_Language_;
				/*
				 * Language tags are defined in section 3.10. The primary purpose of 
				 * Content-Language is to allow a user to identify and differentiate 
				 * entities according to the user’s own preferred language. Thus, if 
				 * the body content is intended only for a Danish-literate audience, 
				 * the appropriate field is Content-Language: da
				 *
				 * If no Content-Language is specified, the default is that the content 
				 * is intended for all language audiences. This might mean that the 
				 * sender does not consider it to be specific to any natural language, 
				 * or that the sender does not know for which language it is intended.
				 *
				 * Multiple languages MAY be listed for content that is intended for 
				 * multiple audiences. For example, a rendition of the “Treaty of 
				 * Waitangi,” presented simultaneously in the original Maori and 
				 * English versions, would call for
				 *     Content-Language: mi, en
				 * However, just because multiple languages are present within an 
				 * entity does not mean that it is intended for multiple linguistic 
				 * audiences. An example would be a beginner’s language primer, such 
				 * as “A First Lesson in Latin,” which is clearly intended to be used 
				 * by an English-literate audience. In this case, the Content-Language
				 * would properly only include “en”.
				 *
				 * Content-Language MAY be applied to any media type -- it is not 
				 * limited to textual documents.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.13
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Content-Length entity-header field indicates the size of the 
				 * entity-body, in decimal number of OCTETs, sent to the recipient or, 
				 * in the case of the HEAD method, the size of the entity-body that would 
				 * have been sent had the request been a GET.
				 */
				boost::spirit::rule< Scanner > Content_Length_;
				/*
				 * An example is
				 *     Content-Length: 3495
				 * Applications SHOULD use this field to indicate the transfer-length 
				 * of the message-body, unless this is prohibited by the rules in 
				 * section 4.4.
				 *
				 * Any Content-Length greater than or equal to zero is a valid value. 
				 * Section 4.4 describes how to determine the length of a message-body
				 * if a Content-Length is not given.
				 * 
				 * Note that the meaning of this field is significantly different from 
				 *      the corresponding definition in MIME, where it is an optional 
				 *      field used within the “message/external-body” content-type. In 
				 *      HTTP, it SHOULD be sent whenever the message’s length can be 
				 *      determined prior to being transferred, unless this is prohibited 
				 *      by the rules in section 4.4.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.14
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Content-Location entity-header field MAY be used to supply the 
				 * resource location for the entity enclosed in the message when that 
				 * entity is accessible from a location separate from the requested 
				 * resource’s URI. A server SHOULD provide a Content-Location for the 
				 * variant corresponding to the response entity; especially in the case 
				 * where a resource has multiple entities associated with it, and those 
				 * entities actually have separate locations by which they might be 
				 * individually accessed, the server SHOULD provide a Content-Location 
				 * for the particular variant which is returned.
				 */
				boost::spirit::rule< Scanner > Content_Location_;
				/*
				 * The value of Content-Location also defines the base URI for the
				 * entity.
				 *
				 * The Content-Location value is not a replacement for the original 
				 * requested URI; it is only a statement of the location of the resource 
				 * corresponding to this particular entity at the time of the request. 
				 * Future requests MAY specify the Content-Location URI as the 
				 * request-URI if the desire is to identify the source of that particular
				 * entity.
				 *
				 * A cache cannot assume that an entity with a Content-Location different 
				 * from the URI used to retrieve it can be used to respond to later 
				 * requests on that Content-Location URI. However, the Content-Location 
				 * can be used to differentiate between multiple entities retrieved 
				 * from a single requested resource, as described in section 13.6.
				 *
				 * If the Content-Location is a relative URI, the relative URI is 
				 * interpreted relative to the Request-URI.
				 *
				 * The meaning of the Content-Location header in PUT or POST requests 
				 * is undefined; servers are free to ignore it in those cases.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.15
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Content-MD5 entity-header field, as defined in RFC 1864 [23], is 
				 * an MD5 digest of the entity-body for the purpose of providing an 
				 * end-to-end message integrity check (MIC) of the entity-body. (Note: 
				 * a MIC is good for detecting accidental modification of the 
				 * entity-body in transit, but is not proof against malicious attacks.)
				 */
				boost::spirit::rule< Scanner > Content_MD5_;
				boost::spirit::rule< Scanner > md5_digest_;
				/*
				 * The Content-MD5 header field MAY be generated by an origin server or 
				 * client to function as an integrity check of the entity-body. Only 
				 * origin servers or clients MAY generate the Content-MD5 header field;
				 * proxies and gateways MUST NOT generate it, as this would defeat its
				 * value as an end-to-end integrity check. Any recipient of the
				 * entitybody, including gateways and proxies, MAY check that the digest
				 * value in this header field matches that of the entity-body as received.
				 *
				 * The MD5 digest is computed based on the content of the entity-body,
				 * including any content-coding that has been applied, but not including
				 * any transfer-encoding applied to the message-body. If the message is
				 * received with a transfer-encoding, that encoding MUST be removed prior
				 * to checking the Content-MD5 value against the received entity.
				 *
				 * This has the result that the digest is computed on the octets of the
				 * entity-body exactly as, and in the order that, they would be sent if
				 * no transfer-encoding were being applied.
				 *
				 * HTTP extends RFC 1864 to permit the digest to be computed for MIME 
				 * composite media-types (e.g., multipart/* and message/rfc822), but
				 * this does not change how the digest is computed as defined in the
				 * preceding paragraph.
				 *
				 * There are several consequences of this. The entity-body for composite
				 * types MAY contain many body-parts, each with its own MIME and HTTP
				 * headers (including Content-MD5, Content-Transfer-Encoding, and 
				 * Content-Encoding headers). If a body-part has a Content-Transfer-Encoding
				 * or Content-Encoding header, it is assumed that the content of the 
				 * body-part has had the encoding applied, and the body-part is included
				 * in the Content-MD5 digest as is -- i.e., after the application. The
				 * Transfer-Encoding header field is not allowed within body-parts.
				 *
				 * Conversion of all line breaks to CRLF MUST NOT be done before computing 
				 * or checking the digest: the line break convention used in the text actually
				 * transmitted MUST be left unaltered when computing the digest.
				 *
				 * Note: while the definition of Content-MD5 is exactly the same for HTTP as
				 *       in RFC 1864 for MIME entity-bodies, there are several ways in which 
				 *       the application of Content-MD5 to HTTP entity-bodies differs from its
				 *       application to MIME entity-bodies. One is that HTTP, unlike MIME, does
				 *       not use Content-Transfer-Encoding, and does use Transfer-Encoding and
				 *       Content-Encoding. Another is that HTTP more frequently uses binary
				 *       content types than MIME, so it is worth noting that, in such cases,
				 *       the byte order used to compute the digest is the transmission byte
				 *       order defined for the type. Lastly, HTTP allows transmission of text
				 *       types with any of several line break conventions and not just the 
				 *       canonical form using CRLF.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.16
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Content-Range entity-header is sent with a partial entity-body to 
				 * specify where in the full entity-body the partial body should be 
				 * applied. Range units are defined in section 3.12.
				 */
				boost::spirit::rule< Scanner > Content_Range_;
				boost::spirit::rule< Scanner > content_range_spec_;
				boost::spirit::rule< Scanner > byte_content_range_spec_;
				boost::spirit::rule< Scanner > byte_range_resp_spec_;
				boost::spirit::rule< Scanner > instance_length_;
				/*
				 * The header SHOULD indicate the total length of the full entity-body, 
				 * unless this length is unknown or difficult to determine. The asterisk 
				 * “*” character means that the instance-length is unknown at the time 
				 * when the response was generated.
				 *
				 * Unlike byte-ranges-specifier values (see section 14.35.1), a 
				 * byte-range-resp-spec MUST only specify one range, and MUST contain 
				 * absolute byte positions for both the first and last byte of the range.
				 *
				 * A byte-content-range-spec with a byte-range-resp-spec whose 
				 * last-byte-pos value is less than its first-byte-pos value, or whose 
				 * instance-length value is less than or equal to its lastbytepos value,
				 * is invalid. The recipient of an invalid byte-content-range-spec MUST 
				 * ignore it and any content transferred along with it.
				 *
				 * A server sending a response with status code 416 (Requested range 
				 * not satisfiable) SHOULD include a Content-Range field with a 
				 * byte-range-resp-spec of “*”. The instance-length specifies the current 
				 * length of the selected resource. A response with status code 206 
				 * (Partial Content) MUST NOT include a Content-Range field with a 
				 * byte-range-resp-spec of “*”.
				 *
				 * Examples of byte-content-range-spec values, assuming that the entity 
				 * contains a total of 1234 bytes:
				 * · The first 500 bytes:
				 *   bytes 0-499/1234
				 * · The second 500 bytes:
				 *   bytes 500-999/1234
				 * · All except for the first 500 bytes:
				 *   bytes 500-1233/1234
				 * · The last 500 bytes:
				 *   bytes 734-1233/1234
				 * When an HTTP message includes the content of a single range (for 
				 * example, a response to a request for a single range, or to a 
				 * request for a set of ranges that overlap without any holes), this 
				 * content is transmitted with a Content-Range header, and a 
				 * Content-Length header showing the number of bytes actually 
				 * transferred. For example,
				 *     HTTP/1.1 206 Partial content
				 *     Date: Wed, 15 Nov 1995 06:25:24 GMT
				 *     Last-Modified: Wed, 15 Nov 1995 04:58:08 GMT
				 *     Content-Range: bytes 21010-47021/47022
				 *     Content-Length: 26012
				 *     Content-Type: image/gif
				 * When an HTTP message includes the content of multiple ranges (for 
				 * example, a response to a request for multiple non-overlapping 
				 * ranges), these are transmitted as a multipart message. The 
				 * multipart media type used for this purpose is “multipart/byteranges”
				 * as defined in appendix 19.2. See appendix 19.6.3 for a 
				 * compatibility issue.
				 *
				 * A response to a request for a single range MUST NOT be sent using 
				 * the multipart/byteranges media type. A response to a request for 
				 * multiple ranges, whose result is a single range, MAY be sent as a 
				 * multipart/byteranges media type with one part. A client that cannot 
				 * decode a multipart/byteranges message MUST NOT ask for multiple 
				 * byte-ranges in a single request.
				 *
				 * When a client requests multiple byte-ranges in one request, the 
				 * server SHOULD return them in the order that they appeared in the 
				 * request. If the server ignores a byte-range-spec because it is 
				 * syntactically invalid, the server SHOULD treat the request as 
				 * if the invalid Range header field did not exist. (Normally, this 
				 * means return a 200 response containing the full entity).
				 *
				 * If the server receives a request (other than one including an 
				 * If-Range request-header field) with an unsatisfiable Range 
				 * request-header field (that is, all of whose byte-range-spec values 
				 * have a first-byte-pos value greater than the current length of the 
				 * selected resource), it SHOULD return a response code of 416 
				 * (Requested range not satisfiable) (section 10.4.17).
				 *
				 * Note: clients cannot depend on servers to send a 416 (Requested 
				 *       range not satisfiable) response instead of a 200 (OK) 
				 *       response for an unsatisfiable Range request-header, since 
				 *       not all servers implement this requestheader.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.17
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Content-Type entity-header field indicates the media type of the 
				 * entity-body sent to the recipient or, in the case of the HEAD method,
				 * the media type that would have been sent had the request been a GET.
				 */
				boost::spirit::rule< Scanner > Content_Type_;
				/*
				 * Media types are defined in section 3.7. An example of the field is
				 *     Content-Type: text/html; charset=ISO-8859-4
				 * Further discussion of methods for identifying the media type of an 
				 * entity is provided in section 7.2.1.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.18
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Date general-header field represents the date and time at which the 
				 * message was originated, having the same semantics as orig-date in RFC 
				 * 822. The field value is an HTTP-date, as described in section 3.3.1;
				 * it MUST be sent in RFC 1123 [8]-date format.
				 */
				boost::spirit::rule< Scanner > Date_;
				/*
				 * An example is
				 *     Date: Tue, 15 Nov 1994 08:12:31 GMT
				 * Origin servers MUST include a Date header field in all responses, 
				 * except in these cases:
				 * 1. If the response status code is 100 (Continue) or 101 (Switching 
				 *    Protocols), the response MAY include a Date header field, at the 
				 *    server’s option.
				 * 2. If the response status code conveys a server error, e.g. 500 
				 *    (Internal Server Error) or 503 (Service Unavailable), and it is 
				 *    inconvenient or impossible to generate a valid Date.
				 * 3. If the server does not have a clock that can provide a reasonable 
				 *    approximation of the current time, its responses MUST NOT include 
				 *    a Date header field. In this case, the rules in section 14.18.1 
				 *    MUST be followed.
				 * A received message that does not have a Date header field MUST be 
				 * assigned one by the recipient if the message will be cached by that 
				 * recipient or gatewayed via a protocol which requires a Date. An HTTP 
				 * implementation without a clock MUST NOT cache responses without 
				 * revalidating them on every use. An HTTP cache, especially a shared 
				 * cache, SHOULD use a mechanism, such as NTP [28], to synchronize its 
				 * clock with a reliable external standard.
				 *
				 * Clients SHOULD only send a Date header field in messages that include 
				 * an entity-body, as in the case of the PUT and POST requests, and even 
				 * then it is optional. A client without a clock MUST NOT send a Date 
				 * header field in a request.
				 *
				 * The HTTP-date sent in a Date header SHOULD NOT represent a date and 
				 * time subsequent to the generation of the message. It SHOULD represent 
				 * the best available approximation of the date and time of message 
				 * generation, unless the implementation has no means of generating a 
				 * reasonably accurate date and time. In theory, the date ought to 
				 * represent the moment just before the entity is generated. In practice, 
				 * the date can be generated at any time during the message origination 
				 * without affecting its semantic value.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.19
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The ETag response-header field provides the current value of the entity 
				 * tag for the requested variant. The headers used with entity tags are 
				 * described in sections 14.24, 14.26 and 14.44. The entity tag MAY be
				 * used for comparison with other entities from the same resource (see 
				 * section 13.3.3).
				 */
				boost::spirit::rule< Scanner > ETag_;
				//////////////////////////////////////////////////////////////////////////
				// Section 14.20
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Expect request-header field is used to indicate that particular 
				 * server behaviors are required by the client.
				 */
				boost::spirit::rule< Scanner > Expect_;
				boost::spirit::rule< Scanner > expectation_;
				boost::spirit::rule< Scanner > expectation_extension_;
				boost::spirit::rule< Scanner > expect_params_;
				/*
				 * A server that does not understand or is unable to comply with any of 
				 * the expectation values in the Expect field of a request MUST respond 
				 * with appropriate error status. The server MUST respond with a 417 
				 * (Expectation Failed) status if any of the expectations cannot be met 
				 * or, if there are other problems with the request, some other 4xx status.
				 * 
				 * This header field is defined with extensible syntax to allow for future 
				 * extensions. If a server receives a request containing an Expect field 
				 * that includes an expectation-extension that it does not support, it 
				 * MUST respond with a 417 (Expectation Failed) status.
				 *
				 * Comparison of expectation values is case-insensitive for unquoted 
				 * tokens (including the 100-continue token), and is case-sensitive for 
				 * quoted-string expectation-extensions.
				 * 
				 * The Expect mechanism is hop-by-hop: that is, an HTTP/1.1 proxy MUST 
				 * return a 417 (Expectation Failed) status if it receives a request with 
				 * an expectation that it cannot meet. However, the Expect request-header 
				 * itself is end-to-end; it MUST be forwarded if the request is forwarded.
				 *
				 * Many older HTTP/1.0 and HTTP/1.1 applications do not understand the 
				 * Expect header.
				 *
				 * See section 8.2.3 for the use of the 100 (continue) status.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.21
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Expires entity-header field gives the date/time after which the 
				 * response is considered stale. A stale cache entry may not normally be 
				 * returned by a cache (either a proxy cache or a user agent cache) unless 
				 * it is first validated with the origin server (or with an intermediate 
				 * cache that has a fresh copy of the entity). See section 13.2 for further
				 * discussion of the expiration model.
				 *
				 * The presence of an Expires field does not imply that the original 
				 * resource will change or cease to exist at, before, or after that time.
				 *
				 * The format is an absolute date and time as defined by HTTP-date in 
				 * section 3.3.1; it MUST be in RFC 1123 date format:
				 */
				boost::spirit::rule< Scanner > Expires_;
				/*
				 * An example of its use is
				 *     Expires: Thu, 01 Dec 1994 16:00:00 GMT
				 * Note: if a response includes a Cache-Control field with the max-age 
				 *       directive (see section 14.9.3), that directive overrides the 
				 *       Expires field.
				 *
				 * HTTP/1.1 clients and caches MUST treat other invalid date formats, 
				 * especially including the value “0”, as in the past (i.e., “already 
				 * expired”).
				 *
				 * To mark a response as “already expired,” an origin server sends an 
				 * Expires date that is equal to the Date header value. (See the rules 
				 * for expiration calculations in section 13.2.4.) To mark a response 
				 * as “never expires,” an origin server sends an Expires date approximately 
				 * one year from the time the response is sent. HTTP/1.1 servers SHOULD 
				 * NOT send Expires dates more than one year in the future.
				 *
				 * The presence of an Expires header field with a date value of some time 
				 * in the future on a response that otherwise would by default be 
				 * non-cacheable indicates that the response is cacheable, unless 
				 * indicated otherwise by a Cache-Control header field (section 14.9).
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.22
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The From request-header field, if given, SHOULD contain an Internet 
				 * e-mail address for the human user who controls the requesting user 
				 * agent. The address SHOULD be machine-usable, as defined by “mailbox” 
				 * in RFC 822 [9] as updated by RFC 1123 [8]:
				 */
				boost::spirit::rule< Scanner > From_;
				/*
				 * An example is:
				 *     From: webmaster@w3.org
				 * This header field MAY be used for logging purposes and as a means for 
				 * identifying the source of invalid or unwanted requests. It SHOULD NOT 
				 * be used as an insecure form of access protection. The interpretation 
				 * of this field is that the request is being performed on behalf of the 
				 * person given, who accepts responsibility for the method performed. In 
				 * particular, robot agents SHOULD include this header so that the person 
				 * responsible for running the robot can be contacted if problems occur 
				 * on the receiving end.
				 *
				 * The Internet e-mail address in this field MAY be separate from the 
				 * Internet host which issued the request. For example, when a request 
				 * is passed through a proxy the original issuer’s address SHOULD be used.
				 *
				 * The client SHOULD NOT send the From header field without the user’s 
				 * approval, as it might conflict with the user’s privacy interests or 
				 * their site’s security policy. It is strongly recommended that the 
				 * user be able to disable, enable, and modify the value of this field 
				 * at any time prior to a request.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.23
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Host request-header field specifies the Internet host and port number 
				 * of the resource being requested, as obtained from the original URI 
				 * given by the user or referring resource (generally an HTTP URL, as 
				 * described in section 3.2.2). The Host field value MUST represent the 
				 * naming authority of the origin server or gateway given by the original 
				 * URL. This allows the origin server or gateway to differentiate between 
				 * internally-ambiguous URLs, such as the root “/” URL of a server for 
				 * multiple host names on a single IP address.
				 */
				boost::spirit::rule< Scanner > Host_;
				/*
				 * A “host” without any trailing port information implies the default port 
				 * for the service requested (e.g., “80” for an HTTP URL). For example, a
				 * request on the origin server for <http://www.w3.org/pub/WWW/> would
				 * properly include:
				 *     GET /pub/WWW/ HTTP/1.1
				 *     Host: www.w3.org
				 *
				 * A client MUST include a Host header field in all HTTP/1.1 request 
				 * messages . If the requested URI does not include an Internet host 
				 * name for the service being requested, then the Host header field MUST 
				 * be given with an empty value. An HTTP/1.1 proxy MUST ensure that any 
				 * request message it forwards does contain an appropriate Host header 
				 * field that identifies the service being requested by the proxy. All 
				 * Internet-based HTTP/1.1 servers MUST respond with a 400 (Bad Request) 
				 * status code to any HTTP/1.1 request message which lacks a Host header
				 * field.
				 *
				 * See sections 5.2 and 19.6.1.1 for other requirements relating to Host.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.24
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The If-Match request-header field is used with a method to make it 
				 * conditional. A client that has one or more entities previously obtained
				 * from the resource can verify that one of those entities is current by
				 * including a list of their associated entity tags in the If-Match header
				 * field. Entity tags are defined in section 3.11. The purpose of this 
				 * feature is to allow efficient updates of cached information with a 
				 * minimum amount of transaction overhead. It is also used, on updating 
				 * requests, to prevent inadvertent modification of the wrong version of 
				 * a resource. As a special case, the value “*” matches any current entity
				 * of the resource.
				 */
				boost::spirit::rule< Scanner > If_Match_;
				/*
				 * If any of the entity tags match the entity tag of the entity that would 
				 * have been returned in the response to a similar GET request (without the
				 * If-Match header) on that resource, or if “*” is given and any current
				 * entity exists for that resource, then the server MAY perform the requested
				 * method as if the If-Match header field did not exist.
				 *
				 * A server MUST use the strong comparison function (see section 13.3.3) to
				 * compare the entity tags in If-Match.
				 *
				 * If none of the entity tags match, or if “*” is given and no current entity
				 * exists, the server MUST NOT perform the requested method, and MUST return
				 * a 412 (Precondition Failed) response. This behavior is most useful when the
				 * client wants to prevent an updating method, such as PUT, from modifying a
				 * resource that has changed since the client last retrieved it.
				 *
				 * If the request would, without the If-Match header field, result in anything
				 * other than a 2xx or 412 status, then the If-Match header MUST be ignored.
				 *
				 * The meaning of “If-Match: *” is that the method SHOULD be performed if the
				 * representation selected by the origin server (or by a cache, possibly using
				 * the Vary mechanism, see section 14.44) exists, and MUST NOT be performed if
				 * the representation does not exist.
				 *
				 * A request intended to update a resource (e.g., a PUT) MAY include an If-Match
				 * header field to signal that the request method MUST NOT be applied if the 
				 * entity corresponding to the If-Match value (a single entity tag) is no
				 * longer a representation of that resource. This allows the user to indicate 
				 * that they do not wish the request to be successful if the resource has been
				 * changed without their knowledge. Examples:
				 *     If-Match: "xyzzy"
				 *     If-Match: "xyzzy", "r2d2xxxx", "c3piozzzz"
				 *     If-Match: *
				 * The result of a request having both an If-Match header field and either an 
				 * If-None-Match or an If-Modified-Since header fields is undefined by this 
				 * specification.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.25
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The If-Modified-Since request-header field is used with a method to make
				 * it conditional: if the requested variant has not been modified since the
				 * time specified in this field, an entity will not be returned from the
				 * server; instead, a 304 (not modified) response will be returned without
				 * any message-body.
				 */
				boost::spirit::rule< Scanner > If_Modified_Since_;
				/* An example of the field is:
				 *     If-Modified-Since: Sat, 29 Oct 1994 19:43:31 GMT
				 * A GET method with an If-Modified-Since header and no Range header requests 
				 * that the identified entity be transferred only if it has been modified 
				 * since the date given by the If-Modified-Since header. The algorithm
				 * for determining this includes the following cases:
				 * a) If the request would normally result in anything other than a 200 
				 *    (OK) status, or if the passed If-Modified-Since date is invalid, 
				 *    the response is exactly the same as for a normal GET. A date which is 
				 *    later than the server’s current time is invalid.
				 * b) If the variant has been modified since the If-Modified-Since date, 
				 *    the response is exactly the same as for a normal GET.
				 * c) If the variant has not been modified since a valid If-Modified-Since 
				 *    date, the server SHOULD return a 304 (Not Modified) response.
				 * The purpose of this feature is to allow efficient updates of cached 
				 * information with a minimum amount of transaction overhead.
				 *
				 * Note: The Range request-header field modifies the meaning of 
				 *       If-Modified-Since; see section 14.35 for full details.
				 *
				 * Note: If-Modified-Since times are interpreted by the server, whose 
				 *       clock might not be synchronized with the client.
				 *
				 * Note: When handling an If-Modified-Since header field, some servers 
				 *       will use an exact date comparison function, rather than a 
				 *       less-than function, for deciding whether to send a 304 
				 *       (Not Modified) response. To get best results when sending 
				 *       an If-Modified-Since header field for cache validation,
				 *       clients are advised to use the exact date string received 
				 *       in a previous Last-Modified header field whenever possible.
				 *
				 * Note: If a client uses an arbitrary date in the If-Modified-Since 
				 *       header instead of a date taken from the Last-Modified header 
				 *       for the same request, the client should be aware of the fact 
				 *       that this date is interpreted in the server’s understanding 
				 *       of time. The client should consider unsynchronized clocks and
				 *       rounding problems due to the different encodings of time 
				 *       between the client and server. This includes the possibility 
				 *       of race conditions if the document has changed between the 
				 *       time it was first requested and the If-Modified-Since date of 
				 *       a subsequent request, and the possibility of clock-skew-related 
				 *       problems if the If-Modified-Since date is derived from the 
				 *       client’s clock without correction to the server’s clock.
				 *
				 *       Corrections for different time bases between client and server 
				 *       are at best approximate due to network latency.
				 *
				 * The result of a request having both an If-Modified-Since header field 
				 * and either an If-Match or an If-Unmodified-Since header fields is 
				 * undefined by this specification.
				 */
				boost::spirit::rule< Scanner > target_;
			};
		};
	}
}

#endif


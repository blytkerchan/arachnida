#ifndef _spin_private_grammar_h
#define _spin_private_grammar_h

#include <boost/spirit.hpp>

namespace Spin
{
	namespace Private
	{
		template < typename Derived >
		struct Grammar : boost::spirit::grammar< Derived >
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
					http_URL_		= str_p("http:") >> "//" >> host_ >> !(':' >> port_) >> !(abs_path_ >> !('?' >> query_))
									;
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
					trailer_		= *(entity_header_ >> CRLF_)
									;
					media_type_		= type_ >> '/' >> subtype_ >> *(';' >> parameter_)
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
									| (ch_p('1') >> !repeat_p(0,3)[ch_p('0')])
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
					message_body_	= +OCTET_//entity_body_
									| +OCTET_ // <entity-body encoded as per Transfer-Encoding>
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
									| authority_
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
									;
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
					Accept_			= str_p("Accept") >> ':' >> !((*LWS_ >> media_range_ >> *LWS_ >> !accept_params_ >> *LWS_) % ',')
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
					Accept_Charset_	= str_p("Accept-Charset") >> ':' >> ((*LWS_ >> (charset_ | '*') >> *LWS_ >> !(*LWS_ >> ch_p(';') >> *LWS_ >> 'q' >> *LWS_ >> '=' >> *LWS_ >> qvalue_ >> *LWS_)) % ',')
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
					Allow_			= str_p("Allow") >> ':' >> ((*LWS_ >> Method_ >> *LWS_) % ',')
									;
					Authorization_	= str_p("Authorization") >> ':' >> token_//credentials_
									;
					Cache_Control_	= str_p("Cache-Control") >> ':' >> ((*LWS_ >> cache_directive_ >> *LWS_) % ',')
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
									| (str_p("private") >> !( '=' >> ch_p('"') >> ((*LWS_ >> field_name_ >> *LWS_) % ',') >> ch_p('"') ))	// Section 14.9.1
									| ("no-cache" >> !( ch_p('=') >> '"' >>  (field_name_ % ',') >> '"' ))	// Section 14.9.1
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
									= str_p("Content-Location") >> ':' >> (absoluteURI_ | relativeURI_)
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
									= token_ >> !('=' >> (token_ | quoted_string_) >> *expect_params_)
									;
					expect_params_	= ';' >> token_ >> !('=' >> (token_ | quoted_string_))
									;
					Expires_		= str_p("Expires") >> ':' >> HTTP_date_
									;
					//From_			= str_p("From") >> ':' >> mailbox_
					//				;
					Host_			= str_p("Host") >> ':' >> host_ >> !(':' >> port_)
									;
					If_Match_		= str_p("If-Match") >> ':' >> ('*' | ((*LWS_ >> entity_tag_ >> *LWS_) % ','))
									;
					If_Modified_Since_
									= str_p("If-Modified-Since") >> ':' >> HTTP_date_
									;
					If_None_Match_	= str_p("If-none-Match") >> ':' >> ('*' | ((*LWS_ >> entity_tag_ >> *LWS_) % ','))
									;
					If_Unmodified_Since_
									= str_p("If-Unmodified-Since") >> ':' >> HTTP_date_
									;
					Last_Modified_	= str_p("Last-Modified") >> ':' >> HTTP_date_
									;
					Location_		= str_p("Last-Modified") >> ':' >> absoluteURI_
									;
					Max_Forwards_	= str_p("Max-Forwards") >> ':' >> +DIGIT_
									;
					Pragma_			= str_p("Pragma") >> ':' >> ((*LWS_ >> pragma_directive_ >> *LWS_) % ',')
									;
					pragma_directive_
									= "no-cache"
									| extension_pragma_
									;
					extension_pragma_
									= token_ >> !('=' >> (token_ | quoted_string_))
									;
					//Proxy_Authenticate_
					//				= str_p("Proxy-Authenticate") >> ':' >> ((*LWS_ >> challenge_ >> *LWS_) % ',')
					//				;
					//Proxy_Authorization_
					//				= str_p("Proxy-Authorization") >> ':' >> credentials_
					//				;
					ranges_specifier_
									= byte_ranges_specifier_
									;
					byte_ranges_specifier_
									= bytes_unit_ >> '=' >> byte_range_set_
									;
					byte_range_set_	= ((*LWS_ >> (byte_range_spec_ | suffix_byte_range_spec_) >> *LWS_) % ',')
									;
					byte_range_spec_= first_byte_pos_ >> '-' >> !last_byte_pos_
									;
					first_byte_pos_	= +DIGIT_
									;
					last_byte_pos_	= +DIGIT_
									;
					suffix_byte_range_spec_
									= '-' >> suffix_length_
									;
					suffix_length_	= +DIGIT_
									;
					Range_			= str_p("Range") >> ':' >> ranges_specifier_
									;
					Referer_		= str_p("Referer") >> ':' >> (absoluteURI_ | relativeURI_)
									;
					Retry_After_	= str_p("Retry-After") >> ':' >> (HTTP_date_ | delta_seconds_)
									;
					Trailer_		= str_p("Trailer") >> ':' >> ((*LWS_ >> field_name_ >> *LWS_) % ',')
									;
					Transfer_Encoding_
									= str_p("Transfer-Encoding") >> ':' >> ((*LWS_ >> transfer_coding_ >> *LWS_) % ',')
									;
					Upgrade_		= str_p("Upgrade") >> ':' >> ((*LWS_ >> product_ >> *LWS_) % ',')
									;
					User_Agent_		= str_p("User-Agent") >> ':' >> ((*LWS_ >> (product_ | comment_) >> *LWS_) % ',')
									;
					Vary_			= str_p("Vary") >> ':' >> ('*' | ((*LWS_ >> field_name_ >> *LWS_) % ','))
									;
					Via_			= str_p("Via") >> ':' >> ((received_protocol_ >> received_by_ >> !comment_) % ',')
									;
					received_protocol_
									= !( protocol_name_ >> '/' ) >> protocol_version_
									;
					protocol_name_	= token_
									;
					protocol_version_
									= token_
									;
					received_by_	= ( host_ >> !( ':' >> port_ ) )
									| pseudonym_
									;
					pseudonym_		= token_
									;
					Warning_		= str_p("Warning") >> ':' >> (warning_value_ % ',')
									;
					warning_value_	= warn_code_ >> SP_ >> warn_agent_ >> SP_ >> warn_text_ >> !(SP_ >> warn_date_)
									;
					warn_code_		= repeat_p(3)[DIGIT_]	
									;
					warn_agent_		= ( host_ >> !(':' >> port_ ) )
									| pseudonym_
									;
					warn_text_		= quoted_string_
									;
					warn_date_		= '"' >> HTTP_date_ >> '"'
									;
					//WWW_Authenticate_
					//				= str_p("WWW-Authenticate") >> ':' >> ((*LWS_ >> challenge_ >> *LWS_) % ',')
					//				;
					absoluteURI_	= lexeme_d[scheme_ >> ':' >> (hier_part_ | opaque_part_)]
									;
					hier_part_		= (net_path_ | abs_path_) >> !('?' >> query_)
									;
					net_path_		= "//" >> authority_ >> !abs_path_
									;
					abs_path_		= '/' >> path_segments_
									;
					opaque_part_	= uric_no_slash_ >> *uric_
									;
					uric_no_slash_	= unreserved_
									| escaped_
									| ';'
									| '?'
									| ':'
									| '@'
									| '&'
									| '='
									| '+' 
									| '$'
									| ','
									;
					uric_			= reserved_
									| unreserved_
									| escaped_
									;
					reserved_		= ch_p(';')
									| '/'
									| '?'
									| ':'
									| '@'
									| '&'
									| '='
									| '+'
									| '$'
									| ','
									;
					unreserved_		= ALPHA_ | DIGIT_ | mark_
									;
					mark_			= ch_p('-')
									| '_'
									| '.'
									| '!'
									| '~'
									| '*'
									| '\''
									| '('
									| ')'
									;
					escaped_		= '%' >> HEX_ >> HEX_
									;
					delims_			= ch_p('<')
									| '>'
									| '#'
									| '%'
									| '"'
									;
					unwise_			= ch_p('{')
									| '}'
									| '|'
									| '\\'
									| '^' 
									| '['
									| ']'
									| '`'
									;
					scheme_			= ALPHA_ >> *(ALPHA_ | DIGIT_ | '+' | '-' | '.')
									;
					authority_		= server_
									| reg_name_
									;
					reg_name_		= +( unreserved_
									   | escaped_
									   | '$'
									   | ','
									   | ';'
									   | ':'
									   | '@'
									   | '&'
									   | '='
									   | '+'
									   )
									;
					server_			= !(userinfo_ >> '@') >> hostport_
									;
					userinfo_		= *( unreserved_
									   | escaped_
									   | ';'
									   | ':'
									   | '&'
									   | '='
									   | '+'
									   | '$'
									   | ','
									   )
									;
					hostport_		= host_ >> !(':' >> port_)
									;
					host_			= hostname_ | IPV4address_
									;
					hostname_		= *(domainlabel_ >> '.') >> toplabel_ >> !ch_p('.')
									;
					domainlabel_	= (ALPHA_ | DIGIT_)
									| (ALPHA_ | DIGIT_) >> *(ALPHA_ | DIGIT_ | '-') >> (ALPHA_ | DIGIT_)
									;
					toplabel_		= ALPHA_
									| ALPHA_ >> *(ALPHA_ | DIGIT_ | '-') >> (ALPHA_ | DIGIT_)
									;
					IPV4address_	= +DIGIT_ >> repeat_p(3)['.' >> +DIGIT_]
									;
					port_			= +DIGIT_
									;
					path_			= abs_path_ | opaque_part_
									;
					path_segments_	= segment_ % '/'
									;
					segment_		= *pchar_ >> *(';' >> param_)
									;
					param_			= *pchar_
									;
					pchar_			= unreserved_
									| escaped_
									| ':'
									| '@'
									| '&'
									| '='
									| '+'
									| '$'
									| ','
									;
					query_			= *uric_
									;
					URI_reference_	= !( absoluteURI_ | relativeURI_ ) >> !( '#' >> fragment_ )
									;
					fragment_		= *uric_
									;
					relativeURI_	= (net_path_ | abs_path_ | rel_path_) >> !('?' >> query_)
									;
					rel_path_		= rel_segment_ >> !abs_path_
									;
					rel_segment_	= +( unreserved_ 
									   | escaped_
									   | ';'
									   | '@'
									   | '&'
									   | '='
									   | '+'
									   | '$'
									   | ','
									   )
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
				//////////////////////////////////////////////////////////////////////////
				// Section 14.26
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The If-None-Match request-header field is used with a method to make 
				 * it conditional. A client that has one or more entities previously 
				 * obtained from the resource can verify that none of those entities is 
				 * current by including a list of their associated entity tags in the 
				 * If-None-Match header field. The purpose of this feature is to allow 
				 * efficient updates of cached information with a minimum amount of 
				 * transaction overhead. It is also used to prevent a method (e.g. PUT)
				 * from inadvertently modifying an existing resource when the client 
				 * believes that the resource does not exist.
				 *
				 * As a special case, the value “*” matches any current entity of the 
				 * resource.
				 */
				boost::spirit::rule< Scanner > If_None_Match_;
				/*
				 * If any of the entity tags match the entity tag of the entity that would 
				 * have been returned in the response to a similar GET request (without the 
				 * If-None-Match header) on that resource, or if “*” is given and any current
				 * entity exists for that resource, then the server MUST NOT perform the 
				 * requested method, unless required to do so because the resource’s 
				 * modification date fails to match that supplied in an If-Modified-Since 
				 * header field in the request. Instead, if the request method was GET or 
				 * HEAD, the server SHOULD respond with a 304 (Not Modified) response, 
				 * including the cache-related header fields (particularly ETag) of one of 
				 * the entities that matched. For all other request methods, the server 
				 * MUST respond with a status of 412 (Precondition Failed).
				 *
				 * See section 13.3.3 for rules on how to determine if two entities tags 
				 * match. The weak comparison function can only be used with GET or HEAD 
				 * requests.
				 *
				 * If none of the entity tags match, then the server MAY perform the requested 
				 * method as if the If-None-Match header field did not exist, but MUST also 
				 * ignore any If-Modified-Since header field(s) in the request. That is,
				 * if no entity tags match, then the server MUST NOT return a 304 (Not Modified)
				 * response.
				 *
				 * If the request would, without the If-None-Match header field, result in 
				 * anything other than a 2xx or 304 status, then the If-None-Match header MUST 
				 * be ignored. (See section 13.3.4 for a discussion of server behavior when
				 * both If-Modified-Since and If-None-Match appear in the same request.)
				 *
				 * The meaning of “If-None-Match: *” is that the method MUST NOT be performed 
				 * if the representation selected by the origin server (or by a cache, possibly 
				 * using the Vary mechanism, see section 14.44) exists, and SHOULD be performed 
				 * if the representation does not exist. This feature is intended to be useful 
				 * in preventing races between PUT operations.
				 *
				 * Examples:
				 *     If-None-Match: "xyzzy"
				 *     If-None-Match: W/"xyzzy"
				 *     If-None-Match: "xyzzy", "r2d2xxxx", "c3piozzzz"
				 *     If-None-Match: W/"xyzzy", W/"r2d2xxxx", W/"c3piozzzz"
				 *     If-None-Match: *
				 * The result of a request having both an If-None-Match header field and either 
				 * an If-Match or an If-Unmodified-Since header fields is undefined by this 
				 * specification.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.27
				//////////////////////////////////////////////////////////////////////////
				/*
				 * If a client has a partial copy of an entity in its cache, and wishes to 
				 * have an up-to-date copy of the entire entity in its cache, it could use 
				 * the Range request-header with a conditional GET (using either or both 
				 * of If-Unmodified-Since and If-Match.) However, if the condition fails 
				 * because the entity has been modified, the client would then have to make 
				 * a second request to obtain the entire current entity-body.
				 * 
				 * The If-Range header allows a client to “short-circuit” the second request.
				 * Informally, its meaning is ‘if the entity is unchanged, send me the part(s)
				 * that I am missing; otherwise, send me the entire new entity.’
				 */
				boost::spirit::rule< Scanner > If_Range_;
				/*
				 * If the client has no entity tag for an entity, but does have a Last-Modified 
				 * date, it MAY use that date in an If-Range header. (The server can distinguish 
				 * between a valid HTTP-date and any form of entity-tag by examining no more than 
				 * two characters.) The If-Range header SHOULD only be used together with a Range
				 * header, and MUST be ignored if the request does not include a Range header, or
				 * if the server does not support the sub-range operation.
				 *
				 * If the entity tag given in the If-Range header matches the current entity tag
				 * for the entity, then the server SHOULD provide the specified sub-range of the
				 * entity using a 206 (Partial content) response. If the entity tag does not match,
				 * then the server SHOULD return the entire entity using a 200 (OK) response.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.28
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The If-Unmodified-Since request-header field is used with a method to 
				 * make it conditional. If the requested resource has not been modified 
				 * since the time specified in this field, the server SHOULD perform the 
				 * requested operation as if the If-Unmodified-Since header were not
				 * present.
				 *
				 * If the requested variant has been modified since the specified time, 
				 * the server MUST NOT perform the requested operation, and MUST return 
				 * a 412 (Precondition Failed).
				 */
				boost::spirit::rule< Scanner > If_Unmodified_Since_;
				/*
				 * An example of the field is:
				 *     If-Unmodified-Since: Sat, 29 Oct 1994 19:43:31 GMT
				 * If the request normally (i.e., without the If-Unmodified-Since header)
				 * would result in anything other than a 2xx or 412 status, the 
				 * If-Unmodified-Since header SHOULD be ignored.
				 *
				 * If the specified date is invalid, the header is ignored. 
				 *
				 * The result of a request having both an If-Unmodified-Since header field 
				 * and either an If-None-Match or an If-Modified-Since header fields is 
				 * undefined by this specification.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.29
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Last-Modified entity-header field indicates the date and time at 
				 * which the origin server believes the variant was last modified.
				 */
				boost::spirit::rule< Scanner > Last_Modified_;
				/*
				 * An example of its use is
				 *     Last-Modified: Tue, 15 Nov 1994 12:45:26 GMT
				 * The exact meaning of this header field depends on the implementation 
				 * of the origin server and the nature of the original resource. For 
				 * files, it may be just the file system last-modified time. For entities 
				 * with dynamically included parts, it may be the most recent of the set 
				 * of last-modify times for its component parts. For database gateways, it 
				 * may be the last-update time stamp of the record. For virtual objects, 
				 * it may be the last time the internal state changed.
				 *
				 * An origin server MUST NOT send a Last-Modified date which is later than 
				 * the server’s time of message origination. In such cases, where the 
				 * resource’s last modification would indicate some time in the future, 
				 * the server MUST replace that date with the message origination date.
				 *
				 * An origin server SHOULD obtain the Last-Modified value of the entity as 
				 * close as possible to the time that it generates the Date value of its 
				 * response. This allows a recipient to make an accurate assessment of the 
				 * entity’s modification time, especially if the entity changes near the 
				 * time that the response is generated.
				 *
				 * HTTP/1.1 servers SHOULD send Last-Modified whenever feasible
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.30
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Location response-header field is used to redirect the recipient to 
				 * a location other than the Request-URI for completion of the request or 
				 * identification of a new resource. For 201 (Created) responses, the 
				 * Location is that of the new resource which was created by the request. 
				 * For 3xx responses, the location SHOULD indicate the server’s preferred 
				 * URI for automatic redirection to the resource. The field value consists 
				 * of a single absolute URI.
				 */
				boost::spirit::rule< Scanner > Location_;
				/*
				 * An example is:
				 *     Location: http://www.w3.org/pub/WWW/People.html
				 * Note: The Content-Location header field (section 14.14) differs from 
				 *       Location in that the Content-Location identifies the original 
				 *       location of the entity enclosed in the request. It is therefore
				 *       possible for a response to contain header fields for both Location 
				 *       and Content-Location. Also see section 13.10 for cache requirements
				 *       of some methods. 
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.31
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Max-Forwards request-header field provides a mechanism with the 
				 * TRACE (section 9.8) and OPTIONS (section 9.2) methods to limit the number
				 * of proxies or gateways that can forward the request to the next inbound 
				 * server. This can be useful when the client is attempting to trace a request
				 * chain which appears to be failing or looping in mid-chain.
				 */
				boost::spirit::rule< Scanner > Max_Forwards_;
				/*
				 * The Max-Forwards value is a decimal integer indicating the remaining 
				 * number of times this request message may be forwarded.
				 *
				 * Each proxy or gateway recipient of a TRACE or OPTIONS request 
				 * containing a Max-Forwards header field MUST check and update its value 
				 * prior to forwarding the request. If the received value is zero (0), 
				 * the recipient MUST NOT forward the request; instead, it MUST respond 
				 * as the final recipient. If the received Max-Forwards value is greater 
				 * than zero, then the forwarded message MUST contain an updated Max-Forwards
				 * field with a value decremented by one (1).
				 *
				 * The Max-Forwards header field MAY be ignored for all other methods defined 
				 * by this specification and for any extension methods for which it is not 
				 * explicitly referred to as part of that method definition.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.32
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Pragma general-header field is used to include implementation-specific 
				 * directives that might apply to any recipient along the request/response 
				 * chain. All pragma directives specify optional behavior from the viewpoint 
				 * of the protocol; however, some systems MAY require that behavior be 
				 * consistent with the directives.
				 */
				boost::spirit::rule< Scanner > Pragma_;
				boost::spirit::rule< Scanner > pragma_directive_;
				boost::spirit::rule< Scanner > extension_pragma_;
				/*
				 * When the no-cache directive is present in a request message, an application 
				 * SHOULD forward the request toward the origin server even if it has a cached 
				 * copy of what is being requested. This pragma directive has the same semantics 
				 * as the no-cache cache-directive (see section 14.9) and is defined here for 
				 * backward compatibility with HTTP/1.0. Clients SHOULD include both header 
				 * fields when a no-cache request is sent to a server not known to be HTTP/1.1 
				 * compliant.
				 *
				 * Pragma directives MUST be passed through by a proxy or gateway application,
				 * regardless of their significance to that application, since the directives 
				 * might be applicable to all recipients along the request/response chain. It 
				 * is not possible to specify a pragma for a specific recipient; however, any 
				 * pragma directive not relevant to a recipient SHOULD be ignored by that 
				 * recipient.
				 *
				 * HTTP/1.1 caches SHOULD treat “Pragma: no-cache” as if the client had sent 
				 * “Cache-Control: nocache”. No new Pragma directives will be defined in HTTP.
				 *
				 * Note: because the meaning of “Pragma: no-cache” as a response header field 
				 *       is not actually specified, it does not provide a reliable replacement
				 *       for “Cache-Control: no-cache” in a response.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.33
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Proxy-Authenticate response-header field MUST be included as part 
				 * of a 407 (Proxy Authentication Required) response. The field value 
				 * consists of a challenge that indicates the authentication scheme and 
				 * parameters applicable to the proxy for this Request-URI.
				 */
				boost::spirit::rule< Scanner > Proxy_Authenticate_;
				/*
				 * The HTTP access authentication process is described in “HTTP 
				 * Authentication: Basic and Digest Access Authentication” [43]. Unlike 
				 * WWW-Authenticate, the Proxy-Authenticate header field applies only to 
				 * the current connection and SHOULD NOT be passed on to downstream 
				 * clients. However, an intermediate proxy might need to obtain its own 
				 * credentials by requesting them from the downstream client, which in 
				 * some circumstances will appear as if the proxy is forwarding the 
				 * Proxy-Authenticate header field.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.34
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Proxy-Authorization request-header field allows the client to 
				 * identify itself (or its user) to a proxy which requires authentication. 
				 * The Proxy-Authorization field value consists of credentials containing 
				 * the authentication information of the user agent for the proxy and/or
				 * realm of the resource being requested.
				 */
				boost::spirit::rule< Scanner > Proxy_Authorization_;
				/*
				 * The HTTP access authentication process is described in “HTTP 
				 * Authentication: Basic and Digest Access Authentication” [43] . Unlike 
				 * Authorization, the Proxy-Authorization header field applies only to the
				 * next outbound proxy that demanded authentication using the 
				 * Proxy-Authenticate field. When multiple proxies are used in a chain, 
				 * the Proxy-Authorization header field is consumed by the first outbound 
				 * proxy that was expecting to receive credentials. A proxy MAY relay the 
				 * credentials from the client request to the next proxy if that is the 
				 * mechanism by which the proxies cooperatively authenticate a given request.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.35.1
				//////////////////////////////////////////////////////////////////////////
				/*
				 * Since all HTTP entities are represented in HTTP messages as sequences 
				 * of bytes, the concept of a byte range is meaningful for any HTTP entity.
				 * (However, not all clients and servers need to support byte-range 
				 * operations.)
				 *
				 * Byte range specifications in HTTP apply to the sequence of bytes in the 
				 * entity-body (not necessarily the same as the message-body).
				 *
				 * A byte range operation MAY specify a single range of bytes, or a set of 
				 * ranges within a single entity.
				 */
				boost::spirit::rule< Scanner > ranges_specifier_;
				boost::spirit::rule< Scanner > byte_ranges_specifier_;
				boost::spirit::rule< Scanner > byte_range_set_;
				boost::spirit::rule< Scanner > byte_range_spec_;
				boost::spirit::rule< Scanner > first_byte_pos_;
				boost::spirit::rule< Scanner > last_byte_pos_;
				/*
				 * The first-byte-pos value in a byte-range-spec gives the byte-offset of 
				 * the first byte in a range. The last-byte-pos value gives the byte-offset 
				 * of the last byte in the range; that is, the byte positions specified are
				 * inclusive. Byte offsets start at zero.
				 *
				 * If the last-byte-pos value is present, it MUST be greater than or equal 
				 * to the first-byte-pos in that byte-range-spec, or the byte-range-spec is
				 * syntactically invalid. The recipient of a byte-range-set that includes 
				 * one or more syntactically invalid byte-range-spec values MUST ignore the 
				 * header field that includes that byte-range-set.
				 *
				 * If the last-byte-pos value is absent, or if the value is greater than or 
				 * equal to the current length of the entity-body, last-byte-pos is taken 
				 * to be equal to one less than the current length of the entity-body in 
				 * bytes. By its choice of last-byte-pos, a client can limit the number of 
				 * bytes retrieved without knowing the size of the entity.
				 */
				boost::spirit::rule< Scanner > suffix_byte_range_spec_;
				boost::spirit::rule< Scanner > suffix_length_;
				/*
				 * A suffix-byte-range-spec is used to specify the suffix of the entity-body,
				 * of a length given by the suffix-length value. (That is, this form specifies
				 * the last N bytes of an entity-body.) If the entity is shorter than the 
				 * specified suffix-length, the entire entity-body is used.
				 * 
				 * If a syntactically valid byte-range-set includes at least one 
				 * byte-range-spec whose first-byte-pos is less than the current length of the
				 * entity-body, or at least one suffix-byte-range-spec with a non-zero 
				 * suffix-length, then the byterange-set is satisfiable. Otherwise, the 
				 * byte-range-set is unsatisfiable. If the byte-range-set is unsatisfiable, 
				 * the server SHOULD return a response with a status of 416 (Requested range 
				 * not satisfiable). Otherwise, the server SHOULD return a response with a 
				 * status of 206 (Partial Content) containing the satisfiable ranges of the 
				 * entity-body.
				 *
				 * Examples of byte-ranges-specifier values (assuming an entity-body of 
				 * length 10000):
				 * · The first 500 bytes (byte offsets 0-499, inclusive):
				 *   bytes=0-499
				 * · The second 500 bytes (byte offsets 500-999, inclusive):
				 *   bytes=500-999
				 * · The final 500 bytes (byte offsets 9500-9999, inclusive):
				 *   bytes=-500
				 * · Or
				 *   bytes=9500-
				 * · The first and last bytes only (bytes 0 and 9999):
				 *   bytes=0-0,-1
				 * · Several legal but not canonical specifications of the second 500 bytes 
				 *   (byte offsets 500-999, inclusive):
				 *   bytes=500-600,601-999
				 *   bytes=500-700,601-999
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.35.2
				//////////////////////////////////////////////////////////////////////////
				/*
				 * HTTP retrieval requests using conditional or unconditional GET methods 
				 * MAY request one or more sub-ranges of the entity, instead of the entire
				 * entity, using the Range request header, which applies to the entity 
				 * returned as the result of the request:
				 */
				boost::spirit::rule< Scanner > Range_;
				/*
				 * A server MAY ignore the Range header. However, HTTP/1.1 origin servers 
				 * and intermediate caches ought to support byte ranges when possible, 
				 * since Range supports efficient recovery from partially failed transfers, 
				 * and supports efficient partial retrieval of large entities.
				 *
				 * If the server supports the Range header and the specified range or 
				 * ranges are appropriate for the entity:
				 * · The presence of a Range header in an unconditional GET modifies what 
				 *   is returned if the GET is otherwise successful. In other words, the
				 *   response carries a status code of 206 (Partial Content) instead of
				 *   200 (OK).
				 * · The presence of a Range header in a conditional GET (a request using 
				 *   one or both of If-Modified-Since and If-None-Match, or one or both of 
				 *   If-Unmodified-Since and If-Match) modifies what is returned if the 
				 *   GET is otherwise successful and the condition is true. It does not 
				 *   affect the 304 (Not Modified) response returned if the conditional 
				 *   is false.
				 * In some cases, it might be more appropriate to use the If-Range header 
				 * (see section 14.27) in addition to the Range header.
				 *
				 * If a proxy that supports ranges receives a Range request, forwards the 
				 * request to an inbound server, and receives an entire entity in reply, 
				 * it SHOULD only return the requested range to its client. It SHOULD 
				 * store the entire received response in its cache if that is consistent 
				 * with its cache allocation policies.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.36
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Referer[sic] request-header field allows the client to specify, 
				 * for the server’s benefit, the address (URI) of the resource from which
				 * the Request-URI was obtained (the “referrer”, although the header field
				 * is misspelled.) The Referer request-header allows a server to generate
				 * lists of back-links to resources for interest, logging, optimized
				 * caching, etc. It also allows obsolete or mistyped links to be traced
				 * for maintenance. The Referer field MUST NOT be sent if the Request-URI
				 * was obtained from a source that does not have its own URI, such as 
				 * input from the user keyboard.
				 */
				boost::spirit::rule< Scanner > Referer_;
				/*
				 * Example:
				 *     Referer: http://www.w3.org/hypertext/DataSources/Overview.html
				 * If the field value is a relative URI, it SHOULD be interpreted 
				 * relative to the Request-URI. The URI MUST NOT include a fragment. 
				 * See section 15.1.3 for security considerations.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.37
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Retry-After response-header field can be used with a 503 (Service 
				 * Unavailable) response to indicate how long the service is expected to 
				 * be unavailable to the requesting client. This field MAY also be used 
				 * with any 3xx (Redirection) response to indicate the minimum time the 
				 * user-agent is asked wait before issuing the redirected request. The 
				 * value of this field can be either an HTTP-date or an integer number 
				 * of seconds (in decimal) after the time of the response.
				 */
				boost::spirit::rule< Scanner > Retry_After_;
				/*
				 * Two examples of its use are
				 *     Retry-After: Fri, 31 Dec 1999 23:59:59 GMT
				 *     Retry-After: 120
				 * In the latter example, the delay is 2 minutes.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.38
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Server response-header field contains information about the 
				 * software used by the origin server to handle the request. The field 
				 * can contain multiple product tokens (section 3.8) and comments
				 * identifying the server and any significant subproducts. The product
				 * tokens are listed in order of their significance for identifying the
				 * application.
				 */
				boost::spirit::rule< Scanner > Server_;
				/*
				 * Example:
				 *     Server: CERN/3.0 libwww/2.17
				 * If the response is being forwarded through a proxy, the proxy 
				 * application MUST NOT modify the Server response-header. Instead, it 
				 * SHOULD include a Via field (as described in section 14.45).
				 *
				 * Note: Revealing the specific software version of the server might 
				 *       allow the server machine to become more vulnerable to attacks 
				 *       against software that is known to contain security holes. Server
				 *       implementors are encouraged to make this field a configurable
				 *       option.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.39
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The TE request-header field indicates what extension transfer-codings
				 * it is willing to accept in the response and whether or not it is willing
				 * to accept trailer fields in a chunked transfer-coding. Its value may
				 * consist of the keyword “trailers” and/or a comma-separated list of
				 * extension transfer-coding names with optional accept parameters (as
				 * described in section 3.6).
				 */
				boost::spirit::rule< Scanner > TE_;
				boost::spirit::rule< Scanner > t_codings_;
				/*
				 * The presence of the keyword “trailers” indicates that the client is willing 
				 * to accept trailer fields in a chunked transfer-coding, as defined in section
				 * 3.6.1. This keyword is reserved for use with transfer-coding values even
				 * though it does not itself represent a transfer-coding.
				 *
				 * Examples of its use are:
				 *     TE: deflate
				 *     TE:
				 *     TE: trailers, deflate;q=0.5
				 * The TE header field only applies to the immediate connection. Therefore, the 
				 * keyword MUST be supplied within a Connection header field (section 14.10)
				 * whenever TE is present in an HTTP/1.1 message.
				 *
				 * A server tests whether a transfer-coding is acceptable, according to a TE 
				 * field, using these rules:
				 * 1. The “chunked” transfer-coding is always acceptable. If the keyword 
				 *    “trailers” is listed, the client indicates that it is willing to accept 
				 *    trailer fields in the chunked response on behalf of itself and an downstream
				 *    clients. The implication is that, if given, the client is stating that 
				 *    either all downstream clients are willing to accept trailer fields in
				 *    the forwarded response, or that it will attempt to buffer the response on
				 *    behalf of downstream recipients.
				 *
				 *    Note: HTTP/1.1 does not define any means to limit the size of a chunked
				 *          response such that a client can be assured of buffering the entire
				 *          response.
				 * 2. If the transfer-coding being tested is one of the transfer-codings 
				 *    listed in the TE field, then it is acceptable unless it is accompanied 
				 *    by a qvalue of 0. (As defined in section 3.9, a qvalue of 0 means 
				 *    “not acceptable.”)
				 * 3. If multiple transfer-codings are acceptable, then the acceptable 
				 *    transfer-coding with the highest non-zero qvalue is preferred. The 
				 *    “chunked” transfer-coding always has a qvalue of 1.
				 *
				 * If the TE field-value is empty or if no TE field is present, the only 
				 * transfer-coding is “chunked”. A message with no transfer-coding is always 
				 * acceptable.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.40
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Trailer general field value indicates that the given set of header
				 * fields is present in the trailer of a message encoded with chunked
				 * transfer-coding.
				 */
				boost::spirit::rule< Scanner > Trailer_;
				/*
				 * An HTTP/1.1 message SHOULD include a Trailer header field in a message
				 * using chunked transfer-coding with a non-empty trailer. Doing so allows
				 * the recipient to know which header fields to expect in the trailer.
				 *
				 * If no Trailer header field is present, the trailer SHOULD NOT include
				 * any header fields. See section 3.6.1 for restrictions on the use of
				 8 trailer fields in a “chunked” transfer-coding. 
				 *
				 * Message header fields listed in the Trailer header field MUST NOT include
				 * the following header fields:
				 * · Transfer-Encoding
				 * · Content-Length
				 * · Trailer
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.41
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Transfer-Encoding general-header field indicates what (if any) type
				 * of transformation has been applied to the message body in order to
				 * safely transfer it between the sender and the recipient. This differs
				 * from the contentcoding in that the transfer-coding is a property of the
				 * message, not of the entity.
				 */
				boost::spirit::rule< Scanner > Transfer_Encoding_;
				/*
				 * Transfer-codings are defined in section 3.6. An example is:
				 *     Transfer-Encoding: chunked
				 * If multiple encodings have been applied to an entity, the transfer-codings
				 * MUST be listed in the order in which they were applied. Additional
				 * information about the encoding parameters MAY be provided by other 
				 * entity-header fields not defined by this specification.
				 *
				 * Many older HTTP/1.0 applications do not understand the Transfer-Encoding
				 * header.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.42
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Upgrade general-header allows the client to specify what additional
				 * communication protocols it supports and would like to use if the server
				 * finds it appropriate to switch protocols. The server MUST use the
				 * Upgrade header field within a 101 (Switching Protocols) response to
				 * indicate which protocol(s) are being switched.
				 */
				boost::spirit::rule< Scanner > Upgrade_;
				/*
				 * For example,
				 *     Upgrade: HTTP/2.0, SHTTP/1.3, IRC/6.9, RTA/x11
				 *
				 * The Upgrade header field is intended to provide a simple mechanism for
				 * transition from HTTP/1.1 to some other, incompatible protocol. It does
				 * so by allowing the client to advertise its desire to use another
				 * protocol, such as a later version of HTTP with a higher major version
				 * number, even though the current request has been made using HTTP/1.1.
				 * This eases the difficult transition between incompatible protocols by
				 * allowing the client to initiate a request in the more commonly supported
				 * protocol while indicating to the server that it would like to use a
				 * “better” protocol if available (where “better” is determined by the
				 * server, possibly according to the nature of the method and/or resource
				 * being requested).
				 *
				 * The Upgrade header field only applies to switching application-layer
				 * protocols upon the existing transport-layer connection. Upgrade cannot
				 * be used to insist on a protocol change; its acceptance and use by the
				 * server is optional.
				 *
				 * The capabilities and nature of the application-layer communication
				 * after the protocol change is entirely dependent upon the new protocol
				 * chosen, although the first action after changing the protocol MUST be
				 * a response to the initial HTTP request containing the Upgrade header
				 * field.
				 *
				 * The Upgrade header field only applies to the immediate connection.
				 * Therefore, the upgrade keyword MUST be supplied within a Connection
				 * header field (section 14.10) whenever Upgrade is present in an HTTP/1.1
				 * message.
				 *
				 * The Upgrade header field cannot be used to indicate a switch to a
				 * protocol on a different connection. For that purpose, it is more
				 * appropriate to use a 301, 302, 303, or 305 redirection response.
				 *
				 * This specification only defines the protocol name “HTTP” for use by
				 * the family of Hypertext Transfer Protocols, as defined by the HTTP
				 * version rules of section 3.1 and future updates to this specification.
				 * Any token can be used as a protocol name; however, it will only be
				 * useful if both the client and server associate the name with the same
				 * protocol.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.43
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The User-Agent request-header field contains information about the user
				 * agent originating the request. This is for statistical purposes, the
				 * tracing of protocol violations, and automated recognition of user
				 * agents for the sake of tailoring responses to avoid particular user
				 * agent limitations. User agents SHOULD include this field with requests.
				 * The field can contain multiple product tokens (section 3.8) and comments
				 * identifying the agent and any subproducts which form a significant part
				 * of the user agent. By convention, the product tokens are listed in order
				 * of their significance for identifying the application.
				 */
				boost::spirit::rule< Scanner > User_Agent_;
				/*
				 * Example:
				 *     User-Agent: CERN-LineMode/2.15 libwww/2.17b3
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.44
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Vary field value indicates the set of request-header fields that
				 * fully determines, while the response is fresh, whether a cache is
				 * permitted to use the response to reply to a subsequent request without
				 * revalidation. For uncacheable or stale responses, the Vary field value
				 * advises the user agent about the criteria that were used to select
				 * the representation. A Vary field value of “*” implies that a cache
				 * cannot determine from the request headers of a subsequent request whether
				 * this response is the appropriate representation. See section 13.6 for
				 * use of the Vary header field by caches.
				 */
				boost::spirit::rule< Scanner > Vary_;
				/*
				 * An HTTP/1.1 server SHOULD include a Vary header field with any cacheable 
				 * response that is subject to serverdriven negotiation. Doing so allows a
				 * cache to properly interpret future requests on that resource and informs
				 * the user agent about the presence of negotiation on that resource. A
				 * server MAY include a Vary header field with a non-cacheable response
				 * that is subject to server-driven negotiation, since this might provide
				 * the user agent with useful information about the dimensions over which
				 * the response varies at the time of the response.
				 *
				 * A Vary field value consisting of a list of field-names signals that the
				 * representation selected for the response is based on a selection algorithm
				 * which considers ONLY the listed request-header field values in selecting
				 * the most appropriate representation. A cache MAY assume that the same 
				 * selection will be made for future requests with the same values for the
				 * listed field names, for the duration of time for which the response is
				 * fresh.
				 *
				 * The field-names given are not limited to the set of standard request-header
				 * fields defined by this specification. Field names are case-insensitive.
				 *
				 * A Vary field value of “*” signals that unspecified parameters not limited
				 * to the request-headers (e.g., the network address of the client), play a
				 * role in the selection of the response representation. The “*” value MUST
				 * NOT be generated by a proxy server; it may only be generated by an origin
				 * server.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.45
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Via general-header field MUST be used by gateways and proxies to
				 * indicate the intermediate protocols and recipients between the user agent
				 * and the server on requests, and between the origin server and the client
				 * on responses. It is analogous to the “Received” field of RFC 822 [9] and
				 * is intended to be used for tracking message forwards, avoiding request
				 * loops, and identifying the protocol capabilities of all senders along the
				 * request/response chain.
				 */
				boost::spirit::rule< Scanner > Via_;
				boost::spirit::rule< Scanner > received_protocol_;
				boost::spirit::rule< Scanner > protocol_name_;
				boost::spirit::rule< Scanner > protocol_version_;
				boost::spirit::rule< Scanner > received_by_;
				boost::spirit::rule< Scanner > pseudonym_;
				/*
				 * The received-protocol indicates the protocol version of the message 
				 * received by the server or client along each segment of the 
				 * request/response chain. The received-protocol version is appended to 
				 * the Via field value when the message is forwarded so that information
				 * about the protocol capabilities of upstream applications remains visible
				 * to all recipients.
				 *
				 * The protocol-name is optional if and only if it would be “HTTP”. The
				 * received-by field is normally the host and optional port number of a
				 * recipient server or client that subsequently forwarded the message.
				 * However, if the real host is considered to be sensitive information,
				 * it MAY be replaced by a pseudonym. If the port is not given, it MAY
				 * be assumed to be the default port of the received-protocol.
				 *
				 * Multiple Via field values represents each proxy or gateway that has
				 * forwarded the message. Each recipient MUST append its information such
				 * that the end result is ordered according to the sequence of forwarding
				 * applications.
				 *
				 * Comments MAY be used in the Via header field to identify the software of
				 * the recipient proxy or gateway, analogous to the User-Agent and Server
				 * header fields. However, all comments in the Via field are optional and
				 * MAY be removed by any recipient prior to forwarding the message.
				 *
				 * For example, a request message could be sent from an HTTP/1.0 user agent
				 * to an internal proxy code-named “fred”, which uses HTTP/1.1 to forward
				 * the request to a public proxy at nowhere.com, which completes the request
				 * by forwarding it to the origin server at www.ics.uci.edu. The request
				 * received by www.ics.uci.edu would then have the following Via header field:
				 *     Via: 1.0 fred, 1.1 nowhere.com (Apache/1.1)
				 * Proxies and gateways used as a portal through a network firewall SHOULD NOT,
				 * by default, forward the names and ports of hosts within the firewall region.
				 * This information SHOULD only be propagated if explicitly enabled. If not
				 * enabled, the received-by host of any host behind the firewall SHOULD be
				 * replaced by an appropriate pseudonym for that host.
				 *
				 * For organizations that have strong privacy requirements for hiding internal
				 * structures, a proxy MAY combine an ordered subsequence of Via header field
				 * entries with identical received-protocol values into a single such entry.
				 * For example,
				 *     Via: 1.0 ricky, 1.1 ethel, 1.1 fred, 1.0 lucy
				 * could be collapsed to
				 *     Via: 1.0 ricky, 1.1 mertz, 1.0 lucy
				 * 
				 * Applications SHOULD NOT combine multiple entries unless they are all under
				 * the same organizational control and the hosts have already been replaced by
				 * pseudonyms. Applications MUST NOT combine entries which have different
				 * received-protocol values.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.46
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The Warning general-header field is used to carry additional information
				 * about the status or transformation of a message which might not be
				 * reflected in the message. This information is typically used to warn
				 * about a possible lack of semantic transparency from caching operations
				 * or transformations applied to the entity body of the message.
				 *
				 * Warning headers are sent with responses using:
				 */
				boost::spirit::rule< Scanner > Warning_;
				boost::spirit::rule< Scanner > warning_value_;
				boost::spirit::rule< Scanner > warn_code_;
				boost::spirit::rule< Scanner > warn_agent_;
				boost::spirit::rule< Scanner > warn_text_;
				boost::spirit::rule< Scanner > warn_date_;
				/*
				 * The warn-text SHOULD be in a natural language and character set that is
				 * most likely to be intelligible to the human user receiving the response.
				 * This decision MAY be based on any available knowledge, such as the location
				 * of the cache or user, the Accept-Language field in a request, the
				 * Content-Language field in a response, etc. The default language is English
				 * and the default character set is ISO-8859-1.
				 * 
				 * If a character set other than ISO-8859-1 is used, it MUST be encoded in the
				 * warn-text using the method described in RFC 2047 [14].
				 * 
				 * Warning headers can in general be applied to any message, however some specific
				 * warn-codes are specific to caches and can only be applied to response messages.
				 * New Warning headers SHOULD be added after any existing Warning headers. A cache
				 * MUST NOT delete any Warning header that it received with a message. However, 
				 * if a cache successfully validates a cache entry, it SHOULD remove any Warning 
				 * headers previously attached to that entry except as specified for specific
				 * Warning codes. It MUST then add any Warning headers received in the validating
				 * response. In other words, Warning headers are those that would be attached to
				 * the most recent relevant response.
				 *
				 * When multiple Warning headers are attached to a response, the user agent ought 
				 * to inform the user of as many of them as possible, in the order that they appear
				 * in the response. If it is not possible to inform the user of all of the
				 * warnings, the user agent SHOULD follow these heuristics:
				 * · Warnings that appear early in the response take priority over those appearing 
				 *   later in the response.
				 * · Warnings in the user’s preferred character set take priority over warnings in
				 *   other character sets but with identical warn-codes and warn-agents.
				 * Systems that generate multiple Warning headers SHOULD order them with this user 
				 * agent behavior in mind.
				 * 
				 * Requirements for the behavior of caches with respect to Warnings are stated 
				 * in section 13.1.2.
				 * 
				 * This is a list of the currently-defined warn-codes, each with a recommended 
				 * warn-text in English, and a description of its meaning.
				 *
				 * 110 Response is stale
				 *     MUST be included whenever the returned response is stale.
				 * 111 Revalidation failed
				 *     MUST be included if a cache returns a stale response because an attempt 
				 *     to revalidate the response failed, due to an inability to reach the server.
				 * 112 Disconnected operation
				 *     SHOULD be included if the cache is intentionally disconnected from the rest 
				 *     of the network for a period of time.
				 * 113 Heuristic expiration
				 *     MUST be included if the cache heuristically chose a freshness lifetime 
				 *     greater than 24 hours and the response’s age is greater than 24 hours.
				 * 199 Miscellaneous warning
				 *     The warning text MAY include arbitrary information to be presented to a 
				 *     human user, or logged. A system receiving this warning MUST NOT take any 
				 *     automated action, besides presenting the warning to the user.
				 * 214 Transformation applied
				 *     MUST be added by an intermediate cache or proxy if it applies any 
				 *     transformation changing the content-coding (as specified in the 
				 *     Content-Encoding header) or media-type (as specified in the Content-Type 
				 *     header) of the response, or the entity-body of the response, unless this
				 *     Warning code already appears in the response.
				 * 299 Miscellaneous persistent warning
				 *     The warning text MAY include arbitrary information to be presented to a 
				 *     human user, or logged. A system receiving this warning MUST NOT take any 
				 *     automated action.
				 *
				 * If an implementation sends a message with one or more Warning headers whose 
				 * version is HTTP/1.0 or lower, then the sender MUST include in each warning-value 
				 * a warn-date that matches the date in the response.
				 *
				 * If an implementation receives a message with a warning-value that includes a 
				 * warn-date, and that warndate is different from the Date value in the response,
				 * then that warning-value MUST be deleted from the message before storing, 
				 * forwarding, or using it. (This prevents bad consequences of naive caching of
				 * Warning header fields.) If all of the warning-values are deleted for this 
				 * reason, the Warning header MUST be deleted as well.
				 */
				//////////////////////////////////////////////////////////////////////////
				// Section 14.47
				//////////////////////////////////////////////////////////////////////////
				/*
				 * The WWW-Authenticate response-header field MUST be included in 401 
				 * (Unauthorized) response messages. The field value consists of at least 
				 * one challenge that indicates the authentication scheme(s) and parameters
				 * applicable to the Request-URI.
				 */
				boost::spirit::rule< Scanner > WWW_Authenticate_;
				/*
				 * The HTTP access authentication process is described in “HTTP 
				 * Authentication: Basic and Digest Access Authentication” [43]. User
				 * agents are advised to take special care in parsing the WWW-Authenticate
				 * field value as it might contain more than one challenge, or if more
				 * than one WWW-Authenticate header field is provided, the contents of
				 * a challenge itself can contain a comma-separated list of authentication
				 * parameters.
				 */
				//////////////////////////////////////////////////////////////////////////
				// RFC 2396, Section 2
				//////////////////////////////////////////////////////////////////////////
				boost::spirit::rule< Scanner > uric_;
				boost::spirit::rule< Scanner > reserved_;
				boost::spirit::rule< Scanner > unreserved_;
				boost::spirit::rule< Scanner > mark_;
				boost::spirit::rule< Scanner > escaped_;
				boost::spirit::rule< Scanner > delims_;
				boost::spirit::rule< Scanner > unwise_;
				//////////////////////////////////////////////////////////////////////////
				// RFC 2396, section 3
				//////////////////////////////////////////////////////////////////////////
				boost::spirit::rule< Scanner > absoluteURI_;
				boost::spirit::rule< Scanner > hier_part_;
				boost::spirit::rule< Scanner > net_path_;
				boost::spirit::rule< Scanner > abs_path_;
				boost::spirit::rule< Scanner > opaque_part_;
				boost::spirit::rule< Scanner > uric_no_slash_;
				boost::spirit::rule< Scanner > scheme_;
				boost::spirit::rule< Scanner > authority_;
				boost::spirit::rule< Scanner > reg_name_;
				boost::spirit::rule< Scanner > server_;
				boost::spirit::rule< Scanner > userinfo_;
				boost::spirit::rule< Scanner > hostport_;
				boost::spirit::rule< Scanner > host_;
				boost::spirit::rule< Scanner > hostname_;
				boost::spirit::rule< Scanner > domainlabel_;
				boost::spirit::rule< Scanner > toplabel_;
				boost::spirit::rule< Scanner > IPV4address_;
				boost::spirit::rule< Scanner > port_;
				boost::spirit::rule< Scanner > path_;
				boost::spirit::rule< Scanner > path_segments_;
				boost::spirit::rule< Scanner > segment_;
				boost::spirit::rule< Scanner > param_;
				boost::spirit::rule< Scanner > pchar_;
				boost::spirit::rule< Scanner > query_;
				boost::spirit::rule< Scanner > URI_reference_;
				boost::spirit::rule< Scanner > fragment_;
				boost::spirit::rule< Scanner > relativeURI_;
				boost::spirit::rule< Scanner > rel_path_;
				boost::spirit::rule< Scanner > rel_segment_;
			};
		};
	}
}

#endif


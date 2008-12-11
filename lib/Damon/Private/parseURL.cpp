#include "parseURL.h"
#include <iostream>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

namespace Damon
{
	namespace Private
	{
		boost::tuple< std::string, std::string, boost::uint16_t, std::string, std::string, std::string > parseURL(const std::string & url)
		{
			boost::regex re("^(([A-Za-z]+)://)?(([^:/@]+)(:([^/@]+))?@)?([^:/]+)(:([0-9]+))?(.*)");
			boost::smatch mr;
			if (!boost::regex_match(url, mr, re))
				throw std::runtime_error("Invalid URL");	// be more eloquent HERE
			else
			{ /* all is well so far */ }
			assert(mr.size() == 11);
			enum
			{
				re_protocol = 2,
				re_username = 4,
				re_password = 6,
				re_host = 7,
				re_port = 9,
				re_path = 10,
			};

			std::cerr << std::endl;
			std::cerr << 0 << ": " << mr[0].matched << " " << std::string(mr[0].first, mr[0].second) << std::endl;
			std::cerr << 1 << ": " << mr[1].matched << " " << std::string(mr[1].first, mr[1].second) << std::endl;
			std::cerr << 2 << ": " << mr[2].matched << " " << std::string(mr[2].first, mr[2].second) << std::endl;
			std::cerr << 3 << ": " << mr[3].matched << " " << std::string(mr[3].first, mr[3].second) << std::endl;
			std::cerr << 4 << ": " << mr[4].matched << " " << std::string(mr[4].first, mr[4].second) << std::endl;
			std::cerr << 5 << ": " << mr[5].matched << " " << std::string(mr[5].first, mr[5].second) << std::endl;
			std::cerr << 6 << ": " << mr[6].matched << " " << std::string(mr[6].first, mr[6].second) << std::endl;
			std::cerr << 7 << ": " << mr[7].matched << " " << std::string(mr[7].first, mr[7].second) << std::endl;
			std::cerr << 8 << ": " << mr[8].matched << " " << std::string(mr[8].first, mr[8].second) << std::endl;
			std::cerr << 9 << ": " << mr[9].matched << " " << std::string(mr[9].first, mr[9].second) << std::endl;
			std::cerr << 10 << ": " << mr[10].matched << " \"" << std::string(mr[10].first, mr[10].second) << "\" " << std::string(mr[10].first, mr[10].second).size() << " " << std::string(mr[10].first, mr[10].second).empty() << std::endl;

			std::string protocol(mr[re_protocol].matched ? std::string(mr[re_protocol].first, mr[re_protocol].second) : "http");
			if (protocol != "http" && protocol != "https")
			{
				throw std::runtime_error("Unknown protocol");	// be more eloquent HERE
			}
			else
			{ /* all is well */ }

			return boost::make_tuple(
				protocol,
				std::string(mr[re_host].first, mr[re_host].second),
				mr[re_port].matched ? boost::lexical_cast< boost::uint16_t >(std::string(mr[re_port].first, mr[re_port].second)) : (protocol == "http" ? 80  : 443),
				mr[re_path].matched ? (std::string(mr[re_path].first, mr[re_path].second).empty() ? "/" : std::string(mr[re_path].first, mr[re_path].second)) : "/",
				mr[re_username], mr[re_password]);
		}
		
		std::string extractHost(const std::string & url)
		{
			// Host name includes both domain name and port number 
			boost::regex re("^([A-Za-z]+://)?([^:/@]+(:[^/@]+)?@)?([^:/]+(:[0-9]+)?).*");
			boost::smatch mr;
			if (!boost::regex_match(url, mr, re))
				return "";
			else
			{ /* all is well so far */ }
			assert(mr.size() == 6);
			assert(mr[4].matched);
			return mr[4];
		}
	}
}



#include "parseURL.h"
#include <iostream>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

namespace Damon
{
	namespace Private
	{
		boost::tuple< std::string, std::string, boost::uint16_t, std::string > parseURL(const std::string & url)
		{
			boost::regex re("^(([A-Za-z]+)://)?([^:/]+)(:([0-9]+))?(.*)");
			boost::smatch mr;
			if (!boost::regex_match(url, mr, re))
				throw std::runtime_error("Invalid URL");	// be more eloquent HERE
			else
			{ /* all is well so far */ }
			assert(mr.size() == 7);

			//std::cerr << std::endl;
			//std::cerr << 0 << ": " << mr[0].matched << " " << std::string(mr[0].first, mr[0].second) << std::endl;
			//std::cerr << 1 << ": " << mr[1].matched << " " << std::string(mr[1].first, mr[1].second) << std::endl;
			//std::cerr << 2 << ": " << mr[2].matched << " " << std::string(mr[2].first, mr[2].second) << std::endl;
			//std::cerr << 3 << ": " << mr[3].matched << " " << std::string(mr[3].first, mr[3].second) << std::endl;
			//std::cerr << 4 << ": " << mr[4].matched << " " << std::string(mr[4].first, mr[4].second) << std::endl;
			//std::cerr << 5 << ": " << mr[5].matched << " " << std::string(mr[5].first, mr[5].second) << std::endl;
			//std::cerr << 6 << ": " << mr[6].matched << " \"" << std::string(mr[6].first, mr[6].second) << "\" " << std::string(mr[6].first, mr[6].second).size() << " " << std::string(mr[6].first, mr[6].second).empty() << std::endl;

			std::string protocol(mr[2].matched ? std::string(mr[2].first, mr[2].second) : "http");
			if (protocol != "http" && protocol != "https")
			{
				throw std::runtime_error("Unknown protocol");	// be more eloquent HERE
			}
			else
			{ /* all is well */ }

			return boost::make_tuple(
				protocol,
				std::string(mr[3].first, mr[3].second),
				mr[5].matched ? boost::lexical_cast< boost::uint16_t >(std::string(mr[5].first, mr[5].second)) : (protocol == "http" ? 80  : 443),
				mr[6].matched ? (std::string(mr[6].first, mr[6].second).empty() ? "/" : std::string(mr[6].first, mr[6].second)) : "/");
		}
		
		std::string extractHost(const std::string & url)
		{
			// Host name includes both domain name and port number 
			boost::regex re("^([A-Za-z]+://)?([^:/]+(:[0-9]+)?).*");
			boost::smatch mr;
			if (!boost::regex_match(url, mr, re))
				return "";
			else
			{ /* all is well so far */ }
			assert(mr.size() == 4);
			assert(mr[2].matched);
			return mr[2];
		}
	}
}



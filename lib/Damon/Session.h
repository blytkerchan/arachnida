#ifndef damon_session_h
#define damon_session_h

#include "Details/prologue.h"
#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace Spin { class Connection; }
namespace Scorpion { class Context; }
namespace Damon
{
	struct Response;
	class Request;

	class DAMON_API Session : boost::noncopyable
	{
	public :
		Session();
		~Session();

		void setContext(const Scorpion::Context & context);

	private :
		typedef std::map< std::string, boost::shared_ptr< Spin::Connection > > ConnectionCache_;

		Scorpion::Context * context_;
		ConnectionCache_ connection_cache_;

		friend DAMON_API Response send(Session & /*session*/, const Request &/* request*/);
		friend DAMON_API std::vector< Response > send(Session & /*session*/, const std::vector< Request > & requests);
	};
}

#endif

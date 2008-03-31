#include "Session.h"
#include <Scorpion/Context.h>

namespace Damon
{
	Session::Session()
		: context_(0)
	{ /* no-op */ }

	Session::~Session()
	{
		delete context_;
	}

	void Session::setContext(const Scorpion::Context & context)
	{
		if (context_)
		{
			delete context_;
			context_ = 0;
		}
		else
		{ /* don't already have a context */ }

		context_ = new Scorpion::Context(context);
	}
}

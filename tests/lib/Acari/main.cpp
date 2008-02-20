#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TextTestRunner.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestResult.h>
#include <boost/filesystem/path.hpp>
#if defined(_WIN32) && !defined(__CYGWIN__)
#include <Agelena/Loggers/DefaultWindowsLogger.h>
#else
#include <Agelena/Loggers/DefaultPOSIXLogger.h>
#endif

boost::filesystem::path cert_path__;

int main(int argc, char ** argv)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
	Agelena::Loggers::DefaultWindowsLogger logger;
#else
	Agelena::Loggers::DefaultPOSIXLogger logger;
#endif

	if (argc > 1)
		cert_path__ = boost::filesystem::path(argv[1], boost::filesystem::native);
	else
		std::cerr << "No path to the SSL certificate for the tests was specified - test won't be performed" << std::endl;;

	// UnitTest
	CppUnit::TextTestRunner runner;
	runner.setOutputter(CppUnit::CompilerOutputter::defaultOutputter(&runner.result(), std::cerr));
	CppUnit::BriefTestProgressListener progress;
	runner.eventManager().addListener( &progress );
	CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
	runner.addTest( registry.makeTest() );
	runner.run();

	return 0;
}

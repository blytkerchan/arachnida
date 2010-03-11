#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TextTestRunner.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestResult.h>
#include <boost/filesystem/path.hpp>

boost::filesystem::path cert_path__;

int main(/*int argc, char ** argv*/)
{
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

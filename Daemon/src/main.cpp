#include <iostream>

#include <algorithm>

#include <Poco/Util/ServerApplication.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/Stopwatch.h>
#include <Poco/File.h>
#include <Poco/SimpleFileChannel.h>
#include <Poco/AutoPtr.h>
#include <Poco/PatternFormatter.h>
#include <Poco/FormattingChannel.h>

#include "scanner/Scanner.h"
#include "parser/Parser.h"
#include "interface/TcpServer.h"
#include "artefact/Artefact.h"
#include "util/LoggerMacros.hpp"


using Poco::Util::Application;
using Poco::Util::ServerApplication;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::OptionCallback;
using Poco::Util::HelpFormatter;

void
run(const std::string& path) {
	// Find files
	titanic::scanner::Scanner scanner;
	Poco::Stopwatch stopWatch;
	stopWatch.start();
	std::vector<titanic::artefact::File> foundFiles = scanner.findFiles(path);
	unsigned long scannerTime = (unsigned long)stopWatch.elapsed();

	// Parse files
	unsigned long totalSize = 0;
	for (auto fileIt = foundFiles.begin(); fileIt < foundFiles.end(); ++fileIt) {
		titanic::parser::Parser parser(fileIt->getPath());
		parser.parse();
		fileIt->setHash(parser.getHash());
		fileIt->setSize(parser.getSize());
		totalSize += parser.getSize();
	}
	unsigned long parserTime = ((unsigned long)stopWatch.elapsed()) - scannerTime;

	// Smallest files
	const unsigned int displayCount = 20;
	std::cout << "========================================================= " << displayCount << " smallest files:" << std::endl;
	unsigned int displayCounter = 0;
	std::sort(foundFiles.begin(), foundFiles.end(), [](titanic::artefact::File a, titanic::artefact::File b) { return a.getSize() < b.getSize(); });
	for (auto fileIt = foundFiles.begin(); fileIt < foundFiles.end() && displayCounter < displayCount; ++fileIt) {
		std::cout << fileIt->getPath() << " (" << (fileIt->getSize() / 1024) << " kb): " << fileIt->getHash() << std::endl;
		++displayCounter;
	}

	// Largest files
	std::cout << "========================================================= " << displayCount << " largest files:" << std::endl;
	displayCounter = 0;
	for (auto fileIt = foundFiles.rbegin(); fileIt < foundFiles.rend() && displayCounter < displayCount; ++fileIt) {
		std::cout << fileIt->getPath() << " (" << (fileIt->getSize() / 1024) << " kb): " << fileIt->getHash() << std::endl;
		++displayCounter;
	}

	// Totals
	std::cout << std::endl;
	std::cout << "============================================================================" << std::endl;
	std::cout << "Found [" << foundFiles.size() << "] files in [" << (scannerTime / 1000) << "] msec" << std::endl;
	std::cout << "Parsed [" << (totalSize / 1024) << "] kb in [" << (parserTime / 1000) << "] msec" << std::endl;

	// Speeds
	double parserTime_sec = ((double)parserTime / 1000 / 1000);
	if (parserTime_sec == 0) {
		parserTime_sec = 1;
	}
	double scannerTime_sec = ((double)scannerTime / 1000 / 1000);
	if (scannerTime_sec == 0) {
		scannerTime_sec = 1;
	}
	std::cout << "Scan speed: " << (foundFiles.size() / scannerTime_sec) << " files/sec" << std::endl;
	std::cout << "Parse speed: " << ((totalSize / 1024) / parserTime_sec) << " kb/sec" << std::endl;
	if (foundFiles.size()) {
		std::cout << "Avg file size: " << ((totalSize / 1024) / foundFiles.size()) << " kb" << std::endl;
	}
	else {
		std::cout << "Avg file size: 0 kb" << std::endl;
	}
}

void run2(const std::string& path) {
	Poco::File file(path);
	titanic::artefact::Artefact artefact(file);
	Poco::Stopwatch stopWatch;
	stopWatch.start();
	artefact.initialize();
	unsigned long start = (unsigned long)stopWatch.elapsed();
	artefact.scan();
	unsigned long scanTime = ((unsigned long)stopWatch.elapsed()) - start;
	std::cout << "ScanTime: " << (scanTime / 1000) << std::endl;
	std::cout << "Files: " << artefact.countFiles() << std::endl;
	std::cout << "Directories: " << artefact.countDirectories() << std::endl;
	std::cout << "Nodes: " << artefact.countAll() << std::endl;
	double scanTime_sec = ((double)scanTime / 1000 / 1000);
	if (scanTime_sec == 0) {
		scanTime_sec = 1;
	}
	std::cout << "Scan speed: " << (artefact.countAll() / scanTime_sec) << " files/sec" << std::endl;
}

void initLogger() {
	Poco::AutoPtr<Poco::SimpleFileChannel> fileChannel(new Poco::SimpleFileChannel("log.txt"));
	Poco::AutoPtr<Poco::PatternFormatter> patternFormatter(new Poco::PatternFormatter());
	patternFormatter->setProperty("pattern", "%Y-%m-%d %H:%M:%S.%F [%T] %s (%U:%u): [%p] %t");
	Poco::AutoPtr<Poco::FormattingChannel> formattingChannel(new Poco::FormattingChannel(patternFormatter, fileChannel));
	Poco::Logger::root().setChannel(formattingChannel);
	Poco::Logger::root().setLevel(Poco::Message::PRIO_TRACE);
}

class TitanicDaemon : public Poco::Util::ServerApplication
{
public:
	TitanicDaemon() : _helpRequested(false)
	{
	}

	~TitanicDaemon()
	{
	}

protected:
	void initialize(Application& self)
	{
		loadConfiguration(); // load default configuration files, if present
		ServerApplication::initialize(self);
	}

	void uninitialize()
	{
		ServerApplication::uninitialize();
	}

	void defineOptions(OptionSet& options)
	{
		ServerApplication::defineOptions(options);

		options.addOption(
			Option("help", "h", "display help information on command line arguments")
			.required(false)
			.repeatable(false)
			.callback(OptionCallback<TitanicDaemon>(this, &TitanicDaemon::handleHelp)));
	}

	void handleHelp(const std::string& name, const std::string& value)
	{
		_helpRequested = true;
		displayHelp();
		stopOptionsProcessing();
	}

	void displayHelp()
	{
		HelpFormatter helpFormatter(options());
		helpFormatter.setCommand(commandName());
		helpFormatter.setUsage("OPTIONS");
		helpFormatter.setHeader("A sample server application that demonstrates some of the features of the Util::ServerApplication class.");
		helpFormatter.format(std::cout);
	}

	int main(const ArgVec& args)
	{
		initLogger();
		if (_helpRequested)
			return Application::EXIT_OK;

		// Read input
		std::string listPath;
		if (args.size() < 1) {
			std::cout << "No path specified, listing current dir";
			listPath = ".";
		}
		else {
			std::cout << "Listing for " << args[0] << std::endl;
			listPath = args[0];
		}
		run2(listPath);
		::run(listPath);

		std::cout << "Waiting..." << std::endl;
		waitForTerminationRequest();
		return Application::EXIT_OK;
	}

private:
	bool _helpRequested;
};


POCO_SERVER_MAIN(TitanicDaemon)
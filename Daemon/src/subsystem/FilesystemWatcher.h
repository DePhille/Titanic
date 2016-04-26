#pragma once

#include <Poco/Util/Subsystem.h>
#include "../util/Logger.h"

namespace titanic {
namespace subsystem {

class FilesystemWatcher : public Poco::Util::Subsystem {
public:
	FilesystemWatcher();
	~FilesystemWatcher();

	const char* name() const;
	void initialize(Poco::Util::Application& app);
	void uninitialize();

private:
	static util::Logger Logger;
};

}
}
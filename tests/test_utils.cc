#include "test_utils.h"

#include <signal.h>
#include <sys/prctl.h>
#include <fstream>
#include <sstream>

#include <boost/filesystem.hpp>

std::string TestUtils::getFreePort() {
  int s = socket(AF_INET, SOCK_STREAM, 0);
  if (s == -1) {
    std::cout << "socket() failed: " << errno;
    throw std::runtime_error("Could not open socket");
  }
  struct sockaddr_in soc_addr;
  memset(&soc_addr, 0, sizeof(struct sockaddr_in));
  soc_addr.sin_family = AF_INET;
  soc_addr.sin_addr.s_addr = INADDR_ANY;
  soc_addr.sin_port = htons(INADDR_ANY);

  if (bind(s, (struct sockaddr *)&soc_addr, sizeof(soc_addr)) == -1) {
    std::cout << "bind() failed: " << errno;
    throw std::runtime_error("Could not bind socket");
  }

  struct sockaddr_in sa;
  unsigned int sa_len = sizeof(sa);
  if (getsockname(s, (struct sockaddr *)&sa, &sa_len) == -1) {
    throw std::runtime_error("getsockname failed");
  }
  close(s);
  std::ostringstream ss;
  ss << ntohs(sa.sin_port);
  return ss.str();
}

void TestUtils::writePathToConfig(const boost::filesystem::path &toml_in, const boost::filesystem::path &toml_out,
                                  const boost::filesystem::path &storage_path) {
  // Append our temp_dir path as storage.path to the config file. This is a hack
  // but less annoying than the alternatives.
  boost::filesystem::copy_file(toml_in, toml_out);
  std::string conf_path_str = toml_out.string();
  std::ofstream cs(conf_path_str.c_str(), std::ofstream::app);
  cs << "\n[storage]\npath = " << storage_path.string() << "\n";
}

TestHelperProcess::TestHelperProcess(const std::string &argv0, const std::string &argv1) {
  pid_ = fork();
  if (pid_ == -1) {
    throw std::runtime_error("Failed to execute process:" + argv0);
  }
  if (pid_ == 0) {
    prctl(PR_SET_PDEATHSIG, SIGTERM);
    execlp(argv0.c_str(), argv0.c_str(), argv1.c_str(), (char *)0);
  }
}

TestHelperProcess::~TestHelperProcess() {
  assert(pid_);
  kill(pid_, SIGINT);
}

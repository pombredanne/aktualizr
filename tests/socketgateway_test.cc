#include <gtest/gtest.h>

#include <stdio.h>
#include <cstdlib>

#include <boost/filesystem.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>

#include "commands.h"
#include "config.h"
#include "events.h"
#include "socketgateway.h"
#include "types.h"
#include "utils.h"

const std::string server = "http://127.0.0.1:8800";
boost::filesystem::path socket_path;

TEST(EventsTest, broadcasted) {
  TemporaryDirectory temp_dir;
  NetworkConfig network_conf;
  network_conf.socket_commands_path = (temp_dir.Path() / "sota-commands.socket").string();
  network_conf.socket_events_path = (temp_dir.Path() / "sota-events.socket").string();
  network_conf.socket_events.push_back("InstalledSoftwareNeeded");
  Config conf;
  conf.network = network_conf;

  command::Channel chan;

  SocketGateway gateway(conf, &chan);
  sleep(1);
  std::string cmd =
      (socket_path / "events.py").string() + " " + (temp_dir.Path() / "sota-events.socket").string() + " &";
  EXPECT_EQ(system(cmd.c_str()), 0);
  sleep(1);
  gateway.processEvent(boost::make_shared<event::InstalledSoftwareNeeded>(event::InstalledSoftwareNeeded()));
  sleep(1);
  std::ifstream file_stream((temp_dir.Path() / "sota-events.socket.txt").c_str());
  std::string content;
  std::getline(file_stream, content);
  EXPECT_EQ("{\"fields\":[],\"variant\":\"InstalledSoftwareNeeded\"}", content);
}

TEST(EventsTest, not_broadcasted) {
  TemporaryDirectory temp_dir;
  NetworkConfig network_conf;
  network_conf.socket_commands_path = (temp_dir.Path() / "sota-commands.socket").string();
  network_conf.socket_events_path = (temp_dir.Path() / "sota-events.socket").string();
  network_conf.socket_events.empty();
  Config conf;
  conf.network = network_conf;

  command::Channel chan;

  SocketGateway gateway(conf, &chan);
  std::string cmd =
      (socket_path / "events.py").string() + " " + (temp_dir.Path() / "sota-events.socket").string() + " &";
  EXPECT_EQ(system(cmd.c_str()), 0);
  sleep(1);
  gateway.processEvent(boost::make_shared<event::InstalledSoftwareNeeded>(event::InstalledSoftwareNeeded()));
  sleep(1);
  std::ifstream file_stream((temp_dir.Path() / "sota-events.socket.txt").c_str());
  std::string content;
  std::getline(file_stream, content);
  EXPECT_EQ("", content);
}

TEST(CommandsTest, recieved) {
  TemporaryDirectory temp_dir;
  NetworkConfig network_conf;
  network_conf.socket_commands_path = (temp_dir.Path() / "sota-commands.socket").string();
  network_conf.socket_events_path = (temp_dir.Path() / "sota-events.socket").string();
  Config conf;
  conf.network = network_conf;

  command::Channel chan;

  SocketGateway gateway(conf, &chan);
  std::string cmd =
      (socket_path / "commands.py").string() + " " + (temp_dir.Path() / "sota-commands.socket").string() + " &";
  EXPECT_EQ(system(cmd.c_str()), 0);
  sleep(1);
  boost::shared_ptr<command::BaseCommand> command;
  chan >> command;

  EXPECT_EQ(command->variant, "Shutdown");
}

#ifndef __NO_MAIN__
int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  if (argc != 2) {
    std::cerr << "Error: " << argv[0] << " requires the path to the fake_unix_socket directory as an input argument.\n";
    return EXIT_FAILURE;
  }
  socket_path = argv[1];
  return RUN_ALL_TESTS();
}
#endif

#pragma once
#include <list>
#include <map>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <curl/curl.h>

#include "config.hpp"
#include "http.hpp"
#include "jenkins.hpp"
#include "command_line_options_parser.hpp"

using namespace std;
namespace cic {
  class tool {
  public:
    int run(int argc, char const *argv []) {
      boost::filesystem::path current_path = boost::filesystem::current_path();
      std::list<boost::filesystem::path> config_files;

      find_config_files_on_path(current_path, config_files);

      auto runtime_config = read_configuration(config_files);

      command_line_options_parser cmdline_parser;
      runtime_config = cmdline_parser.parse(argc, argv).merge(runtime_config);

      if (runtime_config.just_need_help()) {
        cmdline_parser.show_help(std::cout);
      } else if (runtime_config.show_version_only()) {
        cmdline_parser.show_version(std::cout);
      } else {

        if (!runtime_config.is_valid()) {
          std::cerr << "Configuration not complete please check your .ci files or add required parameters to the command line." << std::endl;
          cmdline_parser.show_help(std::cout);
          return 1;
        } else {
          cic::HTTP::curl_client http_client;

          cic::servers::jenkins server(http_client, runtime_config);

          return server.summary();
        }
      }

      return 0;
    }

    config read_configuration(std::list<boost::filesystem::path> config_files) {
      config runtime_config;

      for (auto c : config_files) {
        runtime_config = runtime_config.merge(config::load(c));
      }

      return runtime_config;
    }


    void find_config_files_on_path(boost::filesystem::path path, std::list<boost::filesystem::path> &result) {
      if (boost::filesystem::is_regular(path)) {
        path = path.parent_path();
      }

      do {
        if (boost::filesystem::exists(path / ".ci")) {
          result.push_front(path / ".ci");
        }
        path = path.parent_path();
      } while (path.has_parent_path());
    }
  };
}

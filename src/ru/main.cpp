#include <csignal>
#include <functional>

#include "ru/dataframes_manager.hpp"
#include "ru/ru_args.hpp"

#include "common/json.hpp"
#include "common/log.hpp"

#include "common/transport/endpoint.hpp"
#include "common/transport/stream_recv_client.hpp"

int main(int argc, char* argv[])
{
  // parse command line options (it doesn't raise an exception)
  ru::ru_args const args = ru::parse_ru_args(argc, argv);

  try {
    // set mask before going in multi-thread
    sigset_t set;
    sigfillset(&set);                             // mask all signals
    pthread_sigmask(SIG_SETMASK, &set, nullptr);  // set mask

    // initialize the log component
    common::log::init();

    // add log to stdout console
    common::log::add_console();

    // parse configuration file and get json object
    common::json::object const config =
        common::json::read_json(args.config_file);

    // print json object
    // TODO: try to understand why "LOG_DEBUG<< config;" doesn't compile
    std::ostringstream oss;
    oss << config;
    LOG_DEBUG << "configuration dump:\n" << oss.str();

    // create engine
    common::engine::engine engine(1);

    // get FEE (Front-End Electronics) hostname and port for a given readout
    // unit id
    auto const section = config.get_child("FEE").get_child(args.id);
    std::string const hostname = section.get<std::string>("HOST");
    std::string const port = section.get<std::string>("PORT");

    // create dataframes_manager
    ru::dataframes_manager df_m(engine);

    // create the FEE endpoint
    auto const fee_ep = common::transport::make_endpoint(hostname, port);

    common::transport::stream_recv_client receiver(
        engine,
        fee_ep,
        std::bind(&ru::dataframes_manager::dataframes_parser,
                  &df_m,
                  std::placeholders::_1,
                  std::placeholders::_2));

    // start timeout timer
    common::engine::timer_t stop_timer(engine.get());
    if (args.timeout.count()) {
      stop_timer.expires_from_now(args.timeout);
      stop_timer.async_wait([&](boost::system::error_code const& e) {
        LOG_DEBUG << "Timeout reached, raising signal.";
        // used kill instead of std::raise due to this:
        // https://stackoverflow.com/questions/44520841/child-thread-raising-a-signal-it-is-ignoring
        kill(getpid(), SIGINT);
      });
    }

    // wait for any kind of signal and then stop execution
    int sig_caught;
    sigwait(&set, &sig_caught);
    LOG_INFO << "Received signal \"" << strsignal(sig_caught) << "\"";
    engine.stop();

  } catch (std::runtime_error& e) {
    LOG_FATAL << e.what();
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

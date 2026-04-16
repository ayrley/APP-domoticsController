#include "signalHandlers.h"

#include <cerrno>
#include <csignal>

#include <unistd.h>

namespace {

int installSignalHandler(int signalNumber, void (*handler)(int))
{
    struct sigaction action = {};
    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    if (sigaction(signalNumber, &action, nullptr) != 0) {
        return -errno;
    }

    return 0;
}

} // namespace

SignalHandlers &SignalHandlers::instance()
{
    static SignalHandlers handlers;
    return handlers;
}

int SignalHandlers::installTerminationHandlers()
{
    int ret = installSignalHandler(SIGTERM, &SignalHandlers::terminationHandler);
    if (ret != 0) {
        return ret;
    }

    ret = installSignalHandler(SIGINT, &SignalHandlers::terminationHandler);
    if (ret != 0) {
        return ret;
    }

    return 0;
}

void SignalHandlers::terminationHandler(int signalNumber)
{
    static constexpr char kShutdownMsg[] = "--- Application is shutting down (signal) ---\n";
    (void)::write(STDERR_FILENO, kShutdownMsg, sizeof(kShutdownMsg) - 1);

    // Preserve default termination behavior after emitting a final shutdown log.
    std::signal(signalNumber, SIG_DFL);
    (void)::kill(getpid(), signalNumber);
}

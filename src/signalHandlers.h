#ifndef SIGNAL_HANDLERS_H
#define SIGNAL_HANDLERS_H

class SignalHandlers {
public:
    static SignalHandlers &instance();

    int installTerminationHandlers();

    SignalHandlers(const SignalHandlers &) = delete;
    SignalHandlers &operator=(const SignalHandlers &) = delete;

private:
    SignalHandlers() = default;

    static void terminationHandler(int signalNumber);
};

#endif

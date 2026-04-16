#include "osdpBus.h"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <exception>
#include <thread>

#include "debug.h"
#include "serialBus.h"

extern std::vector<SerialBus *> *g_serialBusses;

std::mutex OsdpBus::s_registryMutex;
std::vector<std::shared_ptr<OsdpBus>> OsdpBus::s_busses;

OsdpBus::OsdpBus(const std::string &busName,
				 const std::string &busLocation,
				 int baudRate)
	: m_busName(busName),
	  m_busLocation(busLocation),
	  m_baudRate(baudRate),
	  m_setupDirty(true),
	  m_running(false)
{
	m_channel.data = this;
	m_channel.send = [](void *data, uint8_t *buf, int len) {
		return static_cast<OsdpBus *>(data)->send(buf, len);
	};
	m_channel.recv = [](void *data, uint8_t *buf, int len) {
		return static_cast<OsdpBus *>(data)->recv(buf, len);
	};
	m_channel.flush = nullptr;
	m_channel.close = nullptr;

	this->logger_init(("osdp::bus:" + m_busName).c_str(), OSDP_LOG_DEBUG, nullptr);
}

OsdpBus::~OsdpBus()
{
	stopWorker();
}

bool OsdpBus::resolveBusConfig(const std::string &identifier,
							   std::string &busName,
							   std::string &busLocation,
							   int &baudRate)
{
	busName = identifier;
	busLocation = identifier;

	if (g_serialBusses == nullptr) {
		return true;
	}

	for (SerialBus *configuredBus : *g_serialBusses) {
		if (configuredBus == nullptr) {
			continue;
		}

		if (configuredBus->getType() != SERIAL_BUS_OSDP) {
			continue;
		}

		if (configuredBus->getName() != identifier &&
			configuredBus->getLocation() != identifier) {
			continue;
		}

		busName = configuredBus->getName();
		busLocation = configuredBus->getLocation();
		if (configuredBus->getBaudRate() > 0) {
			baudRate = configuredBus->getBaudRate();
		}
		return true;
	}

	return true;
}

std::shared_ptr<OsdpBus> OsdpBus::findExistingLocked(const std::string &location)
{
	auto it = std::find_if(s_busses.begin(), s_busses.end(),
						   [&location](const std::shared_ptr<OsdpBus> &bus) {
							   return bus != nullptr && bus->m_busLocation == location;
						   });

	if (it == s_busses.end()) {
		return nullptr;
	}

	return *it;
}

std::shared_ptr<OsdpBus> OsdpBus::getOrCreate(const std::string &busIdentifier,
											  int requestedBaudRate)
{
	std::string resolvedName;
	std::string resolvedLocation;
	int resolvedBaudRate = requestedBaudRate > 0 ? requestedBaudRate : 115200;

	if (!resolveBusConfig(busIdentifier, resolvedName, resolvedLocation, resolvedBaudRate)) {
		return nullptr;
	}

	std::lock_guard<std::mutex> lock(s_registryMutex);
	std::shared_ptr<OsdpBus> existing = findExistingLocked(resolvedLocation);
	if (existing != nullptr) {
		if (existing->m_baudRate != resolvedBaudRate) {
			ERR("OSDP bus '" << resolvedLocation << "' already exists at "
							  << existing->m_baudRate << " baud; requested "
							  << resolvedBaudRate << ". Keeping existing bus.");
		}

		return existing;
	}

	std::shared_ptr<OsdpBus> created =
		std::shared_ptr<OsdpBus>(new OsdpBus(resolvedName, resolvedLocation, resolvedBaudRate));

	s_busses.push_back(created);
	return created;
}

void OsdpBus::startWorkerLocked()
{
	if (m_running.load()) {
		return;
	}

	m_running = true;
	m_worker = std::thread(&OsdpBus::runWorker, this);
}

void OsdpBus::stopWorker()
{
	m_running = false;
	if (m_worker.joinable()) {
		m_worker.join();
	}
}

int OsdpBus::registerReader(int address,
							const std::string &name,
							int baudRate,
							int ledDuration,
							const BadgeReadCallback &onBadgeRead,
							const ErrorCallback &onError)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	for (size_t i = 0; i < m_readers.size(); ++i) {
		if (m_readers[i].info.address != address) {
			continue;
		}

		m_readers[i].name = name;
		m_readers[i].info.name = m_readers[i].name.c_str();
		m_readers[i].info.baud_rate = baudRate > 0 ? baudRate : m_baudRate;
		m_readers[i].ledDuration = ledDuration;
		m_readers[i].pendingBuzzerDurationMs = 0;
		m_readers[i].onBadgeRead = onBadgeRead;
		m_readers[i].onError = onError;
		m_setupDirty = true;
		startWorkerLocked();
		return static_cast<int>(i);
	}

	ReaderSlot newReader = {};
	newReader.name = name;
	newReader.info = {};
	newReader.info.name = newReader.name.c_str();
	newReader.info.baud_rate = baudRate > 0 ? baudRate : m_baudRate;
	newReader.info.address = address;
	newReader.info.flags = 0;
	newReader.info.cap = nullptr;
	newReader.info.scbk = nullptr;
	newReader.ledDuration = ledDuration;
	newReader.pendingLedColor = OSDP_LED_COLOR_NONE;
	newReader.pendingBuzzerDurationMs = 0;
	newReader.onBadgeRead = onBadgeRead;
	newReader.onError = onError;

	m_readers.push_back(std::move(newReader));
	m_setupDirty = true;
	startWorkerLocked();
	return static_cast<int>(m_readers.size() - 1);
}

void OsdpBus::reportErrorForReader(size_t index, const std::string &message)
{
	ErrorCallback callback;

	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (index >= m_readers.size()) {
			return;
		}
		callback = m_readers[index].onError;
	}

	if (callback) {
		callback(message);
	}
}

void OsdpBus::reportErrorForAll(const std::string &message)
{
	std::vector<ErrorCallback> callbacks;

	{
		std::lock_guard<std::mutex> lock(m_mutex);
		callbacks.reserve(m_readers.size());
		for (const ReaderSlot &reader : m_readers) {
			callbacks.push_back(reader.onError);
		}
	}

	for (const ErrorCallback &callback : callbacks) {
		if (callback) {
			callback(message);
		}
	}
}

bool OsdpBus::decodeCardReadToBadge(const osdp_event_cardread &cardRead,
									uint64_t &badge)
{
	badge = 0;

	if (cardRead.format == OSDP_CARD_FMT_ASCII) {
		if (cardRead.length <= 0 || cardRead.length > OSDP_EVENT_CARDREAD_MAX_DATALEN) {
			return false;
		}

		for (int i = 0; i < cardRead.length; ++i) {
			const unsigned char ch = cardRead.data[i];
			if (!std::isdigit(ch)) {
				return false;
			}

			badge = (badge * 10u) + static_cast<uint64_t>(ch - '0');
		}

		return true;
	}

	if (cardRead.format != OSDP_CARD_FMT_RAW_UNSPECIFIED &&
		cardRead.format != OSDP_CARD_FMT_RAW_WIEGAND) {
		return false;
	}

	if (cardRead.length <= 0 || cardRead.length > 64) {
		return false;
	}

	for (int i = 0; i < cardRead.length; ++i) {
		const int bitIndex = (cardRead.direction == 0)
								 ? i
								 : (cardRead.length - 1 - i);
		const int byteIndex = bitIndex / 8;
		const int bitInByte = 7 - (bitIndex % 8);
		const uint8_t bit = (cardRead.data[byteIndex] >> bitInByte) & 0x01;

		badge = (badge << 1u) | static_cast<uint64_t>(bit);
	}

	return true;
}

enum osdp_led_color_e OsdpBus::mapLedColor(ledColor color)
{
	switch (color) {
	case LED_RED:
		return OSDP_LED_COLOR_RED;
	case LED_GREEN:
		return OSDP_LED_COLOR_GREEN;
	case LED_YELLOW:
		return OSDP_LED_COLOR_AMBER;
	case LED_NONE:
	default:
		return OSDP_LED_COLOR_NONE;
	}
}

int OsdpBus::send(uint8_t *buf, int len)
{
	if (m_uart == nullptr) {
		return 0;
	}

	try {
		const ssize_t written = m_uart->writeNonBlocking(buf, static_cast<size_t>(len));
		return written > 0 ? static_cast<int>(written) : 0;
	} catch (const std::exception &e) {
		reportErrorForAll("OSDP send failed on bus '" + m_busLocation + "': " + e.what());
		return -EIO;
	}
}

int OsdpBus::recv(uint8_t *buf, int len)
{
	if (m_uart == nullptr) {
		return 0;
	}

	try {
		const ssize_t readCount = m_uart->readNonBlocking(buf, static_cast<size_t>(len));
		return readCount > 0 ? static_cast<int>(readCount) : 0;
	} catch (const std::exception &e) {
		reportErrorForAll("OSDP recv failed on bus '" + m_busLocation + "': " + e.what());
		return -EIO;
	}
}

int OsdpBus::event(int pd, struct osdp_event *event)
{
	if (event == nullptr || event->type != OSDP_EVENT_CARDREAD) {
		return 0;
	}

	if (pd < 0) {
		return -EINVAL;
	}

	BadgeReadCallback onBadgeRead;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (static_cast<size_t>(pd) >= m_readers.size()) {
			return -EINVAL;
		}
		onBadgeRead = m_readers[static_cast<size_t>(pd)].onBadgeRead;
	}

	uint64_t badge = 0;
	if (!decodeCardReadToBadge(event->cardread, badge)) {
		reportErrorForReader(static_cast<size_t>(pd),
							 "Unable to decode OSDP badge payload");
		return -EINVAL;
	}

	if (!onBadgeRead) {
		return 0;
	}

	try {
		ReaderDecision decision = onBadgeRead(badge);
		std::lock_guard<std::mutex> lock(m_mutex);
		if (static_cast<size_t>(pd) < m_readers.size()) {
			ReaderSlot &reader = m_readers[static_cast<size_t>(pd)];
			const enum osdp_led_color_e mappedColor = mapLedColor(decision.led);
			reader.pendingLedColor = mappedColor != OSDP_LED_COLOR_NONE
								 ? mappedColor
								 : (decision.granted ? OSDP_LED_COLOR_GREEN
													 : OSDP_LED_COLOR_RED);
			reader.pendingBuzzerDurationMs = decision.buzzer
									 ? (decision.buzzerDurationMs > 0
											? decision.buzzerDurationMs
											: reader.ledDuration)
									 : 0;
		}
	} catch (const std::exception &e) {
		reportErrorForReader(static_cast<size_t>(pd),
							 "OSDP badge callback failed: " + std::string(e.what()));
		return -EINVAL;
	}

	return 0;
}

void OsdpBus::runWorker()
{
	try {
		m_uart = std::make_unique<funcmod::Uart>(m_busLocation, m_baudRate);
	} catch (const std::exception &e) {
		reportErrorForAll("Unable to open OSDP bus '" + m_busLocation + "': " + e.what());
		m_running = false;
		return;
	}

	while (m_running.load()) {
		bool applySetup = false;
		std::vector<osdp_pd_info_t> readerInfos;

		{
			std::lock_guard<std::mutex> lock(m_mutex);

			if (m_setupDirty) {
				readerInfos.reserve(m_readers.size());
				for (ReaderSlot &reader : m_readers) {
					reader.info.name = reader.name.c_str();
					readerInfos.push_back(reader.info);
				}
				m_setupDirty = false;
				applySetup = true;
			}

			for (size_t i = 0; i < m_readers.size(); ++i) {
				if (m_readers[i].pendingLedColor == OSDP_LED_COLOR_NONE &&
					m_readers[i].pendingBuzzerDurationMs <= 0) {
					continue;
				}

				if (m_readers[i].pendingLedColor != OSDP_LED_COLOR_NONE) {
					struct osdp_cmd ledCmd = {};
					ledCmd.id = OSDP_CMD_LED;
					ledCmd.led.reader = 0;
					ledCmd.led.led_number = 0;

					ledCmd.led.temporary.control_code = 0x02;
					ledCmd.led.temporary.on_count = 10;
					ledCmd.led.temporary.off_count = 0;
					ledCmd.led.temporary.on_color = m_readers[i].pendingLedColor;
					ledCmd.led.temporary.off_color = OSDP_LED_COLOR_NONE;
					ledCmd.led.temporary.timer_count =
						m_readers[i].ledDuration > 0 ? m_readers[i].ledDuration / 100 : 0;

					ledCmd.led.permanent.control_code = 0x01;
					ledCmd.led.permanent.on_count = 1;
					ledCmd.led.permanent.off_count = 0;
					ledCmd.led.permanent.on_color = OSDP_LED_COLOR_NONE;
					ledCmd.led.permanent.off_color = OSDP_LED_COLOR_NONE;

					this->send_command(static_cast<int>(i), &ledCmd);
					m_readers[i].pendingLedColor = OSDP_LED_COLOR_NONE;
				}

				if (m_readers[i].pendingBuzzerDurationMs > 0) {
					const int durationCount = std::max(1,
						std::min(255, m_readers[i].pendingBuzzerDurationMs / 100));
					struct osdp_cmd buzzerCmd = {};
					buzzerCmd.id = OSDP_CMD_BUZZER;
					buzzerCmd.buzzer.reader = 0;
					buzzerCmd.buzzer.control_code = 2;
					buzzerCmd.buzzer.on_count = static_cast<uint8_t>(durationCount);
					buzzerCmd.buzzer.off_count = 0;
					buzzerCmd.buzzer.rep_count = 1;

					this->send_command(static_cast<int>(i), &buzzerCmd);
					m_readers[i].pendingBuzzerDurationMs = 0;
				}
			}
		}

		if (applySetup) {
			if (readerInfos.empty()) {
				this->setup(&m_channel, 0, nullptr);
			} else {
				this->setup(&m_channel,
							static_cast<int>(readerInfos.size()),
							readerInfos.data());
			}

			this->set_event_callback([](void *data, int pd, struct osdp_event *event) {
				return static_cast<OsdpBus *>(data)->event(pd, event);
			},
			this);
		}

		this->refresh();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

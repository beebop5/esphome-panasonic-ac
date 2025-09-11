#include "esphome/components/climate/climate.h"
#include "esphome/components/climate/climate_mode.h"
#include "panasonic_ac_base.h"

namespace esphome {
namespace panasonic_ac {
namespace WLAN {

static const uint8_t HEADER = 0x5A;  // The header of the protocol, every packet starts with this

static const int INIT_TIMEOUT = 10000;       // Time to wait before initializing after boot
static const int INIT_END_TIMEOUT = 20000;   // Time to wait for last handshake packet
static const int FIRST_POLL_TIMEOUT = 650;   // Time to wait before requesting the first poll
static const int POLL_INTERVAL = 30000;      // The interval at which to poll the AC
static const int RESPONSE_TIMEOUT = 600;     // The timeout after which we expect a response to our last command
static const int INIT_FAIL_TIMEOUT = 60000;  // The timeout after which the initialization is considered failed

/*
 * Handshake commands
 */
static const uint8_t CMD_HANDSHAKE_1[]{0x00, 0x06, 0x00, 0x00};
static const uint8_t CMD_HANDSHAKE_2[]{0x00, 0x09, 0x00, 0x00};
static const uint8_t CMD_HANDSHAKE_3[]{0x00, 0x0C, 0x00, 0x00};
static const uint8_t CMD_HANDSHAKE_4[]{0x00, 0x10, 0x00, 0x01, 0x20};
static const uint8_t CMD_HANDSHAKE_5[]{0x00, 0x11, 0x00, 0x02, 0x00, 0x01};
static const uint8_t CMD_HANDSHAKE_6[]{0x00, 0x12, 0x00, 0x04, 0x01, 0x10, 0x11, 0x12};
static const uint8_t CMD_HANDSHAKE_7[]{0x00, 0x41, 0x00, 0x00};
static const uint8_t CMD_HANDSHAKE_8[]{0x01, 0x4C, 0x00, 0x00};
static const uint8_t CMD_HANDSHAKE_9[]{0x10, 0x00, 0x00, 0x00};
static const uint8_t CMD_HANDSHAKE_10[]{0x10, 0x01, 0x00, 0x05, 0x01, 0x30, 0x01, 0x00, 0x01};
static const uint8_t CMD_HANDSHAKE_11[]{0x00, 0x18, 0x00, 0x00};
static const uint8_t CMD_HANDSHAKE_12[]{0x01, 0x00, 0x00, 0x01, 0x10};
static const uint8_t CMD_HANDSHAKE_13[]{0x10, 0x08, 0x00, 0x09, 0x01, 0x01, 0x30, 0x01, 0x01, 0x02, 0x42, 0x01, 0x42};
static const uint8_t CMD_HANDSHAKE_14[]{0x01, 0x89, 0x00, 0x07, 0x00, 0xB8, 0xB7, 0xF1, 0x9B, 0x4F, 0xA6};
static const uint8_t CMD_HANDSHAKE_15[]{0x00, 0xA0, 0x00, 0x13, 0x00, 0x08, 0x30, 0x32, 0x2E, 0x30, 0x33, 0x2E,
                                        0x30, 0x30, 0x08, 0x30, 0x31, 0x30, 0x31, 0x30, 0x31, 0x30, 0x33};
static const uint8_t CMD_HANDSHAKE_16[]{0x01, 0x00, 0x00, 0x01, 0x11};

/*
 * Ping and poll commands
 */
static const uint8_t CMD_PING[]{0x01, 0x81, 0x00, 0x03, 0x00, 0x11, 0x12};
static const uint8_t CMD_POLL[]{0x10, 0x09, 0x00, 0x38, 0x01, 0x01, 0x30, 0x01, 0x11, 0x00, 0x80, 0x00,
                                0x00, 0xB0, 0x00, 0x02, 0x31, 0x00, 0x00, 0xA0, 0x00, 0x00, 0xA1, 0x00,
                                0x00, 0xA5, 0x00, 0x00, 0xA4, 0x00, 0x00, 0xB2, 0x00, 0x02, 0x35, 0x00,
                                0x02, 0x33, 0x00, 0x02, 0x34, 0x00, 0x02, 0x32, 0x00, 0x00, 0xBB, 0x00,
                                0x00, 0xBE, 0x00, 0x02, 0x20, 0x00, 0x02, 0x21, 0x00, 0x00, 0x86, 0x00};

/*
 * Report acknowledgment
 */
static const uint8_t CMD_REPORT_ACK[]{0x10, 0x8A, 0x00, 0x04, 0x00, 0x01, 0x30, 0x01};

enum class ACState {
  Initializing,     // Before first handshake packet is sent
  HandshakeDelay,   // Waiting for delay between handshake packets
  Handshake,        // During the initial handshake
  FirstPoll,        // After the handshake, before polling for the first time
  HandshakeEnding,  // After the first poll, waiting for the last handshake packet
  Ready,            // All done, ready to receive regular packets
  Failed            // Initialization failed
};

class PanasonicACWLAN : public PanasonicAC {
 public:
  void control(const climate::ClimateCall &call) override;

  void on_horizontal_swing_change(const std::string &swing) override;
  void on_vertical_swing_change(const std::string &swing) override;
  void on_nanoex_change(bool nanoex) override;

  void setup() override;
  void loop() override;

 protected:
  ACState state_ = ACState::Initializing;  // Stores the internal state of the AC, used during initialization

  uint8_t transmit_packet_count_ = 0;  // Counter used in packet (2nd byte) when we are sending packets
  uint8_t receive_packet_count_ = 0;   // Counter used in packet (2nd byte) when AC is sending us packets

  const uint8_t *last_command_;  // Stores a pointer to the last command we executed
  size_t last_command_length_;   // Stores the length of the last command we executed

  uint8_t set_queue_[16][2];     // Queue to store the key/value for the set commands
  uint8_t set_queue_index_ = 0;  // Stores the index of the next key/value set

  uint32_t handshake_delay_start_ = 0;  // Time when handshake delay started

  void handle_init_packets();
  void handle_handshake_packet();

  void handle_poll();
  bool verify_packet();
  void handle_packet();

  void send_set_command();
  void send_command(const uint8_t *command, size_t commandLength, CommandType type = CommandType::Normal);
  void send_packet(std::vector<uint8_t> packet, CommandType type = CommandType::Normal);

  climate::ClimateMode determine_mode(uint8_t mode);
  std::string determine_fan_speed(uint8_t speed);
  std::string determine_preset(uint8_t preset);
  std::string determine_swing_vertical(uint8_t swing);
  std::string determine_swing_horizontal(uint8_t swing);
  climate::ClimateSwingMode determine_swing(uint8_t swing);
  bool determine_nanoex(uint8_t nanoex);

  void handle_resend();

  void set_value(uint8_t key, uint8_t value);
};

}  // namespace WLAN
}  // namespace panasonic_ac
}  // namespace esphome

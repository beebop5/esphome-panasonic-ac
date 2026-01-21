#pragma once

#include "esphome/components/climate/climate.h"
#include "esphome/components/climate/climate_mode.h"
#include "panasonic_ac_base.h"
#include "panasonic_ac_cnt_commands.h"

namespace esphome {
namespace panasonic_ac {

// CNT protocol (CZ-TACG1 via CN-CNT port)
namespace cnt {
static constexpr uint8_t CTRL_HEADER = 0xF0;
static constexpr uint8_t POLL_HEADER = 0x70;
static constexpr int POLL_INTERVAL = 5000;
static constexpr int CMD_INTERVAL = 250;
}  // namespace cnt

enum class CNTState {
  Initializing,
  Ready,
};

class PanasonicACCNT : public PanasonicACBase {
 public:
  void control(const climate::ClimateCall &call) override;

  void on_horizontal_swing_change(const std::string &swing) override;
  void on_vertical_swing_change(const std::string &swing) override;
  void on_nanoex_change(bool nanoex) override;
  void on_powerful_change(bool powerful) override;
  void on_quiet_change(bool quiet) override;

  void setup() override;
  void loop() override;

 protected:
  CNTState state_ = CNTState::Initializing;

  std::vector<uint8_t> data_ = std::vector<uint8_t>(10);  // last-known 10-byte CNT payload
  std::vector<uint8_t> cmd_;                               // pending 10-byte command payload

  void handle_poll_();
  void handle_cmd_();

  void set_data_(bool publish);

  void send_command_(std::vector<uint8_t> payload, CommandType type, uint8_t header);
  void send_packet_(const std::vector<uint8_t> &packet, CommandType type);

  bool verify_packet_();
  void handle_packet_();
};

}  // namespace panasonic_ac
}  // namespace esphome


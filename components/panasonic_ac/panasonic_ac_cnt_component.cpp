#include "panasonic_ac_cnt_component.h"

#include "esphome/core/log.h"

#include <cstring>

namespace esphome {
namespace panasonic_ac {

static const char *const TAG = "panasonic_ac.cz_tacg1";

static climate::ClimateMode determine_mode_cnt(uint8_t mode) {
  uint8_t nib1 = (mode >> 4) & 0x0F;  // Left nib for mode
  uint8_t nib2 = (mode >> 0) & 0x0F;  // Right nib for power state

  if (nib2 == 0x00)
    return climate::CLIMATE_MODE_OFF;

  switch (nib1) {
    case 0x00:  // Auto
      return climate::CLIMATE_MODE_HEAT_COOL;
    case 0x03:  // Cool
      return climate::CLIMATE_MODE_COOL;
    case 0x04:  // Heat
      return climate::CLIMATE_MODE_HEAT;
    case 0x02:  // Dry
      return climate::CLIMATE_MODE_DRY;
    case 0x06:  // Fan only
      return climate::CLIMATE_MODE_FAN_ONLY;
    default:
      ESP_LOGW(TAG, "Received unknown climate mode");
      return climate::CLIMATE_MODE_OFF;
  }
}

static const char *determine_fan_speed_cnt(uint8_t speed) {
  switch (speed) {
    case 0xA0:  // Auto
      return "Automatic";
    case 0x30:
      return "1";
    case 0x40:
      return "2";
    case 0x50:
      return "3";
    case 0x60:
      return "4";
    case 0x70:
      return "5";
    default:
      ESP_LOGW(TAG, "Received unknown fan speed");
      return "Unknown";
  }
}

static std::string determine_vertical_swing_cnt(uint8_t swing) {
  uint8_t nib = (swing >> 4) & 0x0F;
  switch (nib) {
    case 0x0E:
      return "swing";
    case 0x0F:
      return "auto";
    case 0x01:
      return "up";
    case 0x02:
      return "up_center";
    case 0x03:
      return "center";
    case 0x04:
      return "down_center";
    case 0x05:
      return "down";
    case 0x00:
      return "unsupported";
    default:
      ESP_LOGW(TAG, "Received unknown vertical swing mode: 0x%02X", nib);
      return "Unknown";
  }
}

static std::string determine_horizontal_swing_cnt(uint8_t swing) {
  uint8_t nib = (swing >> 0) & 0x0F;
  switch (nib) {
    case 0x0D:
      return "auto";
    case 0x09:
      return "left";
    case 0x0A:
      return "left_center";
    case 0x06:
      return "center";
    case 0x0B:
      return "right_center";
    case 0x0C:
      return "right";
    case 0x00:
      return "unsupported";
    default:
      ESP_LOGW(TAG, "Received unknown horizontal swing mode");
      return "Unknown";
  }
}

static const char *determine_preset_cnt(uint8_t preset) {
  uint8_t nib = (preset >> 0) & 0x0F;  // right nib
  switch (nib) {
    case 0x02:
      return "Powerful";
    case 0x04:
      return "Quiet";
    case 0x00:
      return "Normal";
    default:
      ESP_LOGW(TAG, "Received unknown preset");
      return "Normal";
  }
}

static bool determine_nanoex_cnt(uint8_t preset) {
  uint8_t nib = (preset >> 4) & 0x04;
  if (nib == 0x04)
    return true;
  if (nib == 0x00)
    return false;
  ESP_LOGW(TAG, "Received unknown nanoex value");
  return false;
}

void PanasonicACCNT::setup() {
  PanasonicACBase::setup();
  ESP_LOGD(TAG, "Using CZ-TACG1 protocol via CN-CNT");
}

void PanasonicACCNT::loop() {
  PanasonicACBase::read_data();

  if (millis() - this->last_read_ > READ_TIMEOUT && !this->rx_buffer_.empty()) {
    log_packet(this->rx_buffer_);

    if (!verify_packet_()) {
      this->rx_buffer_.clear();
      return;
    }

    this->waiting_for_response_ = false;
    this->last_packet_received_ = millis();

    handle_packet_();
    this->rx_buffer_.clear();
  }

  handle_cmd_();
  handle_poll_();
}

void PanasonicACCNT::control(const climate::ClimateCall &call) {
  if (this->state_ != CNTState::Ready)
    return;

  if (this->cmd_.empty()) {
    ESP_LOGV(TAG, "Copying data to cmd");
    this->cmd_ = this->data_;
  }

  if (call.get_mode().has_value()) {
    ESP_LOGV(TAG, "Requested mode change");

    switch (*call.get_mode()) {
      case climate::CLIMATE_MODE_COOL:
        this->cmd_[0] = 0x34;
        break;
      case climate::CLIMATE_MODE_HEAT:
        this->cmd_[0] = 0x44;
        break;
      case climate::CLIMATE_MODE_DRY:
        this->cmd_[0] = 0x24;
        break;
      case climate::CLIMATE_MODE_HEAT_COOL:
        this->cmd_[0] = 0x04;
        break;
      case climate::CLIMATE_MODE_FAN_ONLY:
        this->cmd_[0] = 0x64;
        break;
      case climate::CLIMATE_MODE_OFF:
        this->cmd_[0] = this->cmd_[0] & 0xF0;
        break;
      default:
        ESP_LOGV(TAG, "Unsupported mode requested");
        break;
    }
  }

  if (call.get_target_temperature().has_value()) {
    ESP_LOGV(TAG, "Requested target temp change to %.2f", *call.get_target_temperature());
    this->cmd_[1] = (*call.get_target_temperature()) / TEMPERATURE_STEP;
  }

  if (call.has_custom_fan_mode()) {
    ESP_LOGV(TAG, "Requested fan mode change");
    const char *fan_mode = call.get_custom_fan_mode().c_str();

    if (strcmp(fan_mode, "Automatic") == 0)
      this->cmd_[3] = 0xA0;
    else if (strcmp(fan_mode, "1") == 0)
      this->cmd_[3] = 0x30;
    else if (strcmp(fan_mode, "2") == 0)
      this->cmd_[3] = 0x40;
    else if (strcmp(fan_mode, "3") == 0)
      this->cmd_[3] = 0x50;
    else if (strcmp(fan_mode, "4") == 0)
      this->cmd_[3] = 0x60;
    else if (strcmp(fan_mode, "5") == 0)
      this->cmd_[3] = 0x70;
    else
      ESP_LOGV(TAG, "Unsupported fan mode requested");
  }

  if (call.get_swing_mode().has_value()) {
    ESP_LOGV(TAG, "Requested swing mode change");

    switch (*call.get_swing_mode()) {
      case climate::CLIMATE_SWING_BOTH:
        this->cmd_[4] = 0xFD;
        break;
      case climate::CLIMATE_SWING_OFF:
        this->cmd_[4] = 0x36;
        break;
      case climate::CLIMATE_SWING_VERTICAL:
        this->cmd_[4] = 0xF6;
        break;
      case climate::CLIMATE_SWING_HORIZONTAL:
        this->cmd_[4] = 0x3D;
        break;
      default:
        ESP_LOGV(TAG, "Unsupported swing mode requested");
        break;
    }
  }

  if (call.has_custom_preset()) {
    ESP_LOGV(TAG, "Requested preset change");
    const char *preset = call.get_custom_preset().c_str();

    if (strcmp(preset, "Normal") == 0)
      this->cmd_[5] = (this->cmd_[5] & 0xF0);
    else if (strcmp(preset, "Powerful") == 0)
      this->cmd_[5] = (this->cmd_[5] & 0xF0) + 0x02;
    else if (strcmp(preset, "Quiet") == 0)
      this->cmd_[5] = (this->cmd_[5] & 0xF0) + 0x04;
    else
      ESP_LOGV(TAG, "Unsupported preset requested");
  }
}

void PanasonicACCNT::set_data_(bool publish) {
  this->mode = determine_mode_cnt(this->data_[0]);
#ifdef USE_TEXT_SENSOR
  update_fan_mode_status(determine_fan_speed_cnt(this->data_[3]));
#endif

  std::string vertical = determine_vertical_swing_cnt(this->data_[4]);
  std::string horizontal = determine_horizontal_swing_cnt(this->data_[4]);

  const char *preset = determine_preset_cnt(this->data_[5]);
  bool nanoex = determine_nanoex_cnt(this->data_[5]);

  update_target_temperature((uint8_t) this->data_[1]);

  if (publish) {
    // Current temperature (if available) may be at rx_buffer_[18] or rx_buffer_[21] per DomiStyle
    if (this->rx_buffer_.size() > 22) {
      if (this->rx_buffer_[18] != 0x80)
        update_current_temperature((int8_t) this->rx_buffer_[18]);
      else if (this->rx_buffer_[21] != 0x80)
        update_current_temperature((int8_t) this->rx_buffer_[21]);
    }

    if (this->outside_temperature_sensor_ != nullptr && this->rx_buffer_.size() > 23) {
      if (this->rx_buffer_[19] != 0x80)
        update_outside_temperature((int8_t) this->rx_buffer_[19]);
      else if (this->rx_buffer_[22] != 0x80)
        update_outside_temperature((int8_t) this->rx_buffer_[22]);
    }
  }

  // Swing mode derivation
  if (vertical == "auto" && horizontal == "auto")
    this->swing_mode = climate::CLIMATE_SWING_BOTH;
  else if (vertical == "auto")
    this->swing_mode = climate::CLIMATE_SWING_VERTICAL;
  else if (horizontal == "auto")
    this->swing_mode = climate::CLIMATE_SWING_HORIZONTAL;
  else
    this->swing_mode = climate::CLIMATE_SWING_OFF;

  update_swing_vertical(vertical);
  update_swing_horizontal(horizontal);

#ifdef USE_TEXT_SENSOR
  update_preset_status(preset);
#endif
  update_nanoex(nanoex);

  // Keep powerful/quiet switches in sync with preset
  update_powerful(std::string(preset) == "Powerful");
  update_quiet(std::string(preset) == "Quiet");
}

void PanasonicACCNT::send_command_(std::vector<uint8_t> payload, CommandType type, uint8_t header) {
  uint8_t length = payload.size();
  payload.insert(payload.begin(), header);
  payload.insert(payload.begin() + 1, length);

  uint8_t checksum = 0;
  for (uint8_t b : payload)
    checksum -= b;
  payload.push_back(checksum);

  send_packet_(payload, type);
}

void PanasonicACCNT::send_packet_(const std::vector<uint8_t> &packet, CommandType type) {
  this->last_packet_sent_ = millis();
  if (type != CommandType::Response)
    this->waiting_for_response_ = true;
  write_array(packet);
  log_packet(packet, true);
}

bool PanasonicACCNT::verify_packet_() {
  if (this->rx_buffer_.size() < 12) {
    ESP_LOGW(TAG, "Dropping invalid packet (length)");
    return false;
  }

  if (this->rx_buffer_[0] != cnt::CTRL_HEADER && this->rx_buffer_[0] != cnt::POLL_HEADER) {
    ESP_LOGW(TAG, "Dropping invalid packet (header)");
    return false;
  }

  if (this->rx_buffer_[1] != this->rx_buffer_.size() - 3) {
    ESP_LOGD(TAG, "Dropping invalid packet (length mismatch)");
    return false;
  }

  uint8_t checksum = 0;
  for (uint8_t b : this->rx_buffer_)
    checksum += b;
  if (checksum != 0) {
    ESP_LOGD(TAG, "Dropping invalid packet (checksum)");
    return false;
  }

  return true;
}

void PanasonicACCNT::handle_packet_() {
  if (this->rx_buffer_[0] == cnt::POLL_HEADER) {
    // payload starts at byte 2, length 10
    if (this->rx_buffer_.size() >= 2 + 10) {
      this->data_ = std::vector<uint8_t>(this->rx_buffer_.begin() + 2, this->rx_buffer_.begin() + 12);
      set_data_(true);
      publish_state();
      if (this->state_ != CNTState::Ready)
        this->state_ = CNTState::Ready;
    }
  } else {
    ESP_LOGD(TAG, "Received unknown packet");
  }
}

void PanasonicACCNT::handle_poll_() {
  if (millis() - this->last_packet_sent_ > cnt::POLL_INTERVAL) {
    ESP_LOGV(TAG, "Polling AC");
    send_command_(cnt::CMD_POLL, CommandType::Normal, cnt::POLL_HEADER);
  }
}

void PanasonicACCNT::handle_cmd_() {
  if (!this->cmd_.empty() && millis() - this->last_packet_sent_ > cnt::CMD_INTERVAL) {
    ESP_LOGV(TAG, "Sending Command");
    send_command_(this->cmd_, CommandType::Normal, cnt::CTRL_HEADER);
    this->cmd_.clear();
  }
}

void PanasonicACCNT::on_vertical_swing_change(const std::string &swing) {
  if (this->state_ != CNTState::Ready)
    return;

  if (this->cmd_.empty())
    this->cmd_ = this->data_;

  if (swing == "down")
    this->cmd_[4] = (this->cmd_[4] & 0x0F) + 0x50;
  else if (swing == "down_center")
    this->cmd_[4] = (this->cmd_[4] & 0x0F) + 0x40;
  else if (swing == "center")
    this->cmd_[4] = (this->cmd_[4] & 0x0F) + 0x30;
  else if (swing == "up_center")
    this->cmd_[4] = (this->cmd_[4] & 0x0F) + 0x20;
  else if (swing == "up")
    this->cmd_[4] = (this->cmd_[4] & 0x0F) + 0x10;
  else if (swing == "swing")
    this->cmd_[4] = (this->cmd_[4] & 0x0F) + 0xE0;
  else if (swing == "auto")
    this->cmd_[4] = (this->cmd_[4] & 0x0F) + 0xF0;
  else
    ESP_LOGW(TAG, "Unsupported vertical swing position received");
}

void PanasonicACCNT::on_horizontal_swing_change(const std::string &swing) {
  if (this->state_ != CNTState::Ready)
    return;

  if (this->cmd_.empty())
    this->cmd_ = this->data_;

  if (swing == "left")
    this->cmd_[4] = (this->cmd_[4] & 0xF0) + 0x09;
  else if (swing == "left_center")
    this->cmd_[4] = (this->cmd_[4] & 0xF0) + 0x0A;
  else if (swing == "center")
    this->cmd_[4] = (this->cmd_[4] & 0xF0) + 0x06;
  else if (swing == "right_center")
    this->cmd_[4] = (this->cmd_[4] & 0xF0) + 0x0B;
  else if (swing == "right")
    this->cmd_[4] = (this->cmd_[4] & 0xF0) + 0x0C;
  else if (swing == "auto")
    this->cmd_[4] = (this->cmd_[4] & 0xF0) + 0x0D;
  else
    ESP_LOGW(TAG, "Unsupported horizontal swing position received");
}

void PanasonicACCNT::on_nanoex_change(bool state) {
  if (this->state_ != CNTState::Ready)
    return;

  if (this->cmd_.empty())
    this->cmd_ = this->data_;

  if (state) {
    ESP_LOGV(TAG, "Turning nanoex on");
    this->cmd_[5] = (this->cmd_[5] & 0x0F) + 0x40;
  } else {
    ESP_LOGV(TAG, "Turning nanoex off");
    this->cmd_[5] = (this->cmd_[5] & 0x0F);
  }
}

void PanasonicACCNT::on_powerful_change(bool state) {
  if (this->state_ != CNTState::Ready)
    return;
  if (this->cmd_.empty())
    this->cmd_ = this->data_;

  if (state) {
    ESP_LOGD(TAG, "Turning powerful mode on");
    this->cmd_[5] = (this->cmd_[5] & 0xF0) + 0x02;
    if (this->quiet_state_)
      update_quiet(false);
  } else {
    ESP_LOGD(TAG, "Turning powerful mode off");
    this->cmd_[5] = (this->cmd_[5] & 0xF0);
  }
}

void PanasonicACCNT::on_quiet_change(bool state) {
  if (this->state_ != CNTState::Ready)
    return;
  if (this->cmd_.empty())
    this->cmd_ = this->data_;

  if (state) {
    ESP_LOGD(TAG, "Turning quiet mode on");
    this->cmd_[5] = (this->cmd_[5] & 0xF0) + 0x04;
    if (this->powerful_state_)
      update_powerful(false);
  } else {
    ESP_LOGD(TAG, "Turning quiet mode off");
    this->cmd_[5] = (this->cmd_[5] & 0xF0);
  }
}

}  // namespace panasonic_ac
}  // namespace esphome


#pragma once

#include <vector>
#include <cstdint>

namespace esphome {
namespace panasonic_ac {
namespace cnt {

// CNT poll payload (10 bytes). Header/length/checksum are added by the CNT component when sending.
static const std::vector<uint8_t> CMD_POLL = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

}  // namespace cnt
}  // namespace panasonic_ac
}  // namespace esphome


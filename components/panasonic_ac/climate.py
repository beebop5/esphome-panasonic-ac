from esphome.const import (
    CONF_ID,
    CONF_DISABLED_BY_DEFAULT,
    CONF_RESTORE_MODE,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
)
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, climate, sensor, select, switch, text_sensor
from esphome.core import ID

AUTO_LOAD = ["switch", "sensor", "select", "text_sensor"]
DEPENDENCIES = ["uart"]

panasonic_ac_ns = cg.esphome_ns.namespace("panasonic_ac")
PanasonicAC = panasonic_ac_ns.class_(
    "PanasonicAC", cg.Component, uart.UARTDevice, climate.Climate
)
PanasonicACCNT = panasonic_ac_ns.class_(
    "PanasonicACCNT", cg.Component, uart.UARTDevice, climate.Climate
)

CONF_TYPE = "type"
TYPE_WLAN = "wlan"
TYPE_CNT = "cnt"

CONF_OUTSIDE_TEMPERATURE = "outside_temperature"
CONF_HORIZONTAL_SWING_SELECT_ID = "horizontal_swing_select_id"
CONF_VERTICAL_SWING_SELECT_ID = "vertical_swing_select_id"
CONF_NANOEX_SWITCH_ID = "nanoex_switch_id"
CONF_POWERFUL_SWITCH_ID = "powerful_switch_id"
CONF_QUIET_SWITCH_ID = "quiet_switch_id"
CONF_FAN_MODE_STATUS = "fan_mode_status"
CONF_PRESET_STATUS = "preset_status"


COMMON_SCHEMA = {
    cv.Optional(CONF_OUTSIDE_TEMPERATURE): sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    # Support for connecting to external select/switch components
    cv.Optional(CONF_HORIZONTAL_SWING_SELECT_ID): cv.use_id(select.Select),
    cv.Optional(CONF_VERTICAL_SWING_SELECT_ID): cv.use_id(select.Select),
    cv.Optional(CONF_NANOEX_SWITCH_ID): cv.use_id(switch.Switch),
    cv.Optional(CONF_POWERFUL_SWITCH_ID): cv.use_id(switch.Switch),
    cv.Optional(CONF_QUIET_SWITCH_ID): cv.use_id(switch.Switch),
    # Optional status-only text sensors for reporting current fan mode and preset
    cv.Optional(CONF_FAN_MODE_STATUS): text_sensor.text_sensor_schema(),
    cv.Optional(CONF_PRESET_STATUS): text_sensor.text_sensor_schema(),
}

WLAN_SCHEMA = climate.climate_schema(PanasonicAC).extend(
    {cv.GenerateID(): cv.declare_id(PanasonicAC), **COMMON_SCHEMA}
).extend(uart.UART_DEVICE_SCHEMA)

CNT_SCHEMA = climate.climate_schema(PanasonicACCNT).extend(
    {cv.GenerateID(): cv.declare_id(PanasonicACCNT), **COMMON_SCHEMA}
).extend(uart.UART_DEVICE_SCHEMA)

CONFIG_SCHEMA = cv.typed_schema(
    {TYPE_WLAN: WLAN_SCHEMA, TYPE_CNT: CNT_SCHEMA},
    default_type=TYPE_WLAN,
    lower=True,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await climate.register_climate(var, config)
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    if CONF_OUTSIDE_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_OUTSIDE_TEMPERATURE])
        cg.add(var.set_outside_temperature_sensor(sens))

    # Connect to external select/switch components
    if CONF_HORIZONTAL_SWING_SELECT_ID in config:
        sel = await cg.get_variable(config[CONF_HORIZONTAL_SWING_SELECT_ID])
        cg.add(var.set_horizontal_swing_select(sel))

    if CONF_VERTICAL_SWING_SELECT_ID in config:
        sel = await cg.get_variable(config[CONF_VERTICAL_SWING_SELECT_ID])
        cg.add(var.set_vertical_swing_select(sel))

    if CONF_NANOEX_SWITCH_ID in config:
        sw = await cg.get_variable(config[CONF_NANOEX_SWITCH_ID])
        cg.add(var.set_nanoex_switch(sw))

    if CONF_POWERFUL_SWITCH_ID in config:
        sw = await cg.get_variable(config[CONF_POWERFUL_SWITCH_ID])
        cg.add(var.set_powerful_switch(sw))

    if CONF_QUIET_SWITCH_ID in config:
        sw = await cg.get_variable(config[CONF_QUIET_SWITCH_ID])
        cg.add(var.set_quiet_switch(sw))

    # Optional status reporting text sensors
    if CONF_FAN_MODE_STATUS in config:
        ts = await text_sensor.new_text_sensor(config[CONF_FAN_MODE_STATUS])
        cg.add(var.set_fan_mode_status_text_sensor(ts))

    if CONF_PRESET_STATUS in config:
        ts = await text_sensor.new_text_sensor(config[CONF_PRESET_STATUS])
        cg.add(var.set_preset_status_text_sensor(ts))
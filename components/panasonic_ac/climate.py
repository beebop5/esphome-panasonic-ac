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
from esphome.components import uart, climate, sensor, select, switch
from esphome.core import ID

AUTO_LOAD = ["switch", "sensor", "select"]
DEPENDENCIES = ["uart"]

panasonic_ac_ns = cg.esphome_ns.namespace("panasonic_ac")
PanasonicAC = panasonic_ac_ns.class_(
    "PanasonicAC", cg.Component, uart.UARTDevice, climate.Climate
)

CONF_HORIZONTAL_SWING_SELECT = "horizontal_swing_select"
CONF_VERTICAL_SWING_SELECT = "vertical_swing_select"
CONF_OUTSIDE_TEMPERATURE = "outside_temperature"
CONF_NANOEX_SWITCH = "nanoex_switch"
CONF_HORIZONTAL_SWING_SELECT_ID = "horizontal_swing_select_id"
CONF_VERTICAL_SWING_SELECT_ID = "vertical_swing_select_id"
CONF_NANOEX_SWITCH_ID = "nanoex_switch_id"

HORIZONTAL_SWING_OPTIONS = ["Left", "Centre Left", "Centre", "Centre Right", "Right"]
VERTICAL_SWING_OPTIONS = ["Up", "Mid Up", "Mid", "Mid Down", "Down"]

CONFIG_SCHEMA = climate.climate_schema(PanasonicAC).extend(
    {
        cv.GenerateID(): cv.declare_id(PanasonicAC),
        cv.Optional(CONF_HORIZONTAL_SWING_SELECT): select.select_schema(select.Select).extend({
            cv.GenerateID(): cv.declare_id(select.Select),
        }),
        cv.Optional(CONF_VERTICAL_SWING_SELECT): select.select_schema(select.Select).extend({
            cv.GenerateID(): cv.declare_id(select.Select),
        }),
        cv.Optional(CONF_OUTSIDE_TEMPERATURE): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_NANOEX_SWITCH): switch.switch_schema(switch.Switch).extend({
            cv.GenerateID(): cv.declare_id(switch.Switch),
        }),
        # Support for connecting to external select/switch components
        cv.Optional(CONF_HORIZONTAL_SWING_SELECT_ID): cv.use_id(select.Select),
        cv.Optional(CONF_VERTICAL_SWING_SELECT_ID): cv.use_id(select.Select),
        cv.Optional(CONF_NANOEX_SWITCH_ID): cv.use_id(switch.Switch),
    }
).extend(uart.UART_DEVICE_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await climate.register_climate(var, config)
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    if CONF_OUTSIDE_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_OUTSIDE_TEMPERATURE])
        cg.add(var.set_outside_temperature_sensor(sens))

    # Create template select components for swing control
    if CONF_HORIZONTAL_SWING_SELECT in config:
        sel_config = config[CONF_HORIZONTAL_SWING_SELECT]
        sel = await select.new_select(sel_config, options=HORIZONTAL_SWING_OPTIONS)
        await cg.register_parented(sel, config[CONF_ID])
        cg.add(var.set_horizontal_swing_select(sel))

    if CONF_VERTICAL_SWING_SELECT in config:
        sel_config = config[CONF_VERTICAL_SWING_SELECT]
        sel = await select.new_select(sel_config, options=VERTICAL_SWING_OPTIONS)
        await cg.register_parented(sel, config[CONF_ID])
        cg.add(var.set_vertical_swing_select(sel))

    # Create template switch component for nanoeX
    if CONF_NANOEX_SWITCH in config:
        sw_config = config[CONF_NANOEX_SWITCH]
        sw = await switch.new_switch(sw_config)
        await cg.register_parented(sw, config[CONF_ID])
        cg.add(var.set_nanoex_switch(sw))

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
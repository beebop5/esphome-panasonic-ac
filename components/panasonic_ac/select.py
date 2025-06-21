import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import select
from esphome.const import CONF_ID, CONF_NAME

from . import panasonic_ac_ns, PanasonicAC

CONF_TYPE = "type"
CONF_PANASONIC_AC_ID = "panasonic_ac_id"

TYPES = ["horizontal_swing", "vertical_swing"]

PanasonicACSelect = panasonic_ac_ns.class_("PanasonicACSelect", select.Select, cg.Component)

CONFIG_SCHEMA = select.SELECT_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(PanasonicACSelect),
    cv.Required(CONF_PANASONIC_AC_ID): cv.use_id(PanasonicAC),
    cv.Required(CONF_TYPE): cv.one_of(*TYPES, lower=True),
    cv.Optional(CONF_NAME): cv.string,
})

HORIZONTAL_SWING_OPTIONS = ["Auto", "Left", "Centre Left", "Centre", "Centre Right", "Right"]
VERTICAL_SWING_OPTIONS = ["Swing", "Auto", "Up", "Mid Up", "Mid", "Mid Down", "Down"]

async def to_code(config):
    parent = await cg.get_variable(config[CONF_PANASONIC_AC_ID])
    if config[CONF_TYPE] == "horizontal_swing":
        options = HORIZONTAL_SWING_OPTIONS
    else:
        options = VERTICAL_SWING_OPTIONS
    var = await select.new_select(config, options=options)
    await cg.register_component(var, config)
    await select.register_select(var, config)
    cg.add(parent.register_select(var, config[CONF_TYPE]))
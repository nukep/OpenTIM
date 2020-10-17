def getU8(addr):
    return getByte(addr) % 0x100

def getS8(addr):
    return getByte(addr)

def getU16(addr):
    return getShort(addr) % 0x10000

def getS16(addr):
    return getShort(addr)

def getU32(addr):
    return getInt(addr) % 0x100000000

def getAddress(offset):
    return currentProgram.getAddressFactory().getDefaultAddressSpace().getAddress(offset)

def getDataPtr(addr):
    return getAddress("1108:{:04X}".format(getU16(addr)))

part_names = [
    "BowlingBall",
    "BrickWall",
    "Incline",
    "TeeterTotter",
    "Balloon",
    "Conveyor",
    "MortTheMouseCage",
    "Pulley",
    "Belt",
    "Basketball",
    "Rope",
    "Cage",
    "PokeyTheCat",
    "JackInTheBox",
    "Gear",
    "BobTheFish",
    "Bellows",
    "Bucket",
    "Cannon",
    "Dynamite",
    "GunBullet",
    "LightSwitchOutlet",
    "DynamiteWithPlunger",
    "EyeHook",
    "Fan",
    "Flashlight",
    "Generator",
    "Gun",
    "Baseball",
    "Lightbulb",
    "MagnifyingGlass",
    "KellyTheMonkey",
    "JackOLantern",
    "HeartBalloon",
    "ChristmasTree",
    "BoxingGlove",
    "Rocket",
    "Scissors",
    "SolarPanels",
    "Trampoline",
    "Windmill",
    "Explosion",
    "MortTheMouse",
    "CannonBall",
    "TennisBall",
    "Candle",
    "PipeStraight",
    "PipeCurved",
    "WoodWall",
    "RopeSeveredEnd",
    "ElectricEngine",
    "Vacuum",
    "Cheese",
    "Nail",
    "MelSchlemming",
    "TitleTheEvenMore",
    "TitleIncredibleMachine",
    "TitleCredits",
    "MelsHouse",
    "SuperBall",
    "DirtWall",
    "ErnieTheAlligator",
    "Teapot",
    "Eightball",
    "PinballBumper",
    "LuckyClover"
]

part_names_snake = [
    "bowling_ball",
    "brick_wall",
    "incline",
    "teeter_totter",
    "balloon",
    "conveyor",
    "mort_the_mouse_cage",
    "pulley",
    "belt",
    "basketball",
    "rope",
    "cage",
    "pokey_the_cat",
    "jack_in_the_box",
    "gear",
    "bob_the_fish",
    "bellows",
    "bucket",
    "cannon",
    "dynamite",
    "gun_bullet",
    "light_switch_outlet",
    "dynamite_with_plunger",
    "eye_hook",
    "fan",
    "flashlight",
    "generator",
    "gun",
    "baseball",
    "lightbulb",
    "magnifying_glass",
    "kelly_the_monkey",
    "jack_o_lantern",
    "heart_balloon",
    "christmas_tree",
    "boxing_glove",
    "rocket",
    "scissors",
    "solar_panels",
    "trampoline",
    "windmill",
    "explosion",
    "mort_the_mouse",
    "cannon_ball",
    "tennis_ball",
    "candle",
    "pipe_straight",
    "pipe_curved",
    "wood_wall",
    "rope_severed_end",
    "electric_engine",
    "vacuum",
    "cheese",
    "nail",
    "mel_schlemming",
    "title_the_even_more",
    "title_incredible_machine",
    "title_credits",
    "mels_house",
    "super_ball",
    "dirt_wall",
    "ernie_the_alligator",
    "teapot",
    "eightball",
    "pinball_bumper",
    "lucky_clover"
]

part_states = [
    None,
    None,
    None,
    3,
    7,
    None,
    None,
    None,
    None,
    None,
    None,
    None,
    10,
    19,
    None,
    23,
    3,
    None,
    12,
    6,
    3,
    8,
    3,
    None,
    4,
    2,
    16,
    7,
    None,
    4,
    None,
    13,
    None,
    None,
    None,
    10,
    10,
    2,
    4,
    5,
    4,
    6,
    2,
    None,
    None,
    6,
    None,
    None,
    None,
    None,
    None,
    9,
    None,
    None,
    31,
    None,
    None,
    None,
    9,
    None,
    None,
    15,
    12,
    None,
    None,
    None
]

DEFAULT_RESET  = 0x10900df7
DEFAULT_RUN    = 0x10900de2
DEFAULT_BOUNCE = 0x10900dca
DEFAULT_FLIP   = 0x10900e0c
DEFAULT_RESIZE = 0x10900e21
DEFAULT_ROPE   = 0x10900e36

print("Def lookup:")
print("match part_type {")
for part_type in range(66):
    print("    {:<22} => &{}::def,".format(part_names[part_type], part_names_snake[part_type]))
print("}")
print("")

print("Part implementations:")
for part_type in range(66):
    states = part_states[part_type]

    data30_addr = getAddress(0x10e80000 + part_type*0x10)
    data31_addr = getAddress(0x10f00000 + part_type*0x3a)

    flags1 = getU16(data30_addr.add(0x00))
    flags3 = getU16(data30_addr.add(0x02))
    size_something2_w = getS16(data30_addr.add(0x04))
    size_something2_h = getS16(data30_addr.add(0x06))
    size_w = getS16(data30_addr.add(0x08))
    size_h = getS16(data30_addr.add(0x0A))
    create_fn = getU32(data30_addr.add(0x0C))

    density = getU16(data31_addr.add(0x00))
    mass = getU16(data31_addr.add(0x02))
    bounciness = getS16(data31_addr.add(0x04))
    friction = getS16(data31_addr.add(0x06))
    max_resize_x = getS16(data31_addr.add(0x0C))
    max_resize_y = getS16(data31_addr.add(0x0E))
    min_resize_x = getS16(data31_addr.add(0x10))
    min_resize_y = getS16(data31_addr.add(0x12))
    render_images_addr = getU16(data31_addr.add(0x16))
    render_pos_offsets_addr = getU16(data31_addr.add(0x18))
    explicit_sizes_addr = getU16(data31_addr.add(0x1A))
    goobers_0 = getU8(data31_addr.add(0x1C))
    goobers_1 = getU8(data31_addr.add(0x1D))
    borders = getU16(data31_addr.add(0x1E))
    part_order = getU16(data31_addr.add(0x20))

    bounce_fn = getU32(data31_addr.add(0x22))
    run_fn = getU32(data31_addr.add(0x26))
    reset_fn = getU32(data31_addr.add(0x2A))
    flip_fn = getU32(data31_addr.add(0x2E))
    resize_fn = getU32(data31_addr.add(0x32))
    rope_fn = getU32(data31_addr.add(0x36))

    resize_x = "None"
    resize_y = "None"

    if max_resize_x != 0:
        resize_x = "Some(({}, {}))".format(min_resize_x, max_resize_x)
    
    if max_resize_y != 0:
        resize_y = "Some(({}, {}))".format(min_resize_y, max_resize_y)

    def fmtfar(ptr):
        return "{:04x}:{:04x}".format(ptr >> 16, ptr & 0xFFFF)

    print("mod {} {{".format(part_names_snake[part_type]))
    print("    use super::prelude::*;")
    print("    pub const def: PartDef = PartDef {")
    print("        borders: {},".format(borders))
    print("")
    print("        density: {},".format(density))
    print("        mass: {},".format(mass))
    print("        bounciness: {},".format(bounciness))
    print("        friction: {},".format(friction))
    print("")
    print("        flags1: 0x{:04X},".format(flags1))
    print("        flags3: 0x{:04X},".format(flags3))
    print("        size_something2: ({}, {}),".format(size_something2_w, size_something2_h))
    print("        size:            ({}, {}),".format(size_w, size_h))
    print("        resize_x: {},".format(resize_x))
    print("        resize_y: {},".format(resize_y))
    print("")
    if render_images_addr == 0:
        print("        render_images: None,")
    else:
        print("        render_images: Some(&[")
        baseaddr = getAddress(0x11080000 | render_images_addr)
        for state in range(states):
            addr = getDataPtr(baseaddr.add(state*2))

            # List of (goober, index, x, y)
            images = []
            while True:
                goober = getU8(addr.add(0x02))

                for i in range(4):
                    idx = getU8(addr.add(0x03+i))
                    x   = getS8(addr.add(0x07+i*2+0))
                    y   = getS8(addr.add(0x07+i*2+1))
                    
                    if idx == 255: break

                    images.append((goober, idx, x, y))

                if getU16(addr) != 0:
                    addr = getDataPtr(addr)
                else:
                    break
            
            s = "&[".format(state)
            first = True
            for (goober, index, x, y) in images:
                if not first: s += ", "

                s += "({}, {}, {}, {})".format(goober, index, x, y)
                first = False
            
            s += "],"

            print("            " + s)
        print("        ]),")
    if render_pos_offsets_addr == 0:
        print("        render_pos_offsets: None,")
    else:
        print("        render_pos_offsets: Some(&[")
        baseaddr = getAddress(0x11080000 | render_pos_offsets_addr)
        s = ""
        for state in range(states):
            x = getS8(baseaddr.add(state*2+0))
            y = getS8(baseaddr.add(state*2+1))
            s += "({},{}), ".format(x, y)
        print("            " + s)
        print("        ]),")
    if explicit_sizes_addr == 0:
        print("        explicit_sizes: None,")
    else:
        print("        explicit_sizes: Some(&[")
        baseaddr = getAddress(0x11080000 | explicit_sizes_addr)
        s = ""
        for state in range(states):
            x = getS16(baseaddr.add(state*4+0))
            y = getS16(baseaddr.add(state*4+2))
            s += "({},{}), ".format(x, y)
        print("            " + s)
        print("        ]),")

    print("        goobers: ({}, {}),".format(goobers_0, goobers_1))
    print("")
    print("        create_fn: create,")
    def out_fn(default_addr, addr, name):
        if addr == default_addr:
            print("        {:<10} None,".format(name+"_fn:"))
        else:
            print("        {:<10} Some({}),".format(name+"_fn:", name))
    out_fn(DEFAULT_RESET, reset_fn, "reset")
    out_fn(DEFAULT_RUN, run_fn, "run")
    out_fn(DEFAULT_BOUNCE, bounce_fn, "bounce")
    out_fn(DEFAULT_FLIP, flip_fn, "flip")
    out_fn(DEFAULT_RESIZE, resize_fn, "resize")
    out_fn(DEFAULT_ROPE, rope_fn, "rope")
    print("    };")
    print("")
    print("    // TIMWIN: " + fmtfar(create_fn))
    print("    fn create(part: &mut Part) {")
    print("        unimplemented();")
    print("    }")
    def out_fn2(default_addr, addr, name, sig, args):
        if addr != default_addr:
            print("")
            print("    // TIMWIN: " + fmtfar(addr))
            print("    fn {}{} {{".format(name, sig))
            print("        // {}_c!({}_{}, {});".format(name, part_names_snake[part_type], name, args))
            print("        unimplemented();")
            print("    }")
    
    out_fn2(DEFAULT_RESET,  reset_fn,  "reset",  "(part: &mut Part)", "part")
    out_fn2(DEFAULT_RUN,    run_fn,    "run",    "(part: &mut Part)", "part")
    out_fn2(DEFAULT_BOUNCE, bounce_fn, "bounce", "(part: &mut Part) -> bool", "part")
    out_fn2(DEFAULT_FLIP,   flip_fn,   "flip",   "(part: &mut Part, _orientation: u16)", "part, _orientation")
    out_fn2(DEFAULT_RESIZE, resize_fn, "resize", "(part: &mut Part)", "part")
    out_fn2(DEFAULT_ROPE,   rope_fn,   "rope",   "(p1: &mut Part, p2: &mut Part, rope_slot: u8, flags: u16, p1_mass: i16, p1_force: i32) -> u8", "p1, p2, rope_slot, flags, p1_mass, p1_force")
    print("}")
    print("")

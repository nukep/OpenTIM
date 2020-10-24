// For now...
#![allow(unused_variables)]

use std::os::raw::{c_int, c_char};
use crate::tim_c::Part;

pub struct PartDef {
    // Fields henceforth correlate to Segment 30 in TIMWIN

    pub flags1: u16,
    pub flags3: u16,
    pub size_something2: (i16, u16),
    pub size: (i16, u16),

    // Fields henceforth correlate to Segment 31 in TIMWIN

    pub density: u16,
    pub mass: u16,
    pub bounciness: i16,
    pub friction: i16,

    // calculated:
    // OpenTIM - removed
    // TIM would modify this data structure whenver the air pressure or gravity changed, to pre-calculate these properties.
    // To keep the implementation simple, we'll recalculate these each time.
    // pub acceleration: i16,
    // pub terminal_velocity: i16,

    pub resize_x: Option<(u16, u16)>,
    pub resize_y: Option<(u16, u16)>,

    // Number of elements is the number of states (state1).
    pub render_images: Option<&'static [&'static [(u8, u8, i8, i8)]]>,
    // Number of elements is the number of states (state1).
    pub render_pos_offsets: Option<&'static [(i8, i8)]>,
    // Number of elements is the number of states (state1).
    pub explicit_sizes: Option<&'static [(i16, i16)]>,
    pub goobers: (u8, u8),

    // OpenTIM - removed
    // TIM preallocated the border. OpenTIM allocates it on the fly.
    // pub borders: u16,

    pub create_fn: fn(&mut Part),
    pub bounce_fn: Option<fn(&mut Part) -> bool>,
    pub run_fn: Option<fn(&mut Part)>,
    pub reset_fn: Option<fn(&mut Part)>,
    pub flip_fn: Option<fn(&mut Part, u16)>,
    pub resize_fn: Option<fn(&mut Part)>,
    pub rope_fn: Option<fn(&mut Part, &mut Part, u8, u16, i16, i32) -> u8>,
}

#[macro_use]
mod prelude {
    pub use crate::part::PartType;
    pub use crate::parts::PartDef;
    pub use std::os::raw::{c_int, c_char};
    pub use crate::tim_c;
    pub use crate::tim_c::{Part, RopeData, BeltData};

    pub const F2_FLIP_HORZ: u16 = 0x0010;
    pub const F2_FLIP_VERT: u16 = 0x0020;
    pub const F2_DISAPPEARED: u16 = 0x2000;

    pub fn unimplemented() -> ! {
        panic!("Unimplemented");
    }

    // TIMWIN: 10a8:372e
    pub fn other_belt_part(part: &Part) -> *mut Part {
        if let Some(belt_data) = unsafe { part.belt_data.as_ref() } {
            if belt_data.part1 as *const Part == part as *const Part {
                belt_data.part2
            } else {
                belt_data.part1
            }
        } else {
            std::ptr::null_mut::<Part>()
        }
    }

    pub fn angle_to_signed(angle: u16) -> i16 {
        // Really we're converting an unsigned number to a signed number as per two's complement.
        if angle < 0x8000 {
            angle as i16
        } else {
            (-((0x10000 - angle as u32) as i32)) as i16
        }
    }

    #[test]
    fn test_angle_to_signed() {
        assert_eq!(angle_to_signed(0), 0);
        assert_eq!(angle_to_signed(100), 100);
        assert_eq!(angle_to_signed(0x7fff), 0x7fff);
        assert_eq!(angle_to_signed(0x8000), -0x8000);
        assert_eq!(angle_to_signed(0xfff0), -0x10);
        assert_eq!(angle_to_signed(0xffff), -1);
    }

    #[macro_export]
    macro_rules! bounce_c {
        ($c_fn: ident, $part: expr) => {
            extern { fn $c_fn(part: *mut Part) -> c_int; }
            unsafe { return $c_fn($part) != 0 }
        }
    }

    #[macro_export]
    macro_rules! run_c {
        ($c_fn: ident, $part: expr) => {
            extern { fn $c_fn(part: *mut Part); }
            unsafe { return $c_fn($part) }
        }
    }

    #[macro_export]
    macro_rules! reset_c {
        ($c_fn: ident, $part: expr) => {
            extern { fn $c_fn(part: *mut Part); }
            unsafe { return $c_fn($part) }
        }
    }

    #[macro_export]
    macro_rules! flip_c {
        ($c_fn: ident, $part: expr, $orientation: expr) => {
            extern { fn $c_fn(part: *mut Part, orientatio: c_int); }
            unsafe { return $c_fn($part, $orientation as c_int) }
        }
    }

    #[macro_export]
    macro_rules! resize_c {
        ($c_fn: ident, $part: expr) => {
            extern { fn $c_fn(part: *mut Part); }
            unsafe { return $c_fn($part) }
        }
    }

    #[macro_export]
    macro_rules! rope_c {
        ($c_fn: ident, $p1: expr, $p2: expr, $rope_slot: expr, $flags: expr, $p1_mass: expr, $p1_force: expr) => {
            extern { fn $c_fn(p1: *mut Part, p2: *mut Part, rope_slot: c_int, flags: u16, p1_mass: i16, p1_force: i32) -> c_int; }
            unsafe { return $c_fn($p1, $p2, $rope_slot as c_int, $flags, $p1_mass, $p1_force) as u8 }
        }
    }
}

mod bowling_ball {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 2832,
        mass: 200,
        bounciness: 128,
        friction: 16,

        flags1: 0x0800,
        flags3: 0x0008,
        size_something2: (32, 32),
        size:            (32, 32),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (3, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    None,
        bounce_fn: None,
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:023d
    fn create(_part: &mut Part) {
        // do nothing
    }

    // TIMWIN: 1048:051c
    fn reset(part: &mut Part) {
        part.set_border(&[(8, 0), (23, 0), (31, 8), (31, 23), (23, 31), (8, 31), (0, 23), (0, 8)]);
    }
}

mod wall_common {
    use super::prelude::*;

    // TIMWIN: 1078:0280
    pub fn create(part: &mut Part) {
        part.flags1 |= 0x0040;
        part.flags2 |= 0x0100 | 0x0080;
    }

    // TIMWIN: 10d0:0e7a
    pub fn reset(part: &mut Part) {
        let x = part.size.x as u8 - 1;
        let y = part.size.y as u8 - 1;
        part.set_border(&[(0, 0), (x, 0), (x, y), (0, y)]);
    }

    // TIMWIN: 10d0:0ed8
    pub fn resize(part: &mut Part) {
        match unsafe { tim_c::RESIZE_GOPHER } {
            0x8003 => part.size_something2.y = 16,
            0x8004 => part.size_something2.y = 16,
            0x8005 => part.size_something2.x = 16,
            0x8006 => part.size_something2.x = 16,
            _ => ()
        }

        let x = part.size.x as u8 - 1;
        let y = part.size.y as u8 - 1;
        let b = part.border_points_mut();
        b[1].x = x;
        b[2].x = x;
        b[2].y = y;
        b[3].y = y;
    }
}

mod brick_wall {
    use super::prelude::*;
    use super::wall_common::{create, reset, resize};
    pub const DEF: PartDef = PartDef {
        density: 4153,
        mass: 1000,
        bounciness: 1024,
        friction: 24,

        flags1: 0x4800,
        flags3: 0x0000,
        size_something2: (32, 16),
        size:            (32, 16),
        resize_x: Some((16, 240)),
        resize_y: Some((16, 240)),

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (4, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    None,
        bounce_fn: None,
        flip_fn:   None,
        resize_fn: Some(resize),
        rope_fn:   None,
    };
}

mod incline {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 1510,
        mass: 1000,
        bounciness: 1024,
        friction: 16,

        flags1: 0x4800,
        flags3: 0x0000,
        size_something2: (32, 32),
        size:            (32, 32),
        resize_x: Some((16, 64)),
        resize_y: None,

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (4, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    None,
        bounce_fn: None,
        flip_fn:   Some(flip),
        resize_fn: Some(resize),
        rope_fn:   None,
    };

    // TIMWIN: 1078:02cc
    fn create(part: &mut Part) {
        part.flags1 |= 0x0400 | 0x0200;
        part.flags2 |= 0x0080;
        part.state1 = 1;
        part.original_state1 = 1;
    }

    // TIMWIN: 1080:1388
    fn reset(part: &mut Part) {
        let borders = if part.flags2 & 0x0010 == 0 {
            // TIMWIN: 1108:0dc0
            &[
                &[(0, 0), (15, 15), (15, 31), (0, 16)],
                &[(0, 0), (31, 15), (31, 31), (0, 16)],
                &[(0, 0), (47, 15), (47, 31), (0, 16)],
                &[(0, 0), (63, 15), (63, 31), (0, 16)]
            ]
        } else {
            // TIMWIN: 1108:0de8
            &[
                &[(0, 15), (15, 0), (15, 16), (0, 31)],
                &[(0, 15), (31, 0), (31, 16), (0, 31)],
                &[(0, 15), (47, 0), (47, 16), (0, 31)],
                &[(0, 15), (63, 0), (63, 16), (0, 31)]
            ]
        };

        part.set_border(borders[part.state1 as usize]);
    }

    // TIMWIN: 1080:1427
    fn flip(part: &mut Part, orientation: u16) {
        part.flags2 ^= 0x0010;
        reset(part);
        unsafe {
            tim_c::stub_10a8_2b6d(part, 3);
            tim_c::stub_10a8_21cb(part, 2);
        }
    }

    // TIMWIN: 1080:13ec
    fn resize(part: &mut Part) {
        part.size = part.size_something2;
        part.state1 = part.size.x/16 - 1;
        part.original_state1 = part.state1;
        reset(part);
    }
}

mod teeter_totter {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 1888,
        mass: 1000,
        bounciness: 1024,
        friction: 16,

        flags1: 0x4800,
        flags3: 0x0000,
        size_something2: (80, 32),
        size:            (80, 32),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: Some(&[
            (0,0), (0,12), (0,0), 
        ]),
        explicit_sizes: None,
        goobers: (4, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: Some(bounce),
        flip_fn:   Some(flip),
        resize_fn: None,
        rope_fn:   Some(rope),
    };

    // TIMWIN: 1078:0323
    fn create(part: &mut Part) {
        part.flags1 |= 0x0400;
        part.flags2 |= 0x000c;
    }

    // TIMWIN: 10d0:0114
    fn reset(part: &mut Part) {
        // TIMWIN: 1108:2e9e
        let rope_locs_0 = &[(5,27), (4,2), (6,3)];
        // TIMWIN: 1108:2eaa
        let rope_locs_1 = &[(73,3), (75,2), (74,27)];

        // TIMWIN: 1108:2eb6 
        let borders = &[
            &[(0, 32), (79, 3),  (79, 8),  (44, 21), (44, 34), (36, 34), (36, 24), (0, 36)],
            &[(0, 17), (79, 17), (79, 21), (44, 21), (44, 34), (36, 34), (36, 21), (0, 21)],
            &[(0, 3),  (79, 32), (79, 36), (44, 24), (44, 34), (36, 34), (36, 21), (0, 8)]
        ];

        let i = part.state1 as usize;
        part.rope_loc[0].x = rope_locs_0[i].0;
        part.rope_loc[0].y = rope_locs_0[i].1;
        part.rope_loc[1].x = rope_locs_1[i].0;
        part.rope_loc[1].y = rope_locs_1[i].1;
        part.set_border(borders[i]);
    }

    // export to C
    #[no_mangle]
    pub extern fn teeter_totter_reset(part: &mut Part) {
        reset(part);
    }

    // TIMWIN: 10d0:0240
    fn run(part: &mut Part) {
        run_c!(teeter_totter_run, part);
    }

    // TIMWIN: 10d0:0000
    fn bounce(part: &mut Part) -> bool {
        bounce_c!(teeter_totter_bounce, part);
    }

    // TIMWIN: 10d0:01db
    fn flip(part: &mut Part, _orientation: u16) {
        if part.state1 == 0 {
            part.state1 = 2;
        } else {
            part.state1 = 0;
        }

        part.original_state1 = part.state1;
        reset(part);
        unsafe {
            tim_c::part_set_size_and_pos_render(part);
            tim_c::stub_10a8_280a(part, 3);
            tim_c::stub_10a8_2b6d(part, 3);
            tim_c::stub_10a8_21cb(part, 2);
        }
    }

    // TIMWIN: 10d0:04fe
    fn rope(p1: &mut Part, p2: &mut Part, rope_slot: u8, flags: u16, p1_mass: i16, p1_force: i32) -> u8 {
        rope_c!(teeter_totter_rope, p1, p2, rope_slot, flags, p1_mass, p1_force);
    }
}

mod balloon {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 9,
        mass: 1,
        bounciness: 64,
        friction: 32,

        flags1: 0x0800,
        flags3: 0x0008,
        size_something2: (32, 48),
        size:            (32, 48),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: Some(&[
            (0,0), (-15,-9), (-20,-5), (-28,8), (-30,25), (-26,41), (-26,56), 
        ]),
        explicit_sizes: None,
        goobers: (3, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: Some(bounce),
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   Some(rope),
    };

    // TIMWIN: 1078:036f
    fn create(part: &mut Part) {
        part.flags1 |= 0x0020;
        part.flags2 |= 0x0004;
        part.rope_loc[0].x = 16;
        part.rope_loc[0].y = 47;
    }

    // TIMWIN: 1048:067e
    fn reset(part: &mut Part) {
        // TIMWIN: 1108:0b18
        part.set_border(&[(0x00, 0x0a), (0x0c, 0x00), (0x16, 0x00), (0x1f, 0x0a), (0x1f, 0x1c), (0x13, 0x2b), (0x0b, 0x2b), (0x00, 0x1d)]);
    }

    // TIMWIN: 1048:06f4
    fn run(part: &mut Part) {
        run_c!(balloon_run, part);
    }

    // TIMWIN: 1048:06c6
    fn bounce(part: &mut Part) -> bool {
        if part.part_type == PartType::GunBullet.to_u16() {
            part.bounce_part_mut().unwrap().state2 = 1;
        }

        true
    }

    // TIMWIN: 1048:082b
    fn rope(p1: &mut Part, p2: &mut Part, rope_slot: u8, flags: u16, p1_mass: i16, p1_force: i32) -> u8 {
        rope_c!(balloon_rope, p1, p2, rope_slot, flags, p1_mass, p1_force);
    }
}

mod conveyor {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 3776,
        mass: 1000,
        bounciness: 1024,
        friction: 16,

        flags1: 0x4800,
        flags3: 0x0000,
        size_something2: (96, 16),
        size:            (96, 16),
        resize_x: Some((32, 96)),
        resize_y: None,

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (5, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: Some(bounce),
        flip_fn:   None,
        resize_fn: Some(resize),
        rope_fn:   None,
    };

    // TIMWIN: 1078:03c2
    fn create(part: &mut Part) {
        part.flags2 |= 0x0080 | 0x0001;
        part.state1 = 28;
        part.original_state1 = 28;
        part.state2 = 0;
        part.original_state2 = 0;
        part.belt_loc.x = 59;
        part.belt_width = 14;
    }

    // TIMWIN: 1080:025c
    fn reset(part: &mut Part) {
        let x = part.size.x as u8;
        let y = part.size.y as u8;
        part.set_border(&[(0, 0), (x, 0), (x, y), (0, y)]);
    }

    // TIMWIN: 1080:0352
    fn run(part: &mut Part) {
        if part.state2 != 0 {
            if let Some(other) = unsafe { other_belt_part(part).as_ref() } {
                if other.part_type == PartType::Gear.to_u16() {
                    if other.state1_prev1 == other.state1_prev2 {
                        part.state2 = 0;
                    }
                }
            }
        }
        if part.state2 != 0 {
            // Global variable changes here in original game.
            // Not sure if it's needed
            // CONVEYOR_STATE = 2

            if part.state1 == part.state1_prev1 {
                tim_c::play_sound(1);
            }
            if part.state2 > 0 {
                if (part.state1 + 1) % 7 == 0 {
                    part.state1 -= 6;
                } else {
                    part.state1 += 1;
                }
            } else if part.state2 < 0 {
                if part.state1 % 7 == 0 {
                    part.state1 += 6;
                } else {
                    part.state1 -= 1;
                }
            }
        }
    }

    // TIMWIN: 1080:02ac
    fn bounce(part: &mut Part) -> bool {
        let conveyor = unsafe { part.bounce_part.as_ref().unwrap() };
        let border_index = part.bounce_border_index;
        let bounce_angle = angle_to_signed(part.bounce_angle);

        if border_index == 0 || (bounce_angle >= -2048 && bounce_angle <= 2048)  {
            if part.part_type == PartType::Basketball.to_u16() {
                println!("BOING: {} {}", bounce_angle, border_index);
            }
            if conveyor.state2 < 0 {
                // This is a quirk.
                // When subtracting from the velocities, the comparisons to cap the velocities are wrong.
                part.vel_hi_precision.x -= 4096;
                if part.vel_hi_precision.x < 4096 {
                    part.vel_hi_precision.x = -4096;
                }
            } else if conveyor.state2 > 0 {
                part.vel_hi_precision.x = std::cmp::min(4096, part.vel_hi_precision.x + 4096);
            }
        } else if border_index == 2 {
            if conveyor.state2 < 0 {
                part.vel_hi_precision.x = std::cmp::min(4096, part.vel_hi_precision.x + 4096);
            } else if conveyor.state2 > 0 {
                // This is a quirk.
                // When subtracting from the velocities, the comparisons to cap the velocities are wrong.
                part.vel_hi_precision.x -= 4096;
                if part.vel_hi_precision.x < 4096 {
                    part.vel_hi_precision.x = -4096;
                }
            }
        }

        true
    }

    // TIMWIN: 1080:0401
    fn resize(part: &mut Part) {
        part.size = part.size_something2;
        let x = part.size.x as u8;
        let points = part.border_points_mut();
        points[1].x = x;
        points[2].x = x;
        let st = (part.size.x - 32) / 16;
        part.state1 = st*7;
        part.original_state1 = st*7;
        part.belt_loc.x = match st {
            0 => 9,
            1 => 23,
            2 => 38,
            3 => 44,
            4 => 59,
            _ => unreachable!()
        };
    }
}

mod mort_the_mouse_cage {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 3776,
        mass: 1000,
        bounciness: 1024,
        friction: 16,

        flags1: 0x4800,
        flags3: 0x0000,
        size_something2: (48, 32),
        size:            (48, 32),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (4, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: Some(bounce),
        flip_fn:   Some(flip),
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:0426
    fn create(part: &mut Part) {
        part.flags1 |= 0x0400;
        part.flags2 |= 0x0800 | 0x0001;
        part.belt_loc.x = 30;
        part.belt_loc.y = 4;
        part.belt_width = 12;
    }

    // TIMWIN: 1088:0df6
    fn reset(part: &mut Part) {
        let x = part.size.x as u8;
        let y = part.size.y as u8;
        part.set_border(&[(0, 0), (x, 0), (x, y), (0, y)]);
    }

    // TIMWIN: 1088:0e6f
    fn run(part: &mut Part) {
        if part.state2 == 0 {
            unsafe { tim_c::search_for_interactions(part, 0x1000, -16, 16, 0, 0); }

            for p in unsafe { part.interactions_iter() } {
                let p = unsafe { p.as_ref().unwrap() };
                if p.part_type == PartType::PokeyTheCat.to_u16() {
                    make_it_go(part);
                }
            }
        }

        if let Some(other) = unsafe { other_belt_part(part).as_mut() } {
            if other.flags2 & 0x0800 == 0 {
                other.state2 = part.state2;
            }
        }

        if part.state2 != 0 {
            part.state1 ^= 1;
            part.extra1 -= 1;
            if part.extra1 == 0 {
                part.state2 = 0;
            }
        }
    }

    // TIMWIN: 1088:0e46
    fn bounce(part: &mut Part) -> bool {
        unsafe {
            make_it_go(part.bounce_part.as_mut().unwrap());
        }
        true
    }

    // TIMWIN: 1088:0efb
    fn flip(part: &mut Part, _orientation: u16) {
        part.flags2 ^= F2_FLIP_HORZ;
        if (part.flags2 & F2_FLIP_HORZ) == 0 {
            part.belt_loc.x = 30;
        } else {
            part.belt_loc.x = 3;
        }
        unsafe {
            tim_c::stub_10a8_280a(part, 3);
            tim_c::stub_10a8_2b6d(part, 3);
            tim_c::stub_10a8_21cb(part, 2);
        }
    }

    // TIMWIN: 1088:0f4b
    pub fn make_it_go(cage: &mut Part) {
        if cage.state2 == 0 {
            tim_c::play_sound(0x0d);
        }
        cage.state2 = if cage.flags2 & F2_FLIP_HORZ == 0 { 1 } else { -1 };
        cage.extra1 = 100;
    }

    #[no_mangle]
    fn mort_the_mouse_cage_start(part: &mut Part) {
        make_it_go(part);
    }
}

mod pulley {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 3776,
        mass: 1000,
        bounciness: 0,
        friction: 0,

        flags1: 0x4800,
        flags3: 0x0008,
        size_something2: (16, 16),
        size:            (16, 16),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (2, 255),

        create_fn: create,
        reset_fn:  None,
        run_fn:    None,
        bounce_fn: None,
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:0480
    fn create(part: &mut Part) {
        part.flags2 |= 0x0004;
        part.rope_loc[0].x = 0;
        part.rope_loc[0].y = 8;
        part.rope_loc[1].x = 15;
        part.rope_loc[1].y = 8;
        part.init_rope_data_primary();
    }
}

mod belt {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 3776,
        mass: 1000,
        bounciness: 0,
        friction: 0,

        flags1: 0x4800,
        flags3: 0x0000,
        size_something2: (0, 0),
        size:            (0, 0),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (1, 255),

        create_fn: create,
        reset_fn:  None,
        run_fn:    None,
        bounce_fn: None,
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:04d2
    fn create(part: &mut Part) {
        part.init_belt_data();
    }
}

mod basketball {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 1322,
        mass: 20,
        bounciness: 192,
        friction: 16,

        flags1: 0x0800,
        flags3: 0x0008,
        size_something2: (32, 32),
        size:            (32, 32),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (3, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    None,
        bounce_fn: None,
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:0511
    fn create(_part: &mut Part) {
        // do nothing
    }

    // TIMWIN: 1048:051c
    fn reset(part: &mut Part) {
        part.set_border(&[(8, 0), (23, 0), (31, 8), (31, 23), (23, 31), (8, 31), (0, 23), (0, 8)]);
    }
}

mod rope {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 1600,
        mass: 1000,
        bounciness: 0,
        friction: 0,

        flags1: 0x4800,
        flags3: 0x0000,
        size_something2: (0, 0),
        size:            (0, 0),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (1, 255),

        create_fn: create,
        reset_fn:  None,
        run_fn:    None,
        bounce_fn: None,
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:0554
    fn create(part: &mut Part) {
        part.init_rope_data_primary();
    }
}

mod cage {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 7552,
        mass: 150,
        bounciness: 32,
        friction: 64,

        flags1: 0x0800,
        flags3: 0x0008,
        size_something2: (48, 64),
        size:            (48, 64),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (2, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    None,
        bounce_fn: None,
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   Some(rope),
    };

    // TIMWIN: 1078:0592
    fn create(part: &mut Part) {
        part.flags1 |= 0x0020;
        part.flags2 |= 0x0004;
        part.rope_loc[0].x = 21;
        part.rope_loc[0].y = 2;
    }

    // TIMWIN: 1048:17b6
    fn reset(part: &mut Part) {
        part.set_border(&[(0,24), (19,0), (25,0), (45,24), (45,63), (43,63), (43,26), (34,16), (11,16), (2,26), (2,63), (0,63)]);
    }

    // TIMWIN: 1048:1854
    fn rope(p1: &mut Part, p2: &mut Part, rope_slot: u8, flags: u16, p1_mass: i16, p1_force: i32) -> u8 {
        if flags == 1 {
            p2.rope_mut(0).unwrap().rope_unknown += 1;
            return 0;
        } else if p1.part_type == PartType::TeeterTotter.to_u16() {
            if p1_force < p2.force {
                return 1;
            }
        } else if p1_force < p2.force*2 {
            return 1;
        }
        if flags == 2 {
            p2.pos.y -= 20;
            p2.state2 += 1;
            unsafe { tim_c::part_set_size_and_pos_render(p2); }
        }
        return 0;
    }
}

mod pokey_the_cat {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 2000,
        mass: 120,
        bounciness: 0,
        friction: 64,

        flags1: 0x0800,
        flags3: 0x0008,
        size_something2: (40, 41),
        size:            (40, 39),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: Some(&[
            (0,0), (-6,-16), (19,-1), (14,0), (10,0), (8,0), (6,-2), (-1,-2), (-5,-2), (-9,-3), 
        ]),
        explicit_sizes: None,
        goobers: (3, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: Some(bounce),
        flip_fn:   Some(flip),
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:05e5
    fn create(part: &mut Part) {
        part.flags1 |= 0x0400;
        part.flags2 |= 0x8000;
    }

    // TIMWIN: 1048:1434
    fn reset(part: &mut Part) {
        if part.flags2 & 0x0010 == 0 {
            // TIMWIN: 1108:0bf8
            part.set_border(&[(0, 7), (10, 0), (36, 26), (36, 37), (10, 40)]);
        } else {
            // TIMWIN: 1108:0c02
            part.set_border(&[(3, 26), (29, 0), (39, 10), (29, 40), (3, 37)]);
        }
    }

    // TIMWIN: 1048:14cf
    fn run(part: &mut Part) {
        run_c!(pokey_the_cat_run, part);
    }

    // TIMWIN: 1048:148a
    fn bounce(part: &mut Part) -> bool {
        let cat = part.bounce_part_mut().unwrap();
        if cat.state1 == 0 {
            cat.state1 = 1;
            cat.extra1 = 0;
            unsafe {
                tim_c::part_set_size_and_pos_render(cat);
            }
            tim_c::play_sound(0x07);
        }
        true
    }

    // TIMWIN: 1048:1771
    fn flip(part: &mut Part, _orientation: u16) {
        part.flags2 ^= 0x0010;
        reset(part);
        unsafe {
            tim_c::part_set_size_and_pos_render(part);
            tim_c::stub_10a8_2b6d(part, 3);
            tim_c::stub_10a8_21cb(part, 2);
        }
    }
}

mod jack_in_the_box {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 3776,
        mass: 1000,
        bounciness: 1024,
        friction: 16,

        flags1: 0x4800,
        flags3: 0x0000,
        size_something2: (32, 32),
        size:            (32, 32),
        resize_x: None,
        resize_y: None,

        render_images: Some(&[
            &[(3, 0, 0, 3), (3, 1, 0, 0), (3, 2, 9, 9)],
            &[(3, 0, 0, 3), (3, 1, 0, 0), (3, 3, 9, 9)],
            &[(3, 0, 0, 3), (3, 1, 0, 0), (3, 4, 9, 9)],
            &[(3, 0, 0, 3), (3, 1, 0, 0), (3, 5, 9, 9)],
            &[(3, 0, 0, 3), (3, 1, 0, 0), (3, 6, 9, 9)],
            &[(3, 0, 0, 3), (3, 1, 0, 0), (3, 7, 8, 9)],
            &[(3, 0, 0, 3), (3, 1, 0, 0), (3, 8, 9, 9)],
            &[(3, 0, 0, 3), (3, 1, 0, 0), (3, 9, 9, 8)],
            &[(3, 0, 0, 3), (3, 10, 0, -21), (3, 2, 9, 9)],
            &[(3, 0, 0, 3), (3, 11, 1, -34), (3, 2, 9, 9)],
            &[(3, 0, 0, 3), (3, 12, -6, -59), (3, 2, 9, 9)],
            &[(3, 0, 0, 3), (3, 13, -4, -25), (3, 2, 9, 9)],
            &[(3, 0, 0, 3), (3, 14, -6, -30), (3, 2, 9, 9)],
            &[(3, 0, 0, 3), (3, 15, -4, -25), (3, 2, 9, 9)],
            &[(3, 0, 0, 3), (3, 16, -4, -25), (3, 2, 9, 9)],
            &[(3, 0, 0, 3), (3, 17, -4, -25), (3, 2, 9, 9)],
            &[(3, 0, 0, 3), (3, 18, -4, -25), (3, 2, 9, 9)],
            &[(3, 0, 0, 3), (3, 19, -4, -25), (3, 2, 9, 9)],
            &[(3, 0, 0, 3), (3, 20, -4, -25), (3, 2, 9, 9)],
        ]),
        render_pos_offsets: Some(&[
            (0,0), (0,0), (0,0), (0,0), (0,0), (0,0), (0,0), (0,0), (0,-21), (0,-34), (-6,-60), (-4,-25), (-6,-30), (-4,-25), (-4,-25), (-4,-25), (-4,-25), (-4,-25), (-4,-25), 
        ]),
        explicit_sizes: Some(&[
            (32,32), (32,32), (32,32), (32,32), (32,32), (32,32), (32,32), (32,32), (35,53), (35,66), (39,91), (36,57), (38,62), (37,57), (41,57), (40,57), (36,57), (36,57), (36,57), 
        ]),
        goobers: (3, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: None,
        flip_fn:   Some(flip),
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:0632
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 1080:19b7
    fn reset(part: &mut Part) {
        // reset_c!(jack_in_the_box_reset, part);
        unimplemented();
    }

    // TIMWIN: 1080:1834
    fn run(part: &mut Part) {
        // run_c!(jack_in_the_box_run, part);
        unimplemented();
    }

    // TIMWIN: 1080:1a05
    fn flip(part: &mut Part, _orientation: u16) {
        // flip_c!(jack_in_the_box_flip, part, _orientation);
        unimplemented();
    }
}

mod gear {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 7552,
        mass: 1000,
        bounciness: 1024,
        friction: 48,

        flags1: 0x4800,
        flags3: 0x0000,
        size_something2: (32, 32),
        size:            (35, 35),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (4, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: Some(bounce),
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:068c
    fn create(part: &mut Part) {
        part.flags2 |= 0x0001;
        part.belt_loc.x = 13;
        part.belt_loc.y = 13;
        part.belt_width = 8;
    }

    // TIMWIN: 1078:177b
    fn reset(part: &mut Part) {
        // Same as bowling ball and basketball's border
        part.set_border(&[(8, 0), (23, 0), (31, 8), (31, 23), (23, 31), (8, 31), (0, 23), (0, 8)]);

        part.links_to[0] = std::ptr::null_mut::<Part>();
        part.links_to[1] = std::ptr::null_mut::<Part>();
        part.links_to_design[0] = std::ptr::null_mut::<Part>();
        part.links_to_design[1] = std::ptr::null_mut::<Part>();

        // Link this gear up with adjacent gears in the playfield
        for ptr in unsafe { tim_c::static_parts_iter_mut() } {
            if ptr == part { continue; }
            let curpart = unsafe { ptr.as_mut().unwrap() };
            if curpart.part_type != PartType::Gear.to_u16() { continue; }

            if part.original_pos_y == curpart.original_pos_y {
                if part.original_pos_x-curpart.original_pos_x == 32 {
                    part.links_to[0] = curpart;
                } else if part.original_pos_x-curpart.original_pos_x == -32 {
                    part.links_to[1] = curpart;
                }
            } else {
                if part.original_pos_x == curpart.original_pos_x {
                    // The gear uses link_to_design as the above/below links.
                    // This appears to be a hacky workaround from the original game.

                    if part.original_pos_y-curpart.original_pos_y == 32 {
                        part.links_to_design[0] = curpart;
                    } else if part.original_pos_y-curpart.original_pos_y == -32 {
                        part.links_to_design[1] = curpart;
                    }
                }
            }
        }
    }

    // TIMWIN: 1078:187e
    fn helper1(part: &mut Part, otherpart: *mut Part, n: u16, val: u16) -> u16 {
        if otherpart.is_null() { return val; }
        let otherpart = unsafe { otherpart.as_mut().unwrap() };

        let mut val = val;

        if otherpart.state2 == 0 {
            otherpart.state2 = if n == 1 {
                part.state2
            } else {
                -part.state2
            };
        } else if otherpart.state2 != part.state2 && n == 1 {
            val = 1;
        } else if otherpart.state2 == part.state2 && n == 2 {
            val = 1;
        }

        if otherpart.part_type == PartType::Gear.to_u16() && (otherpart.flags2 & 0x0040) == 0 {
            otherpart.flags2 |= 0x0040;

            let mut h = |p: &mut Part, linked: *mut Part, new_n: u16, val: u16| -> u16 {
                if linked.is_null() { return val; }
                let linked = unsafe { linked.as_mut().unwrap() };
                if (linked.flags2 & 0x0800) != 0 { return val; }
    
                helper1(p, linked, new_n, val)
            };

            val = h(otherpart, otherpart.links_to[0], 2, val);
            val = h(otherpart, otherpart.links_to[1], 2, val);
            val = h(otherpart, otherpart.links_to_design[0], 2, val);
            val = h(otherpart, otherpart.links_to_design[1], 2, val);
            val = h(otherpart, other_belt_part(otherpart), 1, val);
        }

        val
    }

    // TIMWIN: 1078:1950
    fn helper2(part: &mut Part, val: u16) {
        part.state1 += part.state2;
        if part.state1 == -1 {
            part.state1 = 3;
        } else if part.state1 == 4 {
            part.state1 = 0;
        }
        part.state2 = 0;

        let mut h = |linked: *mut Part| {
            if linked.is_null() { return; }
            let linked = unsafe { linked.as_mut().unwrap() };
            if linked.state2 == 0 { return; }
            if (linked.flags2 & 0x0800) != 0 { return; }

            if val != 0 {
                linked.state2 = 0;
            }
            if linked.part_type == PartType::Gear.to_u16() {
                helper2(linked, val);
            }
        };

        h(part.links_to[0]);
        h(part.links_to[1]);
        h(part.links_to_design[0]);
        h(part.links_to_design[1]);
        h(other_belt_part(part));
    }

    // TIMWIN: 1078:1810
    fn run(part: &mut Part) {
        if part.state2 != 0 {
            part.flags2 |= 0x0040;
            let mut val = 0;
            val = helper1(part, part.links_to[0], 2, val);
            val = helper1(part, part.links_to[1], 2, val);
            val = helper1(part, part.links_to_design[0], 2, val);
            val = helper1(part, part.links_to_design[1], 2, val);
            if val != 0 {
                part.state2 = 0;
            }
            helper2(part, val);
        }
    }

    // TIMWIN: 1078:1668
    fn bounce(part: &mut Part) -> bool {
        let gear = unsafe { part.bounce_part.as_mut().unwrap() };

        let mut state_delta = gear.state1 - gear.state1_prev1;

        if state_delta < -1 {
            state_delta = 1;
        } else if state_delta > 1 {
            state_delta = -1;
        }

        if state_delta != 0 {
            if part.part_type == PartType::Balloon.to_u16() {
                // Pop the balloon
                part.state2 = 1;
                return true;
            }

            let mode = if state_delta < 1 {
                (part.bounce_border_index + 4) % 8
            } else {
                part.bounce_border_index
            };

            let a: &[(u8, i16)] = match mode {
                0 => &[(1, 4096)],
                1 => &[(1, 2048), (2, 2048)],
                2 => &[(2, 4096)],
                3 => &[(3, 2048), (2, 2048)],
                4 => &[(3, 4096)],
                5 => &[(3, 2048), (4, 2048)],
                6 => &[(4, 4096)],
                7 => &[(1, 2048), (4, 2048)],
                _ => unreachable!()
            };

            for &(method, speed) in a {
                match method {
                    1 => part.vel_hi_precision.x = i16::min( speed, part.vel_hi_precision.x + speed),
                    2 => part.vel_hi_precision.y = i16::min( speed, part.vel_hi_precision.y + speed),

                    // This is a quirk.
                    // When subtracting from the velocities, the comparisons to cap the velocities are wrong.
                    3 => {
                        part.vel_hi_precision.x -= speed;
                        if part.vel_hi_precision.x < speed {
                            part.vel_hi_precision.x = -speed;
                        }
                    },
                    4 => {
                        part.vel_hi_precision.y -= speed;
                        if part.vel_hi_precision.y < speed {
                            part.vel_hi_precision.y = -speed;
                        }
                    },
                    _ => unreachable!()
                }
            }
        }
        true
    }
}

mod bob_the_fish {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 2000,
        mass: 1000,
        bounciness: 128,
        friction: 16,

        flags1: 0x4800,
        flags3: 0x0000,
        size_something2: (48, 48),
        size:            (48, 48),
        resize_x: None,
        resize_y: None,

        render_images: Some(&[
            &[(3, 0, 0, 0), (3, 1, 10, 11), (3, 12, 10, 27)],
            &[(3, 0, 0, 0), (3, 2, 15, 9), (3, 12, 10, 27)],
            &[(3, 0, 0, 0), (3, 3, 26, 16), (3, 12, 10, 27)],
            &[(3, 0, 0, 0), (3, 4, 31, 17), (3, 12, 10, 27)],
            &[(3, 0, 0, 0), (3, 5, 26, 16), (3, 12, 10, 27)],
            &[(3, 0, 0, 0), (3, 6, 10, 11), (3, 12, 10, 27)],
            &[(3, 0, 0, 0), (3, 7, 7, 11), (3, 12, 10, 27)],
            &[(3, 0, 0, 0), (3, 8, 6, 10), (3, 12, 10, 27)],
            &[(3, 0, 0, 0), (3, 9, 7, 16), (3, 12, 10, 27)],
            &[(3, 0, 0, 0), (3, 10, 8, 16), (3, 12, 10, 27)],
            &[(3, 0, 0, 0), (3, 11, 6, 10), (3, 12, 10, 27)],
            &[(3, 13, -16, 8)],
            &[(3, 14, -19, 15)],
            &[(3, 15, -25, 19)],
            &[(3, 16, -28, 25), (3, 17, 5, 39)],
            &[(3, 16, -28, 25), (3, 18, 5, 31)],
            &[(3, 16, -28, 25), (3, 19, 3, 29)],
            &[(3, 16, -28, 25), (3, 20, 5, 29)],
            &[(3, 16, -28, 25), (3, 21, -5, 27)],
            &[(3, 16, -28, 25), (3, 22, -5, 22)],
            &[(3, 16, -28, 25), (3, 23, 1, 31)],
            &[(3, 16, -28, 25), (3, 24, 5, 36)],
            &[(3, 16, -28, 25), (3, 25, 5, 38)],
        ]),
        render_pos_offsets: Some(&[
            (0,0), (0,0), (0,0), (0,0), (0,0), (0,0), (0,0), (0,0), (0,0), (0,0), (0,0), (-16,8), (-19,15), (-25,19), (-28,25), (-28,25), (-28,25), (-28,25), (-28,25), (-28,22), (-28,25), (-28,25), (-28,25), 
        ]),
        explicit_sizes: Some(&[
            (48,48), (48,48), (48,48), (48,48), (48,48), (48,48), (48,48), (48,48), (48,48), (48,48), (48,48), (88,43), (88,38), (96,37), (104,32), (104,32), (104,32), (104,32), (104,32), (104,35), (104,32), (104,32), (104,32), 
        ]),
        goobers: (3, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: Some(bounce),
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:06e0
    fn create(part: &mut Part) {
        part.flags2 |= 0x1000;
    }

    // TIMWIN: 1070:1a60
    fn reset(part: &mut Part) {
        part.set_border(&[(0,18), (11,0), (37,0), (47,18), (47,35), (39,47), (8,47), (0,34)]);
    }

    // TIMWIN: 1070:1ae5
    fn run(part: &mut Part) {
        if part.extra2 < 20 {
            // Bob is still alive
            part.state1 += 1;
        }
        if part.state1 < 23 {
            if part.state1 == 11 {
                part.state1 = 0;
            }
        } else {
            part.state1 = 14;

            // Death nears...
            part.extra2 += 1;
        }

        if part.state1 != part.state1_prev1 {
            unsafe {
                tim_c::part_set_size_and_pos_render(part);
            }
        }
    }

    // TIMWIN: 1070:1aae
    fn bounce(part: &mut Part) -> bool {
        let bob = part.bounce_part_mut().unwrap();
        if bob.state1 > 10 {
            true
        } else {
            break_bowl(bob);
            false
        }
    }

    // TIMWIN: 1070:1b36
    pub fn break_bowl(bob: &mut Part) {
        if bob.state1 < 11 {
            bob.state1 = 11;
            unsafe {
                tim_c::part_set_size_and_pos_render(bob);
                tim_c::play_sound(10);
            }

            bob.update_border_ignore_normals_quirk(&[(8,47), (24,44), (39,47)]);
        }
    }

    #[no_mangle]
    fn bob_the_fish_break_bowl(part: &mut Part) {
        break_bowl(part);
    }
}

mod bellows {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 3776,
        mass: 1000,
        bounciness: 128,
        friction: 16,

        flags1: 0x4800,
        flags3: 0x0008,
        size_something2: (64, 48),
        size:            (64, 48),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: Some(&[
            (0,0), (-8,8), (-11,12), 
        ]),
        explicit_sizes: Some(&[
            (64,48), (72,48), (80,48), 
        ]),
        goobers: (4, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: Some(bounce),
        flip_fn:   Some(flip),
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:0728
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 1048:08f5
    fn reset(part: &mut Part) {
        // reset_c!(bellows_reset, part);
        unimplemented();
    }

    // TIMWIN: 1048:099e
    fn run(part: &mut Part) {
        // run_c!(bellows_run, part);
        unimplemented();
    }

    // TIMWIN: 1048:08a6
    fn bounce(part: &mut Part) -> bool {
        // bounce_c!(bellows_bounce, part);
        unimplemented();
    }

    // TIMWIN: 1048:0959
    fn flip(part: &mut Part, _orientation: u16) {
        // flip_c!(bellows_flip, part, _orientation);
        unimplemented();
    }
}

mod bucket {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 7552,
        mass: 100,
        bounciness: 32,
        friction: 48,

        flags1: 0x0800,
        flags3: 0x0008,
        size_something2: (37, 48),
        size:            (40, 48),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (2, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    None,
        bounce_fn: Some(bounce),
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   Some(rope),
    };

    // TIMWIN: 1078:0770
    fn create(part: &mut Part) {
        part.flags1 |= 0x0020;
        part.flags2 |= 0x0004;
        part.rope_loc[0].x = 18;
        part.rope_loc[0].y = 0;
    }

    // TIMWIN: 1048:0dc1
    fn reset(part: &mut Part) {
        part.set_border(&[(0,19), (10,40), (25,40), (36,20), (27,47), (8,47)]);
    }

    // TIMWIN: 1048:0d66
    fn bounce(part: &mut Part) -> bool {
        let bucket = part.bounce_part().unwrap();
        let part_mid_x = part.pos_prev1.x + part.size.x/2;
        !(part.vel_hi_precision.y > 0 && part_mid_x > bucket.pos_prev1.x + 4 && part_mid_x < bucket.pos_prev1.x + 32)
    }

    // TIMWIN: 1048:0e23
    fn rope(p1: &mut Part, p2: &mut Part, rope_slot: u8, flags: u16, p1_mass: i16, p1_force: i32) -> u8 {
        if flags == 1 {
            p2.rope_mut(0).unwrap().rope_unknown += 1;
            return 0;
        }
        if p1.part_type == PartType::TeeterTotter.to_u16() {
            if p1_force < p2.force {
                return 1;
            }
        } else if p1_force < p2.force*2 {
            return 1;
        }
        return 0;
    }
}

mod cannon {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 14726,
        mass: 1000,
        bounciness: 192,
        friction: 12,

        flags1: 0x4800,
        flags3: 0x0008,
        size_something2: (64, 52),
        size:            (64, 52),
        resize_x: None,
        resize_y: None,

        render_images: Some(&[
            &[(4, 0, 0, 0), (4, 9, 9, 13)],
            &[(4, 0, 0, 0), (4, 9, 9, 13), (4, 10, -9, -6)],
            &[(4, 0, 0, 0), (4, 9, 9, 13), (4, 11, -9, -6)],
            &[(4, 0, 0, 0), (4, 9, 9, 13), (4, 12, -6, -5)],
            &[(4, 0, 0, 0), (4, 9, 9, 13), (4, 13, -7, -3)],
            &[(4, 0, 0, 0), (4, 9, 9, 13), (4, 14, -6, -2)],
            &[(4, 1, -7, -8), (4, 9, 9, 13)],
            &[(4, 2, -8, -11), (4, 9, 9, 13)],
            &[(4, 3, -2, -3), (4, 9, 9, 13), (0, 4, 83, -33)],
            &[(4, 5, -2, -7), (4, 9, 9, 13), (0, 6, 101, -46)],
            &[(4, 7, -3, -3), (4, 9, 9, 13), (0, 8, 127, -33)],
            &[(4, 0, 0, 0), (4, 9, 9, 13), (4, 15, 0, 2)],
        ]),
        render_pos_offsets: Some(&[
            (0,0), (-9,-6), (-9,-6), (-6,-5), (-7,-3), (-6,-2), (-7,-8), (-8,-11), (-2,-33), (-2,-46), (-3,-33), (0,0), 
        ]),
        explicit_sizes: Some(&[
            (64,52), (73,58), (73,58), (70,57), (71,55), (70,54), (53,60), (61,63), (194,84), (210,94), (199,84), (64,52), 
        ]),
        goobers: (4, 0),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: None,
        flip_fn:   Some(flip),
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:07c3
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 1048:138b
    fn reset(part: &mut Part) {
        // reset_c!(cannon_reset, part);
        unimplemented();
    }

    // TIMWIN: 1048:125a
    fn run(part: &mut Part) {
        // run_c!(cannon_run, part);
        unimplemented();
    }

    // TIMWIN: 1048:13ef
    fn flip(part: &mut Part, _orientation: u16) {
        // flip_c!(cannon_flip, part, _orientation);
        unimplemented();
    }
}

mod dynamite {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 1132,
        mass: 90,
        bounciness: 64,
        friction: 32,

        flags1: 0x0800,
        flags3: 0x0008,
        size_something2: (48, 28),
        size:            (48, 28),
        resize_x: None,
        resize_y: None,

        render_images: Some(&[
            &[(3, 0, 0, 0)],
            &[(3, 0, 0, 0), (3, 1, 40, 9)],
            &[(3, 0, 0, 0), (3, 2, 39, 10)],
            &[(3, 0, 0, 0), (3, 3, 39, 9)],
            &[(3, 0, 0, 0), (3, 4, 37, 13)],
            &[(3, 0, 0, 0), (3, 5, 38, 12)],
        ]),
        render_pos_offsets: Some(&[
            (0,0), (0,0), (0,0), (0,0), (0,0), (0,0), 
        ]),
        explicit_sizes: Some(&[
            (48,28), (56,28), (56,28), (56,28), (56,28), (56,28), 
        ]),
        goobers: (3, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: Some(bounce),
        flip_fn:   Some(flip),
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:0814
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 1068:0037
    fn reset(part: &mut Part) {
        // reset_c!(dynamite_reset, part);
        unimplemented();
    }

    // TIMWIN: 1068:009b
    fn run(part: &mut Part) {
        // run_c!(dynamite_run, part);
        unimplemented();
    }

    // TIMWIN: 1068:0000
    fn bounce(part: &mut Part) -> bool {
        // bounce_c!(dynamite_bounce, part);
        unimplemented();
    }

    // TIMWIN: 1068:00ed
    fn flip(part: &mut Part, _orientation: u16) {
        // flip_c!(dynamite_flip, part, _orientation);
        unimplemented();
    }
}

mod gun_bullet {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 0,
        mass: 20000,
        bounciness: 0,
        friction: 1,

        flags1: 0x0800,
        flags3: 0x0008,
        size_something2: (40, 7),
        size:            (40, 7),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: Some(&[
            (0,0), (4,-3), (1,-12), 
        ]),
        explicit_sizes: None,
        goobers: (3, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: Some(bounce),
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:0865
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 1048:0ee8
    fn reset(part: &mut Part) {
        // reset_c!(gun_bullet_reset, part);
        unimplemented();
    }

    // TIMWIN: 1048:0f3e
    fn run(part: &mut Part) {
        // run_c!(gun_bullet_run, part);
        unimplemented();
    }

    // TIMWIN: 1048:0e9e
    fn bounce(part: &mut Part) -> bool {
        // bounce_c!(gun_bullet_bounce, part);
        unimplemented();
    }
}

mod light_switch_outlet {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 3776,
        mass: 1000,
        bounciness: 128,
        friction: 16,

        flags1: 0x4800,
        flags3: 0x0000,
        size_something2: (48, 32),
        size:            (48, 32),
        resize_x: None,
        resize_y: None,

        render_images: Some(&[
            &[(5, 0, 0, 0), (5, 1, 8, 8)],
            &[(5, 0, 0, 0), (5, 1, 8, 8), (5, 3, 29, 4)],
            &[(5, 0, 0, 0), (5, 1, 8, 8), (5, 3, 29, 18)],
            &[(5, 0, 0, 0), (5, 1, 8, 8), (5, 3, 29, 4), (5, 3, 29, 18)],
            &[(5, 0, 0, 0), (5, 2, 8, 8)],
            &[(5, 0, 0, 0), (5, 2, 8, 8), (5, 3, 29, 4)],
            &[(5, 0, 0, 0), (5, 2, 8, 8), (5, 3, 29, 18)],
            &[(5, 0, 0, 0), (5, 2, 8, 8), (5, 3, 29, 4), (5, 3, 29, 18)],
        ]),
        render_pos_offsets: None,
        explicit_sizes: Some(&[
            (48,32), (48,32), (48,32), (48,32), (48,32), (48,32), (48,32), (48,32), 
        ]),
        goobers: (5, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: Some(bounce),
        flip_fn:   Some(flip),
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:08a8
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 1070:008c
    fn reset(part: &mut Part) {
        // reset_c!(light_switch_outlet_reset, part);
        unimplemented();
    }

    // TIMWIN: 1070:0103
    fn run(part: &mut Part) {
        // run_c!(light_switch_outlet_run, part);
        unimplemented();
    }

    // TIMWIN: 1070:0000
    fn bounce(part: &mut Part) -> bool {
        // bounce_c!(light_switch_outlet_bounce, part);
        unimplemented();
    }

    // TIMWIN: 1070:014b
    fn flip(part: &mut Part, _orientation: u16) {
        // flip_c!(light_switch_outlet_flip, part, _orientation);
        unimplemented();
    }
}

mod dynamite_with_plunger {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 3776,
        mass: 1000,
        bounciness: 128,
        friction: 16,

        flags1: 0x4800,
        flags3: 0x0008,
        size_something2: (135, 47),
        size:            (135, 47),
        resize_x: None,
        resize_y: None,

        render_images: Some(&[
            &[(4, 0, 0, 19), (4, 2, 103, 0), (4, 1, 40, 16)],
            &[(4, 0, 0, 19), (4, 2, 103, 5), (4, 1, 40, 16)],
            &[(4, 2, 103, 10), (4, 1, 40, 16)],
        ]),
        render_pos_offsets: Some(&[
            (0,0), (0,0), (0,0), 
        ]),
        explicit_sizes: Some(&[
            (135,48), (135,46), (135,41), 
        ]),
        goobers: (4, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: Some(bounce),
        flip_fn:   Some(flip),
        resize_fn: None,
        rope_fn:   Some(rope),
    };

    // TIMWIN: 1078:08f9
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 1090:0bc2
    fn reset(part: &mut Part) {
        // reset_c!(dynamite_with_plunger_reset, part);
        unimplemented();
    }

    // TIMWIN: 1090:0c56
    fn run(part: &mut Part) {
        // run_c!(dynamite_with_plunger_run, part);
        unimplemented();
    }

    // TIMWIN: 1090:0b60
    fn bounce(part: &mut Part) -> bool {
        // bounce_c!(dynamite_with_plunger_bounce, part);
        unimplemented();
    }

    // TIMWIN: 1090:0d12
    fn flip(part: &mut Part, _orientation: u16) {
        // flip_c!(dynamite_with_plunger_flip, part, _orientation);
        unimplemented();
    }

    // TIMWIN: 1090:0d59
    fn rope(p1: &mut Part, p2: &mut Part, rope_slot: u8, flags: u16, p1_mass: i16, p1_force: i32) -> u8 {
        // rope_c!(dynamite_with_plunger_rope, p1, p2, rope_slot, flags, p1_mass, p1_force);
        unimplemented();
    }
}

mod eye_hook {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 7552,
        mass: 1000,
        bounciness: 128,
        friction: 16,

        flags1: 0x4800,
        flags3: 0x0008,
        size_something2: (16, 16),
        size:            (16, 16),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (2, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    None,
        bounce_fn: None,
        flip_fn:   Some(flip),
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:0946
    fn create(part: &mut Part) {
        part.flags1 |= 0x0200;
        part.flags2 |= 0x0004;
    }

    // TIMWIN: 1070:05a6
    fn reset(part: &mut Part) {
        part.rope_loc[0].y = if part.flags2 & F2_FLIP_VERT != 0 {
            14
        } else {
            1
        };
    }

    // TIMWIN: 1070:05d8
    fn flip(part: &mut Part, _orientation: u16) {
        part.flags2 ^= F2_FLIP_VERT;
        reset(part);
        unsafe {
            tim_c::stub_10a8_280a(part, 3);
            tim_c::stub_10a8_2b6d(part, 3);
            tim_c::stub_10a8_21cb(part, 2);
        }
    }
}

mod fan {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 7552,
        mass: 1000,
        bounciness: 1024,
        friction: 16,

        flags1: 0x4800,
        flags3: 0x0008,
        size_something2: (32, 32),
        size:            (32, 32),
        resize_x: None,
        resize_y: None,

        render_images: Some(&[
            &[(4, 0, 0, 8), (4, 1, 16, 0)],
            &[(4, 0, 0, 8), (4, 2, 16, 0)],
            &[(4, 0, 0, 8), (4, 3, 16, 0)],
            &[(4, 0, 0, 8), (4, 4, 16, 0)],
        ]),
        render_pos_offsets: None,
        explicit_sizes: Some(&[
            (32,32), (32,32), (32,32), (32,32), 
        ]),
        goobers: (4, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: None,
        flip_fn:   Some(flip),
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:0974
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 1070:0620
    fn reset(part: &mut Part) {
        // reset_c!(fan_reset, part);
        unimplemented();
    }

    // TIMWIN: 1070:0676
    fn run(part: &mut Part) {
        // run_c!(fan_run, part);
        unimplemented();
    }

    // TIMWIN: 1070:07bc
    fn flip(part: &mut Part, _orientation: u16) {
        // flip_c!(fan_flip, part, _orientation);
        unimplemented();
    }
}

mod flashlight {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 7552,
        mass: 1000,
        bounciness: 128,
        friction: 16,

        flags1: 0x4800,
        flags3: 0x0008,
        size_something2: (32, 16),
        size:            (32, 16),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: Some(&[
            (0,0), (0,-10), 
        ]),
        explicit_sizes: None,
        goobers: (2, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: Some(bounce),
        flip_fn:   Some(flip),
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:09c5
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 1070:1bcd
    fn reset(part: &mut Part) {
        // reset_c!(flashlight_reset, part);
        unimplemented();
    }

    // TIMWIN: 1070:1c23
    fn run(part: &mut Part) {
        // run_c!(flashlight_run, part);
        unimplemented();
    }

    // TIMWIN: 1070:1b9e
    fn bounce(part: &mut Part) -> bool {
        // bounce_c!(flashlight_bounce, part);
        unimplemented();
    }

    // TIMWIN: 1070:1c67
    fn flip(part: &mut Part, _orientation: u16) {
        // flip_c!(flashlight_flip, part, _orientation);
        unimplemented();
    }
}

mod generator {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 3776,
        mass: 1000,
        bounciness: 128,
        friction: 16,

        flags1: 0x4800,
        flags3: 0x0000,
        size_something2: (72, 32),
        size:            (72, 32),
        resize_x: None,
        resize_y: None,

        render_images: Some(&[
            &[(5, 0, 0, 0), (5, 1, 21, 0)],
            &[(5, 0, 0, 0), (5, 2, 21, -6)],
            &[(5, 0, 0, 0), (5, 3, 21, 1)],
            &[(5, 0, 0, 0), (5, 4, 21, -5)],
            &[(5, 0, 0, 0), (5, 1, 21, 0), (5, 5, 5, 4)],
            &[(5, 0, 0, 0), (5, 2, 21, -6), (5, 5, 5, 4)],
            &[(5, 0, 0, 0), (5, 3, 21, 1), (5, 5, 5, 4)],
            &[(5, 0, 0, 0), (5, 4, 21, -5), (5, 5, 5, 4)],
            &[(5, 0, 0, 0), (5, 1, 21, 0), (5, 5, 5, 18)],
            &[(5, 0, 0, 0), (5, 2, 21, -6), (5, 5, 5, 18)],
            &[(5, 0, 0, 0), (5, 3, 21, 1), (5, 5, 5, 18)],
            &[(5, 0, 0, 0), (5, 4, 21, -5), (5, 5, 5, 18)],
            &[(5, 0, 0, 0), (5, 1, 21, 0), (5, 5, 5, 4), (5, 5, 5, 18)],
            &[(5, 0, 0, 0), (5, 2, 21, -6), (5, 5, 5, 4), (5, 5, 5, 18)],
            &[(5, 0, 0, 0), (5, 3, 21, 1), (5, 5, 5, 4), (5, 5, 5, 18)],
            &[(5, 0, 0, 0), (5, 4, 21, -5), (5, 5, 5, 4), (5, 5, 5, 18)],
        ]),
        render_pos_offsets: Some(&[
            (0,0), (0,-6), (0,0), (0,-5), (0,0), (0,-6), (0,0), (0,-5), (0,0), (0,-6), (0,0), (0,-5), (0,0), (0,-6), (0,0), (0,-5), 
        ]),
        explicit_sizes: Some(&[
            (80,32), (80,38), (80,32), (80,37), (80,32), (80,38), (80,32), (80,37), (80,32), (80,38), (80,32), (80,37), (80,32), (80,38), (80,32), (80,37), 
        ]),
        goobers: (5, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: Some(bounce),
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:0a0d
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 1078:1a0a
    fn reset(part: &mut Part) {
        // reset_c!(generator_reset, part);
        unimplemented();
    }

    // TIMWIN: 1078:1a7d
    fn run(part: &mut Part) {
        // run_c!(generator_run, part);
        unimplemented();
    }

    // TIMWIN: 1078:19f2
    fn bounce(part: &mut Part) -> bool {
        // bounce_c!(generator_bounce, part);
        unimplemented();
    }
}

mod gun {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 7552,
        mass: 1000,
        bounciness: 192,
        friction: 16,

        flags1: 0x4800,
        flags3: 0x0008,
        size_something2: (64, 31),
        size:            (64, 31),
        resize_x: None,
        resize_y: None,

        render_images: Some(&[
            &[(4, 0, 0, 0)],
            &[(4, 1, -1, -5)],
            &[(4, 2, -2, -3)],
            &[(4, 3, -15, -6)],
            &[(4, 4, -11, -3)],
            &[(4, 0, -2, 0), (4, 5, 64, -12)],
            &[(4, 0, 0, 0)],
        ]),
        render_pos_offsets: Some(&[
            (0,0), (-1,-5), (-2,-3), (-15,-6), (-11,-3), (-2,-12), (0,0), 
        ]),
        explicit_sizes: Some(&[
            (64,31), (56,36), (128,37), (112,34), (128,34), (128,43), (64,31), 
        ]),
        goobers: (4, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: None,
        flip_fn:   Some(flip),
        resize_fn: None,
        rope_fn:   Some(rope),
    };

    // TIMWIN: 1078:0a59
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 1078:1c49
    fn reset(part: &mut Part) {
        // reset_c!(gun_reset, part);
        unimplemented();
    }

    // TIMWIN: 1078:1b3e
    fn run(part: &mut Part) {
        // run_c!(gun_run, part);
        unimplemented();
    }

    // TIMWIN: 1078:1cad
    fn flip(part: &mut Part, _orientation: u16) {
        // flip_c!(gun_flip, part, _orientation);
        unimplemented();
    }

    // TIMWIN: 1078:1cfd
    fn rope(p1: &mut Part, p2: &mut Part, rope_slot: u8, flags: u16, p1_mass: i16, p1_force: i32) -> u8 {
        // rope_c!(gun_rope, p1, p2, rope_slot, flags, p1_mass, p1_force);
        unimplemented();
    }
}

mod baseball {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 2000,
        mass: 9,
        bounciness: 64,
        friction: 24,

        flags1: 0x0800,
        flags3: 0x0008,
        size_something2: (15, 15),
        size:            (15, 15),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (3, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    None,
        bounce_fn: None,
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:0aa6
    fn create(part: &mut Part) {
        // do nothing
    }

    // TIMWIN: 1048:0608
    fn reset(part: &mut Part) {
        part.set_border(&[(3,0), (11,0), (14,4), (14,10), (11,14), (3,14), (0,10), (0,4)]);
    }
}

mod lightbulb {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 1300,
        mass: 1000,
        bounciness: 128,
        friction: 16,

        flags1: 0x4800,
        flags3: 0x0008,
        size_something2: (32, 32),
        size:            (32, 32),
        resize_x: None,
        resize_y: None,

        render_images: Some(&[
            &[(2, 0, 0, 0), (2, 4, 20, 28)],
            &[(2, 1, -8, -18), (2, 5, 20, 28)],
            &[(2, 2, 0, 0), (2, 4, 19, 2)],
            &[(2, 3, -8, 0), (2, 5, 19, 2)],
        ]),
        render_pos_offsets: Some(&[
            (0,0), (-8,-18), (0,0), (-8,0), 
        ]),
        explicit_sizes: Some(&[
            (32,54), (47,72), (32,38), (47,50), 
        ]),
        goobers: (2, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: Some(bounce),
        flip_fn:   Some(flip),
        resize_fn: None,
        rope_fn:   Some(rope),
    };

    // TIMWIN: 1078:0ae9
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 1080:1c4c
    fn reset(part: &mut Part) {
        // reset_c!(lightbulb_reset, part);
        unimplemented();
    }

    // TIMWIN: 1080:1c98
    fn run(part: &mut Part) {
        // run_c!(lightbulb_run, part);
        unimplemented();
    }

    // TIMWIN: 1080:1c80
    fn bounce(part: &mut Part) -> bool {
        // bounce_c!(lightbulb_bounce, part);
        unimplemented();
    }

    // TIMWIN: 1080:1cd8
    fn flip(part: &mut Part, _orientation: u16) {
        // flip_c!(lightbulb_flip, part, _orientation);
        unimplemented();
    }

    // TIMWIN: 1080:1d3d
    fn rope(p1: &mut Part, p2: &mut Part, rope_slot: u8, flags: u16, p1_mass: i16, p1_force: i32) -> u8 {
        // rope_c!(lightbulb_rope, p1, p2, rope_slot, flags, p1_mass, p1_force);
        unimplemented();
    }
}

mod magnifying_glass {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 3776,
        mass: 1000,
        bounciness: 128,
        friction: 16,

        flags1: 0x4800,
        flags3: 0x0008,
        size_something2: (16, 37),
        size:            (16, 37),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (3, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: None,
        flip_fn:   Some(flip),
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:0b18
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 1088:0f8e
    fn reset(part: &mut Part) {
        // reset_c!(magnifying_glass_reset, part);
        unimplemented();
    }

    // TIMWIN: 1088:0fa3
    fn run(part: &mut Part) {
        // run_c!(magnifying_glass_run, part);
        unimplemented();
    }

    // TIMWIN: 1088:113f
    fn flip(part: &mut Part, _orientation: u16) {
        // flip_c!(magnifying_glass_flip, part, _orientation);
        unimplemented();
    }
}

mod kelly_the_monkey {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 3776,
        mass: 1000,
        bounciness: 128,
        friction: 16,

        flags1: 0x4800,
        flags3: 0x0008,
        size_something2: (92, 79),
        size:            (92, 79),
        resize_x: None,
        resize_y: None,

        render_images: Some(&[
            &[(4, 0, 0, 12), (4, 4, 40, 0)],
            &[(4, 0, 0, 12), (4, 5, 40, 6)],
            &[(4, 1, 0, 12), (4, 5, 40, 6)],
            &[(4, 2, 0, 12), (4, 5, 40, 6)],
            &[(4, 3, 0, 12), (4, 5, 40, 6)],
            &[(4, 0, 0, 12), (4, 4, 40, 0), (4, 6, 14, 1)],
            &[(4, 0, 0, 12), (4, 4, 40, 0), (4, 7, 16, 1)],
            &[(4, 0, 0, 12), (4, 4, 40, 0), (4, 8, 16, 1)],
            &[(4, 0, 0, 12), (4, 4, 40, 0), (4, 9, 15, 1)],
            &[(4, 0, 0, 12), (4, 5, 40, 6), (4, 6, 14, 1)],
            &[(4, 0, 0, 12), (4, 5, 40, 6), (4, 7, 16, 1)],
            &[(4, 0, 0, 12), (4, 5, 40, 6), (4, 8, 16, 1)],
            &[(4, 0, 0, 12), (4, 5, 40, 6), (4, 9, 14, 1)],
        ]),
        render_pos_offsets: None,
        explicit_sizes: Some(&[
            (92,79), (92,79), (92,79), (92,79), (92,79), (92,79), (92,79), (92,79), (92,79), (92,79), (92,79), (92,79), (92,79), 
        ]),
        goobers: (4, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: Some(bounce),
        flip_fn:   Some(flip),
        resize_fn: None,
        rope_fn:   Some(rope),
    };

    // TIMWIN: 1078:0b42
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 1088:0b88
    fn reset(part: &mut Part) {
        // reset_c!(kelly_the_monkey_reset, part);
        unimplemented();
    }

    // TIMWIN: 1088:0bf9
    fn run(part: &mut Part) {
        // run_c!(kelly_the_monkey_run, part);
        unimplemented();
    }

    // TIMWIN: 1088:0b30
    fn bounce(part: &mut Part) -> bool {
        // bounce_c!(kelly_the_monkey_bounce, part);
        unimplemented();
    }

    // TIMWIN: 1088:0cff
    fn flip(part: &mut Part, _orientation: u16) {
        // flip_c!(kelly_the_monkey_flip, part, _orientation);
        unimplemented();
    }

    // TIMWIN: 1088:0d4f
    fn rope(p1: &mut Part, p2: &mut Part, rope_slot: u8, flags: u16, p1_mass: i16, p1_force: i32) -> u8 {
        // rope_c!(kelly_the_monkey_rope, p1, p2, rope_slot, flags, p1_mass, p1_force);
        unimplemented();
    }
}

mod jack_o_lantern {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 2400,
        mass: 100,
        bounciness: 64,
        friction: 32,

        flags1: 0x0800,
        flags3: 0x0008,
        size_something2: (39, 33),
        size:            (39, 33),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (3, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    None,
        bounce_fn: None,
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:0b8f
    fn create(part: &mut Part) {
        // do nothing
    }

    // TIMWIN: 1090:104c
    fn reset(part: &mut Part) {
        part.set_border(&[(0,15), (9,9), (29,9), (38,18), (38,22), (27,32), (11,32), (0,22)]);
    }
}

mod heart_balloon {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 11,
        mass: 4,
        bounciness: 128,
        friction: 8,

        flags1: 0x0800,
        flags3: 0x0008,
        size_something2: (37, 39),
        size:            (37, 39),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (3, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    None,
        bounce_fn: None,
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   Some(rope),
    };

    // TIMWIN: 1078:0bd2
    fn create(part: &mut Part) {
        part.flags1 |= 0x0020;
        part.flags2 |= 0x0004;

        part.rope_loc[0].x = 18;
        part.rope_loc[0].y = 35;
    }

    // TIMWIN: 1080:0470
    fn reset(part: &mut Part) {
        part.set_border(&[(0,8), (6,0), (30,0), (36,7), (36,16), (17,35), (0,16)]);
    }

    // TIMWIN: 1080:04b8
    fn rope(p1: &mut Part, p2: &mut Part, rope_slot: u8, flags: u16, p1_mass: i16, p1_force: i32) -> u8 {
        // rope_c!(heart_balloon_rope, p1, p2, rope_slot, flags, p1_mass, p1_force);
        unimplemented();
    }
}

mod christmas_tree {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 7552,
        mass: 1000,
        bounciness: 128,
        friction: 16,

        flags1: 0x4800,
        flags3: 0x0008,
        size_something2: (41, 73),
        size:            (41, 73),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (4, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    None,
        bounce_fn: None,
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:0c25
    fn create(part: &mut Part) {
        // do nothing
    }

    // TIMWIN: 1048:192e
    fn reset(part: &mut Part) {
        part.set_border(&[(0,53), (20,0), (39,55), (25,61), (25,72), (16,72), (16,61)]);
    }
}

mod boxing_glove {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 3776,
        mass: 1000,
        bounciness: 128,
        friction: 16,

        flags1: 0x4800,
        flags3: 0x0008,
        size_something2: (48, 31),
        size:            (48, 31),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: Some(&[
            (0,0), (7,-12), (-29,-3), (-84,-6), (-15,-3), (-29,-3), (-30,5), (-21,5), (-25,6), (-21,6), 
        ]),
        explicit_sizes: None,
        goobers: (3, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: Some(bounce),
        flip_fn:   Some(flip),
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:0c68
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 1048:0c34
    fn reset(part: &mut Part) {
        // reset_c!(boxing_glove_reset, part);
        unimplemented();
    }

    // TIMWIN: 1048:0b2f
    fn run(part: &mut Part) {
        // run_c!(boxing_glove_run, part);
        unimplemented();
    }

    // TIMWIN: 1048:0af6
    fn bounce(part: &mut Part) -> bool {
        // bounce_c!(boxing_glove_bounce, part);
        unimplemented();
    }

    // TIMWIN: 1048:0ca4
    fn flip(part: &mut Part, _orientation: u16) {
        // flip_c!(boxing_glove_flip, part, _orientation);
        unimplemented();
    }
}

mod rocket {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 18000,
        mass: 1800,
        bounciness: 128,
        friction: 32,

        flags1: 0x0800,
        flags3: 0x0008,
        size_something2: (16, 52),
        size:            (16, 52),
        resize_x: None,
        resize_y: None,

        render_images: Some(&[
            &[(3, 0, 0, 0), (3, 1, 3, 45)],
            &[(3, 0, 0, 0), (3, 2, 2, 45)],
            &[(3, 0, 0, 0), (3, 3, 2, 45)],
            &[(3, 0, 0, 0), (3, 4, 0, 45)],
            &[(3, 0, 0, 0), (3, 5, -2, 45)],
            &[(3, 0, 0, 0), (3, 6, -2, 45)],
            &[(3, 0, 0, 0), (3, 7, 0, 44)],
            &[(3, 0, 0, 0), (3, 8, 1, 46)],
            &[(3, 0, 0, 0), (3, 9, 0, 46)],
            &[(3, 0, 0, 0), (3, 10, 0, 46)],
        ]),
        render_pos_offsets: Some(&[
            (0,0), (0,0), (0,0), (0,0), (-2,0), (-2,0), (0,0), (0,0), (0,0), (0,0), 
        ]),
        explicit_sizes: Some(&[
            (16,66), (16,74), (16,70), (16,65), (18,59), (18,56), (16,66), (16,81), (16,83), (16,82), 
        ]),
        goobers: (3, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: None,
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:0cb0
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 1090:176e
    fn reset(part: &mut Part) {
        // reset_c!(rocket_reset, part);
        unimplemented();
    }

    // TIMWIN: 1090:1652
    fn run(part: &mut Part) {
        // run_c!(rocket_run, part);
        unimplemented();
    }
}

mod scissors {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 7552,
        mass: 1000,
        bounciness: 128,
        friction: 16,

        flags1: 0x4800,
        flags3: 0x0008,
        size_something2: (40, 32),
        size:            (40, 32),
        resize_x: None,
        resize_y: None,

        render_images: Some(&[
            &[(0, 0, 0, 0), (4, 1, 21, 17)],
            &[(4, 2, -2, 4)],
        ]),
        render_pos_offsets: Some(&[
            (0,0), (-2,0), 
        ]),
        explicit_sizes: Some(&[
            (40,34), (48,24), 
        ]),
        goobers: (4, 0),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: Some(bounce),
        flip_fn:   Some(flip),
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:0cfc
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 10a0:0145
    fn reset(part: &mut Part) {
        // reset_c!(scissors_reset, part);
        unimplemented();
    }

    // TIMWIN: 10a0:01a9
    fn run(part: &mut Part) {
        // run_c!(scissors_run, part);
        unimplemented();
    }

    // TIMWIN: 10a0:00be
    fn bounce(part: &mut Part) -> bool {
        // bounce_c!(scissors_bounce, part);
        unimplemented();
    }

    // TIMWIN: 10a0:020e
    fn flip(part: &mut Part, _orientation: u16) {
        // flip_c!(scissors_flip, part, _orientation);
        unimplemented();
    }
}

mod solar_panels {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 7552,
        mass: 1000,
        bounciness: 128,
        friction: 16,

        flags1: 0x4800,
        flags3: 0x0000,
        size_something2: (72, 32),
        size:            (72, 32),
        resize_x: None,
        resize_y: None,

        render_images: Some(&[
            &[(5, 0, 0, 0)],
            &[(5, 0, 0, 0), (5, 1, 52, 4)],
            &[(5, 0, 0, 0), (5, 1, 52, 18)],
            &[(5, 0, 0, 0), (5, 1, 52, 4), (5, 1, 52, 18)],
        ]),
        render_pos_offsets: None,
        explicit_sizes: Some(&[
            (72,32), (72,32), (72,32), (72,32), 
        ]),
        goobers: (5, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: None,
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:0d49
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 10a0:0cce
    fn reset(part: &mut Part) {
        // reset_c!(solar_panels_reset, part);
        unimplemented();
    }

    // TIMWIN: 10a0:0d01
    fn run(part: &mut Part) {
        // run_c!(solar_panels_run, part);
        unimplemented();
    }
}

mod trampoline {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 7552,
        mass: 1000,
        bounciness: 128,
        friction: 32,

        flags1: 0x4800,
        flags3: 0x0000,
        size_something2: (48, 28),
        size:            (48, 28),
        resize_x: None,
        resize_y: None,

        render_images: Some(&[
            &[(4, 0, 0, 0), (0, 1, 0, 9)],
            &[(4, 2, 0, 0), (0, 3, 0, 9)],
            &[(4, 4, 0, 0), (0, 5, 0, 9)],
            &[(4, 6, 0, 0), (0, 1, 0, 9)],
            &[(4, 7, 0, -3), (0, 1, 0, 9)],
        ]),
        render_pos_offsets: Some(&[
            (0,0), (0,0), (0,0), (0,0), (0,-3), 
        ]),
        explicit_sizes: Some(&[
            (48,28), (48,28), (48,28), (48,28), (48,31), 
        ]),
        goobers: (4, 0),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: Some(bounce),
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:0d6e
    fn create(part: &mut Part) {
        part.flags2 |= 0x1000;
    }

    // TIMWIN: 10d0:0a00
    fn reset(part: &mut Part) {
        part.set_border(&[(0,11), (47,11), (47,27), (0,27)]);
    }

    // TIMWIN: 10d0:0a4e
    fn run(part: &mut Part) {
        if part.state2 != 0 {
            part.state1 += 1;
            if part.state1 == 3 {
                tim_c::play_sound(3);
            }
            if part.state1 == 5 {
                part.state1 = 0;
                part.state2 = 0;
            }
            unsafe { tim_c::part_set_size_and_pos_render(part) };
        }
    }

    // TIMWIN: 10d0:0900
    fn bounce(part: &mut Part) -> bool {
        let trampoline = unsafe { part.bounce_part.as_mut().unwrap() };
        if part.bounce_border_index != 0 {
            return true;
        }
        
        let is_mel_schlemming = part.part_type == PartType::MelSchlemming.to_u16();

        let trampoline_center_x = trampoline.pos.x + trampoline.size.x/2;

        let rel_x = if is_mel_schlemming {
            part.pos.x + part.size_something2.x/2 - trampoline_center_x
        } else {
            part.pos.x + part.size.x/2 - trampoline_center_x
        };

        if rel_x > -15 && rel_x < 15 {
            trampoline.state2 = 1;
            if part.vel_hi_precision.x.abs() >= 1024 {
                part.vel_hi_precision.x /= 2;
            }
            part.bounce_part = std::ptr::null_mut();
            part.flags1 &= !(0x0001);
            part.pos_y_hi_precision = (part.pos.y as i32) * 512;
            if trampoline.state1 == 3 {
                part.vel_hi_precision.y = -part.vel_hi_precision.y.abs() - 1024;
                unsafe { tim_c::part_clamp_to_terminal_velocity(part); }
            }

            false
        } else {
            if is_mel_schlemming {
                part.vel_hi_precision.y = 0;
                if part.state1 > 5 && part.state1 < 9 {
                    part.state1 = 9;
                }
                unsafe { tim_c::part_set_size_and_pos_render(part); }
            }
            true
        }
    }
}

mod windmill {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 7552,
        mass: 1000,
        bounciness: 128,
        friction: 16,

        flags1: 0x4800,
        flags3: 0x0008,
        size_something2: (40, 48),
        size:            (40, 48),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: Some(&[
            (0,0), (-3,-3), (-4,-4), (-3,-3), 
        ]),
        explicit_sizes: None,
        goobers: (4, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: None,
        flip_fn:   Some(flip),
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:0db6
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 10d0:0f5a
    fn reset(part: &mut Part) {
        // reset_c!(windmill_reset, part);
        unimplemented();
    }

    // TIMWIN: 10d0:0f9e
    fn run(part: &mut Part) {
        // run_c!(windmill_run, part);
        unimplemented();
    }

    // TIMWIN: 10d0:1035
    fn flip(part: &mut Part, _orientation: u16) {
        // flip_c!(windmill_flip, part, _orientation);
        unimplemented();
    }
}

mod explosion {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 0,
        mass: 1,
        bounciness: 256,
        friction: 0,

        flags1: 0x0000,
        flags3: 0x0008,
        size_something2: (0, 0),
        size:            (0, 0),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: Some(&[
            (0,0), (-4,-13), (0,-6), (13,9), (20,19), (20,20), 
        ]),
        explicit_sizes: None,
        goobers: (0, 255),

        create_fn: create,
        reset_fn:  None,
        run_fn:    Some(run),
        bounce_fn: None,
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 0000:0000
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 1070:01a8
    fn run(part: &mut Part) {
        // run_c!(explosion_run, part);
        unimplemented();
    }
}

mod mort_the_mouse {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 2000,
        mass: 1,
        bounciness: 0,
        friction: 256,

        flags1: 0x0800,
        flags3: 0x0008,
        size_something2: (24, 11),
        size:            (24, 11),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: Some(&[
            (0,0), (0,1), 
        ]),
        explicit_sizes: None,
        goobers: (3, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: Some(bounce),
        flip_fn:   Some(flip),
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:0e10
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 1090:0e4e
    fn reset(part: &mut Part) {
        // reset_c!(mort_the_mouse_reset, part);
        unimplemented();
    }

    // TIMWIN: 1090:0ebe
    fn run(part: &mut Part) {
        // run_c!(mort_the_mouse_run, part);
        unimplemented();
    }

    // TIMWIN: 1090:0ea6
    fn bounce(part: &mut Part) -> bool {
        // bounce_c!(mort_the_mouse_bounce, part);
        unimplemented();
    }

    // TIMWIN: 1090:100e
    fn flip(part: &mut Part, _orientation: u16) {
        // flip_c!(mort_the_mouse_flip, part, _orientation);
        unimplemented();
    }
}

mod cannon_ball {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 21428,
        mass: 28000,
        bounciness: 32,
        friction: 16,

        flags1: 0x0800,
        flags3: 0x0008,
        size_something2: (24, 23),
        size:            (24, 23),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (3, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    None,
        bounce_fn: None,
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:0e5d
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 1048:0592
    fn reset(part: &mut Part) {
        // reset_c!(cannon_ball_reset, part);
        unimplemented();
    }
}

mod tennis_ball {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 1322,
        mass: 5,
        bounciness: 192,
        friction: 16,

        flags1: 0x0800,
        flags3: 0x0008,
        size_something2: (15, 15),
        size:            (15, 15),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (3, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    None,
        bounce_fn: None,
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:0ea0
    fn create(part: &mut Part) {
        // do nothing
    }

    // TIMWIN: 1048:0608
    fn reset(part: &mut Part) {
        part.set_border(&[(3,0), (11,0), (14,4), (14,10), (11,14), (3,14), (0,10), (0,4)]);
    }
}

mod candle {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 2000,
        mass: 12,
        bounciness: 128,
        friction: 32,

        flags1: 0x0800,
        flags3: 0x0008,
        size_something2: (32, 32),
        size:            (32, 32),
        resize_x: None,
        resize_y: None,

        render_images: Some(&[
            &[(3, 0, 0, 0)],
            &[(3, 0, 0, 0), (3, 1, 11, -4)],
            &[(3, 0, 0, 0), (3, 2, 11, -4)],
            &[(3, 0, 0, 0), (3, 3, 11, -4)],
            &[(3, 0, 0, 0), (3, 4, 11, -4)],
            &[(3, 0, 0, 0), (3, 5, 11, -4)],
        ]),
        render_pos_offsets: Some(&[
            (0,0), (0,-4), (0,-4), (0,-4), (0,-4), (0,-4), 
        ]),
        explicit_sizes: Some(&[
            (34,32), (34,36), (34,36), (34,36), (34,36), (34,36), 
        ]),
        goobers: (3, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: None,
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:0ee3
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 1048:1122
    fn reset(part: &mut Part) {
        // reset_c!(candle_reset, part);
        unimplemented();
    }

    // TIMWIN: 1048:116e
    fn run(part: &mut Part) {
        // run_c!(candle_run, part);
        unimplemented();
    }
}

mod pipe_straight {
    use super::prelude::*;
    use super::wall_common::{create, reset, resize};
    pub const DEF: PartDef = PartDef {
        density: 7552,
        mass: 1000,
        bounciness: 1024,
        friction: 16,

        flags1: 0x4800,
        flags3: 0x0000,
        size_something2: (32, 16),
        size:            (32, 16),
        resize_x: Some((32, 240)),
        resize_y: Some((32, 240)),

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (4, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    None,
        bounce_fn: None,
        flip_fn:   None,
        resize_fn: Some(resize),
        rope_fn:   None,
    };
}

mod pipe_curved {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 7552,
        mass: 1000,
        bounciness: 1024,
        friction: 16,

        flags1: 0x4800,
        flags3: 0x0000,
        size_something2: (32, 32),
        size:            (32, 32),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (4, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    None,
        bounce_fn: None,
        flip_fn:   Some(flip),
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:0f33
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 10a0:0000
    fn reset(part: &mut Part) {
        // reset_c!(pipe_curved_reset, part);
        unimplemented();
    }

    // TIMWIN: 10a0:006f
    fn flip(part: &mut Part, _orientation: u16) {
        // flip_c!(pipe_curved_flip, part, _orientation);
        unimplemented();
    }
}

mod wood_wall {
    use super::prelude::*;
    use super::wall_common::{create, reset, resize};
    pub const DEF: PartDef = PartDef {
        density: 7552,
        mass: 1000,
        bounciness: 1024,
        friction: 12,

        flags1: 0x4800,
        flags3: 0x0000,
        size_something2: (32, 16),
        size:            (32, 16),
        resize_x: Some((32, 240)),
        resize_y: Some((32, 240)),

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (4, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    None,
        bounce_fn: None,
        flip_fn:   None,
        resize_fn: Some(resize),
        rope_fn:   None,
    };
}

mod rope_severed_end {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 1600,
        mass: 1000,
        bounciness: 0,
        friction: 0,

        flags1: 0x0800,
        flags3: 0x0008,
        size_something2: (0, 0),
        size:            (0, 0),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (4, 255),

        create_fn: create,
        reset_fn:  None,
        run_fn:    None,
        bounce_fn: None,
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:0f7b
    fn create(part: &mut Part) {
        part.flags2 |= 0x0004;
        part.rope_loc[0].x = 0;
        part.rope_loc[0].y = 0;
    }
}

mod electric_engine {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 7552,
        mass: 1000,
        bounciness: 1024,
        friction: 16,

        flags1: 0x4800,
        flags3: 0x0008,
        size_something2: (56, 47),
        size:            (56, 47),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (4, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: None,
        flip_fn:   Some(flip),
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:0fa3
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 1068:0248
    fn reset(part: &mut Part) {
        // reset_c!(electric_engine_reset, part);
        unimplemented();
    }

    // TIMWIN: 1068:01ca
    fn run(part: &mut Part) {
        // run_c!(electric_engine_run, part);
        unimplemented();
    }

    // TIMWIN: 1068:02b1
    fn flip(part: &mut Part, _orientation: u16) {
        // flip_c!(electric_engine_flip, part, _orientation);
        unimplemented();
    }
}

mod vacuum {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 7552,
        mass: 1000,
        bounciness: 1024,
        friction: 16,

        flags1: 0x4800,
        flags3: 0x0008,
        size_something2: (50, 50),
        size:            (50, 50),
        resize_x: None,
        resize_y: None,

        render_images: Some(&[
            &[(4, 3, 0, 0), (4, 0, 17, 32), (4, 5, 7, 9)],
            &[(4, 6, 3, 7), (4, 3, 0, -4), (4, 4, 44, 31), (4, 1, 18, 28)],
            &[(4, 7, 0, 9), (4, 3, 0, 0), (4, 0, 17, 32), (4, 4, 47, 35)],
            &[(4, 8, -3, 11), (4, 3, 0, 1), (4, 2, 15, 34), (4, 4, 49, 37)],
            &[(4, 9, -3, 8), (4, 3, 0, 0), (4, 0, 17, 32), (4, 4, 47, 35)],
            &[(4, 9, -2, 5), (4, 3, 0, -4), (4, 4, 44, 31), (4, 1, 18, 28)],
            &[(4, 9, -3, 8), (4, 3, 0, 0), (4, 0, 17, 32), (4, 4, 47, 35)],
            &[(4, 9, -3, 9), (4, 3, 0, 1), (4, 2, 15, 34), (4, 4, 49, 37)],
            &[(4, 9, -3, 8), (4, 3, 0, 0), (4, 0, 17, 32), (4, 4, 47, 35)],
        ]),
        render_pos_offsets: Some(&[
            (0,0), (0,-4), (0,0), (-3,1), (-3,0), (-2,-4), (-3,0), (-3,1), (-3,0), 
        ]),
        explicit_sizes: Some(&[
            (50,50), (58,51), (62,50), (66,50), (64,50), (64,51), (64,50), (66,50), (64,50), 
        ]),
        goobers: (4, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: Some(bounce),
        flip_fn:   Some(flip),
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:0ff3
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 10d0:0af6
    fn reset(part: &mut Part) {
        // reset_c!(vacuum_reset, part);
        unimplemented();
    }

    // TIMWIN: 10d0:0b4c
    fn run(part: &mut Part) {
        // run_c!(vacuum_run, part);
        unimplemented();
    }

    // TIMWIN: 10d0:0a9a
    fn bounce(part: &mut Part) -> bool {
        // bounce_c!(vacuum_bounce, part);
        unimplemented();
    }

    // TIMWIN: 10d0:0d58
    fn flip(part: &mut Part, _orientation: u16) {
        // flip_c!(vacuum_flip, part, _orientation);
        unimplemented();
    }
}

mod cheese {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 1500,
        mass: 40,
        bounciness: 64,
        friction: 32,

        flags1: 0x0800,
        flags3: 0x0008,
        size_something2: (30, 18),
        size:            (30, 18),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (3, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    None,
        bounce_fn: None,
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:1044
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 1048:18e6
    fn reset(part: &mut Part) {
        // reset_c!(cheese_reset, part);
        unimplemented();
    }
}

mod nail {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 7552,
        mass: 20,
        bounciness: 1024,
        friction: 2,

        flags1: 0x4800,
        flags3: 0x0008,
        size_something2: (14, 17),
        size:            (14, 17),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (4, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    None,
        bounce_fn: Some(bounce),
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:1087
    fn create(part: &mut Part) {
        // nothing to do here
    }

    // TIMWIN: 1088:17ca
    fn reset(part: &mut Part) {
        part.set_border(&[(0,0), (12,0), (6,16)]);
    }

    // TIMWIN: 1088:178c
    fn bounce(part: &mut Part) -> bool {
        if part.part_type == PartType::Balloon.to_u16() {
            let angle = part.bounce_angle;

            if angle == 0x5e00 || angle == 0x8000 || angle == 0x9c90 {
                part.state2 = 1;
            }
        }

        true
    }
}

mod mel_schlemming {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 2400,
        mass: 40,
        bounciness: 128,
        friction: 64,

        flags1: 0x0800,
        flags3: 0x0008,
        size_something2: (14, 24),
        size:            (14, 24),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: Some(&[
            (0,0), (0,-1), (0,-1), (0,0), (0,0), (0,-1), (-2,-6), (-8,-6), (-7,-6), (-7,-3), (-10,14), (-14,17), (-8,9), (-3,3), (-1,1), (-1,0), (1,0), (-2,0), (1,0), (-1,0), (-3,0), (-2,0), (-1,0), (1,-7), (-1,-7), (1,-6), (-1,-6), (-2,0), (-8,2), (-8,9), (-11,16), 
        ]),
        explicit_sizes: Some(&[
            (16,24), (16,24), (16,24), (16,24), (16,24), (16,24), (24,30), (32,31), (24,30), (32,26), (40,24), (48,24), (32,24), (24,24), (16,24), (16,24), (16,24), (24,24), (24,24), (16,24), (24,24), (24,24), (16,24), (24,31), (24,31), (24,30), (16,30), (24,24), (24,24), (24,24), (24,24), 
        ]),
        goobers: (3, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: Some(bounce),
        flip_fn:   Some(flip),
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:10ca
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 1080:00c7
    fn reset(part: &mut Part) {
        // reset_c!(mel_schlemming_reset, part);
        unimplemented();
    }

    // TIMWIN: 1080:011c
    fn run(part: &mut Part) {
        // run_c!(mel_schlemming_run, part);
        unimplemented();
    }

    // TIMWIN: 1080:0000
    fn bounce(part: &mut Part) -> bool {
        // bounce_c!(mel_schlemming_bounce, part);
        unimplemented();
    }

    // TIMWIN: 1080:0220
    fn flip(part: &mut Part, _orientation: u16) {
        // flip_c!(mel_schlemming_flip, part, _orientation);
        unimplemented();
    }
}

mod title_the_even_more {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 100,
        mass: 140,
        bounciness: 64,
        friction: 32,

        flags1: 0x0800,
        flags3: 0x0008,
        size_something2: (96, 16),
        size:            (96, 16),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (4, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    None,
        bounce_fn: None,
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:1112
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 1058:1591
    fn reset(part: &mut Part) {
        let x1 = 0;
        let y1 = 0;
        let x2 = 0x52;
        let y2 = part.size.y as u8 - 1;

        if part.state1 != 1 {
            part.set_border(&[(x1, y1), (x2, y1), (x2, y2), (x1, y2)]);
        }

        part.rope_loc[0].x = part.size.x as u8 / 2;
        part.rope_loc[0].y = 0;
    }
}

mod title_incredible_machine {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 100,
        mass: 140,
        bounciness: 128,
        friction: 32,

        flags1: 0x4800,
        flags3: 0x0008,
        size_something2: (212, 16),
        size:            (212, 16),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (5, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    None,
        bounce_fn: None,
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:1170
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 1058:153c
    fn reset(part: &mut Part) {
        // reset_c!(title_incredible_machine_reset, part);
        unimplemented();
    }
}

mod title_credits {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 100,
        mass: 140,
        bounciness: 128,
        friction: 32,

        flags1: 0x0800,
        flags3: 0x0008,
        size_something2: (32, 16),
        size:            (32, 16),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (0, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: None,
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   Some(rope),
    };

    // TIMWIN: 1078:11b3
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 1058:1591
    fn reset(part: &mut Part) {
        let ((x1, y1), (x2, y2)) = if part.state1 == 8 {
            ((0, 10), (10, 0x69))
        } else if part.state1 == 0 {
            ((0x6e, 0), (0x6f, 1))
        } else {
            ((0, 0), (part.size.x as u8 - 1, part.size.y as u8 - 1))
        };

        part.set_border(&[(x1, y1), (x2, y1), (x2, y2), (x1, y2)]);
        part.rope_loc[0].x = part.size.x as u8 / 2;
        part.rope_loc[0].y = 0;
    }

    // TIMWIN: 1058:1646
    fn run(part: &mut Part) {
        // run_c!(title_credits_run, part);
        unimplemented();
    }

    // TIMWIN: 1058:1684
    fn rope(p1: &mut Part, p2: &mut Part, rope_slot: u8, flags: u16, p1_mass: i16, p1_force: i32) -> u8 {
        // rope_c!(title_credits_rope, p1, p2, rope_slot, flags, p1_mass, p1_force);
        unimplemented();
    }
}

mod mels_house {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 7552,
        mass: 4000,
        bounciness: 1024,
        friction: 16,

        flags1: 0x4800,
        flags3: 0x0000,
        size_something2: (48, 64),
        size:            (48, 64),
        resize_x: None,
        resize_y: None,

        render_images: Some(&[
            &[(5, 0, 0, 0)],
            &[(5, 0, 0, 0), (5, 1, 28, 39)],
            &[(5, 0, 0, 0), (5, 1, 28, 39), (5, 2, 8, 39)],
            &[(5, 0, 0, 0), (5, 1, 28, 39), (5, 3, 8, 39)],
            &[(5, 0, 0, 0), (5, 1, 28, 39), (5, 4, 8, 39)],
            &[(5, 0, 0, 0), (5, 1, 28, 39), (5, 5, 8, 39)],
            &[(5, 0, 0, 0), (5, 1, 28, 39), (5, 5, 8, 39), (5, 6, -12, -28)],
            &[(5, 0, 0, 0), (5, 1, 28, 39), (5, 5, 8, 39), (5, 7, -17, -30)],
            &[(5, 0, 0, 0), (5, 1, 28, 39), (5, 5, 8, 39), (5, 8, -18, -33)],
        ]),
        render_pos_offsets: Some(&[
            (0,0), (0,0), (0,0), (0,0), (0,0), (0,0), (-12,-28), (-17,-30), (-18,-33), 
        ]),
        explicit_sizes: Some(&[
            (48,64), (48,64), (48,64), (48,64), (48,64), (48,64), (60,92), (65,94), (66,97), 
        ]),
        goobers: (5, 255),

        create_fn: create,
        reset_fn:  None,
        run_fn:    Some(run),
        bounce_fn: None,
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:11fe
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 1080:1086
    fn run(part: &mut Part) {
        // run_c!(mels_house_run, part);
        unimplemented();
    }
}

mod super_ball {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 1800,
        mass: 14,
        bounciness: 512,
        friction: 16,

        flags1: 0x0800,
        flags3: 0x0008,
        size_something2: (24, 23),
        size:            (24, 23),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (3, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    None,
        bounce_fn: None,
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:121f
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 1048:0592
    fn reset(part: &mut Part) {
        // reset_c!(super_ball_reset, part);
        unimplemented();
    }
}

mod dirt_wall {
    use super::prelude::*;
    use super::wall_common::{create, reset, resize};
    pub const DEF: PartDef = PartDef {
        density: 7552,
        mass: 1000,
        bounciness: 1024,
        friction: 128,

        flags1: 0x4800,
        flags3: 0x0000,
        size_something2: (32, 16),
        size:            (32, 16),
        resize_x: Some((16, 240)),
        resize_y: Some((16, 240)),

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (4, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    None,
        bounce_fn: None,
        flip_fn:   None,
        resize_fn: Some(resize),
        rope_fn:   None,
    };
}

mod ernie_the_alligator {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 2400,
        mass: 40,
        bounciness: 1024,
        friction: 48,

        flags1: 0x4800,
        flags3: 0x0000,
        size_something2: (80, 16),
        size:            (80, 16),
        resize_x: None,
        resize_y: None,

        render_images: Some(&[
            &[(4, 0, 37, 0), (4, 1, 0, 0), (4, 10, 62, 0)],
            &[(4, 0, 37, 0), (4, 1, 0, 0), (4, 11, 62, 2)],
            &[(4, 0, 37, 0), (4, 1, 0, 0), (4, 12, 62, 2)],
            &[(4, 0, 37, 0), (4, 1, 0, 0), (4, 13, 62, 2)],
            &[(4, 0, 37, 0), (4, 1, 0, 0), (4, 14, 62, 1)],
            &[(4, 0, 37, 0), (4, 1, 0, 0), (4, 15, 62, 2)],
            &[(4, 0, 37, 0), (4, 1, 0, 0), (4, 10, 62, 0), (4, 9, 21, 0)],
            &[(4, 0, 37, 0), (4, 2, 2, -10), (4, 10, 62, 0)],
            &[(4, 0, 37, 0), (4, 3, 4, -17), (4, 10, 62, 0)],
            &[(4, 0, 37, 0), (4, 2, 2, -10), (4, 10, 62, 0)],
            &[(4, 0, 37, 0), (4, 4, 0, 0), (4, 10, 62, 0)],
            &[(4, 0, 37, 0), (4, 5, 0, 0), (4, 10, 62, 0)],
            &[(4, 0, 37, 0), (4, 6, 0, 0), (4, 10, 62, 0)],
            &[(4, 0, 37, 0), (4, 7, 0, 0), (4, 10, 62, 0)],
            &[(4, 0, 37, 0), (4, 8, 0, 0), (4, 10, 62, 0)],
        ]),
        render_pos_offsets: Some(&[
            (0,0), (0,0), (0,0), (0,0), (0,0), (0,0), (0,0), (0,-10), (0,-17), (0,-10), (0,0), (0,0), (0,0), (0,0), (0,0), 
        ]),
        explicit_sizes: Some(&[
            (80,16), (80,16), (80,16), (80,16), (80,16), (80,16), (80,16), (80,26), (80,33), (80,26), (80,16), (80,16), (80,16), (80,16), (80,16), 
        ]),
        goobers: (4, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: Some(bounce),
        flip_fn:   Some(flip),
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:1262
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 1048:0203
    fn reset(part: &mut Part) {
        // reset_c!(ernie_the_alligator_reset, part);
        unimplemented();
    }

    // TIMWIN: 1048:0259
    fn run(part: &mut Part) {
        // run_c!(ernie_the_alligator_run, part);
        unimplemented();
    }

    // TIMWIN: 1048:0000
    fn bounce(part: &mut Part) -> bool {
        // bounce_c!(ernie_the_alligator_bounce, part);
        unimplemented();
    }

    // TIMWIN: 1048:0384
    fn flip(part: &mut Part, _orientation: u16) {
        // flip_c!(ernie_the_alligator_flip, part, _orientation);
        unimplemented();
    }
}

mod teapot {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 2400,
        mass: 40,
        bounciness: 64,
        friction: 64,

        flags1: 0x0800,
        flags3: 0x0008,
        size_something2: (30, 22),
        size:            (30, 22),
        resize_x: None,
        resize_y: None,

        render_images: Some(&[
            &[(3, 0, 0, 0)],
            &[(3, 1, 2, -4)],
            &[(3, 2, 3, -13)],
            &[(3, 2, 3, -13)],
            &[(3, 2, 3, -13)],
            &[(3, 2, 3, -13)],
            &[(3, 4, -5, 0), (3, 6, -46, -34)],
            &[(3, 5, -6, 0), (3, 7, -42, -34)],
            &[(3, 4, -5, 0), (3, 8, -45, -35)],
            &[(3, 5, -6, 0), (3, 6, -45, -34)],
            &[(3, 4, -5, 0), (3, 7, -44, -34)],
            &[(3, 5, -6, 0), (3, 8, -44, -35)],
        ]),
        render_pos_offsets: Some(&[
            (0,0), (0,-4), (0,-13), (0,-13), (0,-13), (0,-13), (-46,-34), (-42,-34), (-45,-35), (-45,-34), (-44,-34), (-44,-35), 
        ]),
        explicit_sizes: Some(&[
            (30,22), (36,26), (42,35), (42,35), (42,35), (42,35), (82,56), (78,56), (81,57), (81,56), (80,56), (80,57), 
        ]),
        goobers: (3, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: None,
        flip_fn:   Some(flip),
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:12af
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 10b0:0000
    fn reset(part: &mut Part) {
        // reset_c!(teapot_reset, part);
        unimplemented();
    }

    // TIMWIN: 10b0:008d
    fn run(part: &mut Part) {
        // run_c!(teapot_run, part);
        unimplemented();
    }

    // TIMWIN: 10b0:0048
    fn flip(part: &mut Part, _orientation: u16) {
        // flip_c!(teapot_flip, part, _orientation);
        unimplemented();
    }
}

mod eightball {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 2400,
        mass: 40,
        bounciness: 256,
        friction: 0,

        flags1: 0x0800,
        flags3: 0x0008,
        size_something2: (24, 23),
        size:            (24, 23),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (3, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    None,
        bounce_fn: None,
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:0e5d
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 1048:0592
    fn reset(part: &mut Part) {
        // reset_c!(eightball_reset, part);
        unimplemented();
    }
}

mod pinball_bumper {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 2400,
        mass: 40,
        bounciness: 256,
        friction: 16,

        flags1: 0x4800,
        flags3: 0x0008,
        size_something2: (40, 40),
        size:            (40, 40),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (4, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    Some(run),
        bounce_fn: Some(bounce),
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:12fc
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 1048:1084
    fn reset(part: &mut Part) {
        // reset_c!(pinball_bumper_reset, part);
        unimplemented();
    }

    // TIMWIN: 1048:10cc
    fn run(part: &mut Part) {
        // run_c!(pinball_bumper_run, part);
        unimplemented();
    }

    // TIMWIN: 1048:0fbc
    fn bounce(part: &mut Part) -> bool {
        // bounce_c!(pinball_bumper_bounce, part);
        unimplemented();
    }
}

mod lucky_clover {
    use super::prelude::*;
    pub const DEF: PartDef = PartDef {
        density: 2000,
        mass: 800,
        bounciness: 128,
        friction: 64,

        flags1: 0x0800,
        flags3: 0x0008,
        size_something2: (32, 32),
        size:            (32, 32),
        resize_x: None,
        resize_y: None,

        render_images: None,
        render_pos_offsets: None,
        explicit_sizes: None,
        goobers: (3, 255),

        create_fn: create,
        reset_fn:  Some(reset),
        run_fn:    None,
        bounce_fn: None,
        flip_fn:   None,
        resize_fn: None,
        rope_fn:   None,
    };

    // TIMWIN: 1078:133f
    fn create(part: &mut Part) {
        unimplemented();
    }

    // TIMWIN: 10a0:0c86
    fn reset(part: &mut Part) {
        // reset_c!(lucky_clover_reset, part);
        unimplemented();
    }
}

use crate::part::PartType;

pub fn parts_bin_order() -> &'static [PartType] {
    use crate::part::PartType::*;
    // Used to order what's in the parts bin
    const PARTS_BIN_ORDER: &[PartType] = &[
        BowlingBall,
        Basketball,
        CannonBall,
        Baseball,
        TennisBall,
        Balloon,
        TeeterTotter,
        Bellows,
        BoxingGlove,
        Trampoline,
        Belt,
        Gear,
        Conveyor,
        JackInTheBox,
        Windmill,
        Rope,
        EyeHook,
        Pulley,
        Gun,
        Scissors,
        LightSwitchOutlet,
        Generator,
        SolarPanels,
        Fan,
        ElectricEngine,
        MagnifyingGlass,
        Flashlight,
        Lightbulb,
        Cannon,
        Dynamite,
        Rocket,
        Candle,
        DynamiteWithPlunger,
        Bucket,
        Cage,
        PokeyTheCat,
        MortTheMouse,
        MortTheMouseCage,
        BobTheFish,
        KellyTheMonkey,
        BrickWall,
        PipeStraight,
        PipeCurved,
        WoodWall,
        Incline,
        GunBullet,
        Explosion,
        RopeSeveredEnd,
        Vacuum,
        TitleTheEvenMore,
        TitleIncredibleMachine,
        TitleCredits,
        Cheese,
        Nail,
        MelSchlemming,
        MelsHouse,
        SuperBall,
        DirtWall,
        ErnieTheAlligator,
        Teapot,
        Eightball,
        PinballBumper,
        JackOLantern,
        HeartBalloon,
        ChristmasTree,
        LuckyClover
    ];

    PARTS_BIN_ORDER
}

pub fn get_def(part_type: PartType) -> &'static PartDef {
    use crate::part::PartType::*;

    match part_type {
        BowlingBall            => &bowling_ball::DEF,
        BrickWall              => &brick_wall::DEF,
        Incline                => &incline::DEF,
        TeeterTotter           => &teeter_totter::DEF,
        Balloon                => &balloon::DEF,
        Conveyor               => &conveyor::DEF,
        MortTheMouseCage       => &mort_the_mouse_cage::DEF,
        Pulley                 => &pulley::DEF,
        Belt                   => &belt::DEF,
        Basketball             => &basketball::DEF,
        Rope                   => &rope::DEF,
        Cage                   => &cage::DEF,
        PokeyTheCat            => &pokey_the_cat::DEF,
        JackInTheBox           => &jack_in_the_box::DEF,
        Gear                   => &gear::DEF,
        BobTheFish             => &bob_the_fish::DEF,
        Bellows                => &bellows::DEF,
        Bucket                 => &bucket::DEF,
        Cannon                 => &cannon::DEF,
        Dynamite               => &dynamite::DEF,
        GunBullet              => &gun_bullet::DEF,
        LightSwitchOutlet      => &light_switch_outlet::DEF,
        DynamiteWithPlunger    => &dynamite_with_plunger::DEF,
        EyeHook                => &eye_hook::DEF,
        Fan                    => &fan::DEF,
        Flashlight             => &flashlight::DEF,
        Generator              => &generator::DEF,
        Gun                    => &gun::DEF,
        Baseball               => &baseball::DEF,
        Lightbulb              => &lightbulb::DEF,
        MagnifyingGlass        => &magnifying_glass::DEF,
        KellyTheMonkey         => &kelly_the_monkey::DEF,
        JackOLantern           => &jack_o_lantern::DEF,
        HeartBalloon           => &heart_balloon::DEF,
        ChristmasTree          => &christmas_tree::DEF,
        BoxingGlove            => &boxing_glove::DEF,
        Rocket                 => &rocket::DEF,
        Scissors               => &scissors::DEF,
        SolarPanels            => &solar_panels::DEF,
        Trampoline             => &trampoline::DEF,
        Windmill               => &windmill::DEF,
        Explosion              => &explosion::DEF,
        MortTheMouse           => &mort_the_mouse::DEF,
        CannonBall             => &cannon_ball::DEF,
        TennisBall             => &tennis_ball::DEF,
        Candle                 => &candle::DEF,
        PipeStraight           => &pipe_straight::DEF,
        PipeCurved             => &pipe_curved::DEF,
        WoodWall               => &wood_wall::DEF,
        RopeSeveredEnd         => &rope_severed_end::DEF,
        ElectricEngine         => &electric_engine::DEF,
        Vacuum                 => &vacuum::DEF,
        Cheese                 => &cheese::DEF,
        Nail                   => &nail::DEF,
        MelSchlemming          => &mel_schlemming::DEF,
        TitleTheEvenMore       => &title_the_even_more::DEF,
        TitleIncredibleMachine => &title_incredible_machine::DEF,
        TitleCredits           => &title_credits::DEF,
        MelsHouse              => &mels_house::DEF,
        SuperBall              => &super_ball::DEF,
        DirtWall               => &dirt_wall::DEF,
        ErnieTheAlligator      => &ernie_the_alligator::DEF,
        Teapot                 => &teapot::DEF,
        Eightball              => &eightball::DEF,
        PinballBumper          => &pinball_bumper::DEF,
        LuckyClover            => &lucky_clover::DEF,
    }
}
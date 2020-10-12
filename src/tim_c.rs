// Rust/C interop layer

use std::os::raw::{c_int, c_char};
use crate::part::PartType;
use crate::atmosphere;

/**** Import C declarations to Rust ****/
extern {
    pub fn debug_part_size() -> usize;
    pub fn initialize_llamas();
    pub fn part_new(part_type: c_int) -> *mut Part;
    pub fn part_alloc() -> *mut Part;
    pub fn belt_data_alloc() -> *mut BeltData;
    pub fn rope_data_alloc() -> *mut RopeData;
    pub fn part_free(part: *mut Part);
    pub fn part_alloc_borders_and_reset(part: *mut Part) -> c_int;
    pub fn restore_parts_state_from_design();
    pub fn advance_parts();
    pub fn all_parts_set_prev_vars();
    pub fn insert_part_into_static_parts(part: *mut Part);
    pub fn insert_part_into_moving_parts(part: *mut Part);
    pub fn insert_part_into_parts_bin(part: *mut Part);
    pub fn part_resize(part: *mut Part);
    pub fn part_flip(part: *mut Part, flag: c_int);
    pub fn part_density(part_type: c_int) -> u16;
    pub fn calculate_rope_sag(part: *const Part, rope_data: *const RopeData, time: c_int) -> i16;

    pub static mut GRAVITY: u16;
    pub static mut AIR_PRESSURE: u16;

    pub static mut STATIC_PARTS_ROOT: Part;
    pub static mut MOVING_PARTS_ROOT: Part;
    pub static mut PARTS_BIN_ROOT: Part;
}

#[derive(Clone)]
pub struct PartsIterator<'a> {
    cur: *const Part,
    _phantom: std::marker::PhantomData<&'a Part>
}
impl<'a> PartsIterator<'a> {
    /// Initializes the iterator with a pointer to the first Part.
    /// Unsafe because 1) the pointer could be invalid, and 2) the parts could change during iteration.
    pub unsafe fn new(ptr: *const Part) -> Self {
        PartsIterator {
            cur: ptr,
            _phantom: std::marker::PhantomData
        }
    }
}
impl<'a> Iterator for PartsIterator<'a> {
    type Item = &'a Part;

    fn next(&mut self) -> Option<&'a Part> {
        let r = unsafe { self.cur.as_ref() };

        if let Some(part) = r {
            self.cur = part.next;
        }

        r
    }
}

/// Returns an iterator of static parts.
/// Unsafe because the parts could change during iteration.
pub unsafe fn static_parts_iter<'a>() -> PartsIterator<'a> {
    PartsIterator::new(STATIC_PARTS_ROOT.next)
}

/// Returns an iterator of moving parts.
/// Unsafe because the parts could change during iteration.
pub unsafe fn moving_parts_iter<'a>() -> PartsIterator<'a> {
    PartsIterator::new(MOVING_PARTS_ROOT.next)
}

pub fn print_parts() {
    {
        let iter = unsafe { static_parts_iter() };
        println!("Total static parts: {}", iter.clone().count());
        for part in iter {
            println!("{:?}", part);
        }
    }
    {
        let iter = unsafe { moving_parts_iter() };
        println!("Total moving parts: {}", iter.clone().count());
        for part in iter {
            println!("{:?}", part);
        }
    }
}

#[repr(C)]
#[derive(Copy, Clone, Debug)]
pub struct ByteVec {
    pub x: u8,
    pub y: u8,
}

#[repr(C)]
#[derive(Copy, Clone, Debug)]
pub struct ShortVec {
    pub x: i16,
    pub y: i16,
}

#[repr(C)]
#[derive(Copy, Clone, Debug)]
pub struct BorderPoint {
    pub x: u8,
    pub y: u8,
    pub normal_angle: u16,
}

include!("./generated/structs.rs");

impl Part {
    pub fn new_zero() -> Self {
        unsafe { std::mem::zeroed() }
    }
    pub fn border_points(&self) -> &[BorderPoint] {
        unsafe {
            let size = self.num_borders as usize;
            if size == 0 || self.borders_data.is_null() {
                std::slice::from_raw_parts(std::ptr::NonNull::dangling().as_ptr(), 0)
            } else {
                std::slice::from_raw_parts(self.borders_data, size)
            }
        }
    }

    /// Returns a list of tuples: ((x1, y1), (x2, y2), sag)
    pub fn rope_sections(&self) -> Option<Vec<((i16, i16), (i16, i16), i16)>> {
        if self.part_type != PartType::Rope.to_u16() { return None; }

        let mut sections = vec![];

        let rope = unsafe { self.rope_data[0].as_ref().unwrap() };
        let mut curpart_raw = rope.part1;
        let mut nextpart_raw = unsafe { curpart_raw.as_ref().unwrap() }.links_to[rope.part1_rope_slot as usize];
        if nextpart_raw.is_null() {
            nextpart_raw = rope.part2;
        }

        while !curpart_raw.is_null() && !nextpart_raw.is_null() {
            let curpart = unsafe { curpart_raw.as_ref().unwrap() };
            let nextpart = unsafe { nextpart_raw.as_ref().unwrap() };

            let pos1: ShortVec;
            if curpart.part_type == PartType::Pulley.to_u16() {
                let rpd = unsafe { curpart.rope_data[0].as_ref().unwrap() };
                pos1 = rpd.ends_pos[1];
            } else {
                pos1 = rope.ends_pos[0];
            }

            let pos2: ShortVec;
            if nextpart.part_type == PartType::Pulley.to_u16() {
                let rpd = unsafe { nextpart.rope_data[0].as_ref().unwrap() };
                pos2 = rpd.ends_pos[0];
            } else {
                pos2 = rope.ends_pos[1];
            }

            // DRAW ROPE HERE

            let sag: i16;
            if curpart.part_type == PartType::Pulley.to_u16() && nextpart.part_type == PartType::Pulley.to_u16() {
                sag = 0;
            } else {
                sag = unsafe { calculate_rope_sag(curpart, rope, 3) };
            }

            sections.push(( (pos1.x, pos1.y), (pos2.x, pos2.y), sag ));
            if sections.len() > 256 {
                // we're probably doing something wrong
                // might have ended up with a cycle somehow
                panic!("too many sections!");
            }

            curpart_raw = nextpart_raw;
            if nextpart.part_type == PartType::Pulley.to_u16() {
                nextpart_raw = nextpart.links_to[0];
            } else {
                nextpart_raw = std::ptr::null_mut();
            }
        }

        Some(sections)
    }
}

impl BeltData {
    pub fn new_zero() -> Self {
        unsafe { std::mem::zeroed() }
    }
}

impl RopeData {
    pub fn new_zero() -> Self {
        unsafe { std::mem::zeroed() }
    }
}

#[no_mangle]
pub extern fn unimplemented() {
    panic!("Unimplemented");
}

#[no_mangle]
pub extern fn output_c(c_str: *const c_char) {
    use std::ffi::CStr;
    let c_str = unsafe { CStr::from_ptr(c_str) };
    if let Ok(s) = c_str.to_str() {
        println!("Output: {}", s);
    } else {
        println!("Error handling string");
    }
}

#[no_mangle]
pub extern fn output_part_c(ptr: *const Part) {
    let part = unsafe { ptr.as_ref() };
    if let Some(part) = part {
        println!("Output: {:?}", part);
    } else {
        println!("Output: null part");
    }
}

#[no_mangle]
pub extern fn output_int_c(v: i64) {
    println!("Output: {}", v);
}

/// TIMWIN: 10a8:4d2d
#[no_mangle]
pub extern fn play_sound(id: c_int) {
    println!("Play sound: {}", id);
}

/**** Export math functions to C ****/
use crate::math;

#[no_mangle]
pub extern fn arctan_c(dx: i32, dy: i32) -> u16 {
    math::arctan(dx, dy)
}

#[no_mangle]
pub extern fn sine_c(angle: u16) -> i16 {
    math::sine(angle)
}

#[no_mangle]
pub extern fn cosine_c(angle: u16) -> i16 {
    math::cosine(angle)
}

#[no_mangle]
pub extern fn rotate_point_c(x: &mut i16, y: &mut i16, angle: u16) {
    let (nx, ny) = math::rotate_point(*x, *y, angle);
    *x = nx;
    *y = ny;
}

// bool calculate_line_intersection(const struct Line *a, const struct Line *b, struct ShortVec *out);

#[repr(C)]
#[derive(Debug)]
pub struct Line {
    p0: ShortVec,
    p1: ShortVec,
}

#[no_mangle]
pub extern fn calculate_line_intersection(a: *const Line, b: *const Line, out: *mut ShortVec) -> c_int {
    let a = unsafe { a.as_ref().unwrap() };
    let b = unsafe { b.as_ref().unwrap() };
    let (intersects, o) = math::line_intersection(((a.p0.x, a.p0.y), (a.p1.x, a.p1.y)),
                                                  ((b.p0.x, b.p0.y), (b.p1.x, b.p1.y)));
    
    {
        if let Some(out) = unsafe { out.as_mut() } {
            out.x = o.0;
            out.y = o.1;
        }
    }

    if intersects { 1 } else { 0 }
}

#[no_mangle]
pub extern fn calculate_line_intersection_helper(a: i16, b: i16, c: i16) -> c_int {
    let intersects = math::line_intersection_helper(a, b, c);

    if intersects { 1 } else { 0 }
}

static mut PART_IMAGE_SIZES: Vec<(i16, i16)> = vec![];

// Returns true if a valid image size was found.
// Returns false otherwise. size_out is unchanged in this case.
#[no_mangle]
pub extern fn part_image_size(part_type: c_int, index: u16, out: *mut ShortVec) -> c_int {
    // relies on global variables for now, because the original game did.

    // In TIMWIN, this value comes from (pseudocode):
    // width  = data31[part_type].field_0x14[state].field_0x04 (16-bit signed)
    // height = data31[part_type].field_0x14[state].field_0x06 (16-bit signed)

    // some hard-coded part sizes until we implement loading them from the resource files
    let t = match PartType::from_u16(part_type as u16) {
        PartType::BowlingBall => Some((32, 32)),
        PartType::BrickWall => Some((16, 16)),
        PartType::Incline => match index {
            0 => Some((16, 32)),
            1 => Some((32, 32)),
            2 => Some((48, 32)),
            3 => Some((64, 32)),
            _ => None
        },
        PartType::TeeterTotter => match index {
            0 => Some((80, 36)),
            1 => Some((80, 23)),
            2 => Some((80, 36)),
            _ => None
        },
        PartType::Balloon => match index {
            0 => Some((32, 48)),
            1 => Some((72, 71)),
            2 => Some((80, 67)),
            3 => Some((96, 61)),
            4 => Some((88, 54)),
            5 => Some((88, 46)),
            6 => Some((88, 50)),
            _ => None
        },
        PartType::Pulley => Some((16, 16)),
        PartType::PokeyTheCat => match index {
            0 => Some((40, 41)),
            1 => Some((72, 57)),
            2 => Some((56, 42)),
            3 => Some((48, 41)),
            4 => Some((56, 41)),
            5 => Some((56, 41)),
            6 => Some((56, 43)),
            7 => Some((56, 43)),
            8 => Some((56, 43)),
            9 => Some((56, 44)),
            _ => None
        },
        PartType::RopeSeveredEnd => None,
        PartType::Nail => Some((16, 17)),
        _ => {
            println!("Unimplemented part_image_size: {}", part_type);
            None
        }
    };

    if let Some((width, height)) = t {
        let out = unsafe { out.as_mut().unwrap() };
        out.x = width;
        out.y = height;
        return 1;
    }

    return 0;
}

/// Partial from TIMWIN: 1090:0000
/// Was pre-calculated in TIM each time the air pressure or gravity changed. Here we recalculate it each time.
/// We can possibly memoize this call in the future if performance calls for it.
#[no_mangle]
pub fn part_acceleration(part_type: c_int) -> i16 {
    match PartType::from_u16(part_type as u16) {
        PartType::GunBullet => 0,
        PartType::Eightball => 0,
        _ => atmosphere::calculate_acceleration(unsafe { GRAVITY }, unsafe { AIR_PRESSURE }, unsafe { part_density(part_type) })
    }
}

/// Partial from TIMWIN: 1090:0000
/// Was pre-calculated in TIM each time the air pressure or gravity changed. Here we recalculate it each time.
/// We can possibly memoize this call in the future if performance calls for it.
#[no_mangle]
pub fn part_terminal_velocity(part_type: c_int) -> i16 {
    match PartType::from_u16(part_type as u16) {
        PartType::GunBullet => 0x3000,
        PartType::CannonBall => 0x3000,
        _ => atmosphere::calculate_terminal_velocity(unsafe { AIR_PRESSURE })
    }
}
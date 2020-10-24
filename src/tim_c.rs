// Rust/C interop layer

use std::os::raw::{c_int, c_char};
use crate::part::PartType;
use crate::atmosphere;
use crate::parts;

/**** Import C declarations to Rust ****/
extern {
    pub fn debug_part_size() -> usize;
    pub fn initialize_llamas();
    pub fn part_new(part_type: c_int) -> *mut Part;
    pub fn part_alloc() -> *mut Part;
    pub fn belt_data_alloc() -> *mut BeltData;
    pub fn rope_data_alloc() -> *mut RopeData;
    pub fn part_free(part: *mut Part);
    pub fn part_init_rope_data_primary(part: *mut Part);
    pub fn part_init_belt_data(part: *mut Part);
    pub fn part_alloc_borders(part: *mut Part, length: u16);
    pub fn part_calculate_border_normals(part: *mut Part);
    pub fn part_set_size_and_pos_render(part: *mut Part);
    pub fn part_clamp_to_terminal_velocity(part: *mut Part);
    pub fn restore_parts_state_from_design();
    pub fn advance_parts();
    pub fn all_parts_set_prev_vars();
    pub fn insert_part_into_static_parts(part: *mut Part);
    pub fn insert_part_into_moving_parts(part: *mut Part);
    pub fn insert_part_into_parts_bin(part: *mut Part);
    pub fn calculate_rope_sag(part: *const Part, rope_data: *const RopeData, time: c_int) -> i16;

    pub fn stub_10a8_21cb(part: *mut Part, c: u8);
    pub fn stub_10a8_2b6d(part: *mut Part, c: c_int);
    pub fn stub_10a8_280a(part: *mut Part, c: c_int);
    pub fn search_for_interactions(part: *mut Part, choice: c_int, search_x_min: i16, search_x_max: i16, search_y_min: i16, search_y_max: i16);

    pub static mut GRAVITY: u16;
    pub static mut AIR_PRESSURE: u16;

    pub static mut STATIC_PARTS_ROOT: Part;
    pub static mut MOVING_PARTS_ROOT: Part;
    pub static mut PARTS_BIN_ROOT: Part;

    pub static mut RESIZE_GOPHER: u16;
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

#[derive(Clone)]
pub struct PartsIteratorMut<'a> {
    cur: *mut Part,
    _phantom: std::marker::PhantomData<&'a mut Part>
}
impl<'a> PartsIteratorMut<'a> {
    /// Initializes the iterator with a pointer to the first Part.
    /// Unsafe because 1) the pointer could be invalid, and 2) the parts could change during iteration.
    pub unsafe fn new(ptr: *mut Part) -> Self {
        PartsIteratorMut {
            cur: ptr,
            _phantom: std::marker::PhantomData
        }
    }
}
impl<'a> Iterator for PartsIteratorMut<'a> {
    type Item = *mut Part;

    fn next(&mut self) -> Option<*mut Part> {
        let ptr = self.cur;
        if let Some(part) = unsafe { self.cur.as_ref() } {
            self.cur = part.next;
            Some(ptr)
        } else {
            None
        }
    }
}

/// Returns an iterator of static parts.
/// Unsafe because the parts could change during iteration.
pub unsafe fn static_parts_iter_mut<'a>() -> PartsIteratorMut<'a> {
    PartsIteratorMut::new(STATIC_PARTS_ROOT.next)
}

/// Returns an iterator of moving parts.
/// Unsafe because the parts could change during iteration.
pub unsafe fn moving_parts_iter_mut<'a>() -> PartsIteratorMut<'a> {
    PartsIteratorMut::new(MOVING_PARTS_ROOT.next)
}

#[derive(Clone)]
pub struct PartInteractionsIteratorMut {
    cur: *mut Part
}
impl<'a> PartInteractionsIteratorMut {
    /// Initializes the iterator with a pointer to the first Part.
    /// Unsafe because 1) the pointer could be invalid, and 2) the parts could change during iteration.
    pub unsafe fn new(ptr: *mut Part) -> Self {
        PartInteractionsIteratorMut {
            cur: ptr
        }
    }
}
impl Iterator for PartInteractionsIteratorMut {
    type Item = *mut Part;

    fn next(&mut self) -> Option<*mut Part> {
        let ptr = self.cur;
        if let Some(part) = unsafe { self.cur.as_ref() } {
            self.cur = part.interactions;
            Some(ptr)
        } else {
            None
        }
    }
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
pub struct SByteVec {
    pub x: i8,
    pub y: i8,
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

use std::ops::{Deref, DerefMut};

pub struct RopeDataRefMut<'a> {
    ptr: &'a mut RopeData
}

impl<'a> Deref for RopeDataRefMut<'a> {
    type Target = RopeData;

    fn deref(&self) -> &RopeData {
        self.ptr
    }
}

impl<'a> DerefMut for RopeDataRefMut<'a> {
    fn deref_mut(&mut self) -> &mut RopeData {
        self.ptr
    }
}

pub struct BeltDataRefMut<'a> {
    ptr: &'a mut BeltData
}

impl<'a> Deref for BeltDataRefMut<'a> {
    type Target = BeltData;

    fn deref(&self) -> &BeltData {
        self.ptr
    }
}

impl<'a> DerefMut for BeltDataRefMut<'a> {
    fn deref_mut(&mut self) -> &mut BeltData {
        self.ptr
    }
}

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

    pub fn bounce_part(&self) -> Option<&Part> {
        unsafe { self.bounce_part.as_ref() }
    }

    pub fn bounce_part_mut(&mut self) -> Option<&mut Part> {
        unsafe { self.bounce_part.as_mut() }
    }

    pub fn border_points_mut(&mut self) -> &mut [BorderPoint] {
        unsafe {
            let size = self.num_borders as usize;
            if size == 0 || self.borders_data.is_null() {
                std::slice::from_raw_parts_mut(std::ptr::NonNull::dangling().as_ptr(), 0)
            } else {
                std::slice::from_raw_parts_mut(self.borders_data, size)
            }
        }
    }

    // Allocates the borders to the part, and recalculates the normals
    pub fn set_border(&mut self, points: &[(u8, u8)]) {
        unsafe {
            part_alloc_borders(self, points.len() as u16);

            let b = self.border_points_mut();

            for (i, &(x, y)) in points.iter().enumerate() {
                b[i].x = x;
                b[i].y = y;
            }

            part_calculate_border_normals(self);
        }
    }

    /// Sets borders in an already-allocated buffer. Does NOT update border normals.
    /// Known quirk for parts like Bob the Fish.
    pub fn update_border_ignore_normals_quirk(&mut self, points: &[(u8, u8)]) {
        unsafe {
            if points.len() > self.num_borders as usize {
                panic!("Cannot update borders");
            }

            self.num_borders = points.len() as u16;

            let b = self.border_points_mut();

            for (i, &(x, y)) in points.iter().enumerate() {
                b[i].x = x;
                b[i].y = y;
            }
        }
    }

    pub fn init_rope_data_primary(&mut self) {
        unsafe {
            part_init_rope_data_primary(self);
        }
    }

    pub fn init_belt_data(&mut self) {
        unsafe {
            part_init_belt_data(self);
        }
    }

    pub fn rope_mut(&mut self, rope_slot: usize) -> Option<RopeDataRefMut> {
        if let Some(rope) = unsafe { self.rope_data[rope_slot].as_mut() } {
            Some(RopeDataRefMut {
                ptr: rope
            })
        } else {
            None
        }
    }

    pub fn belt_mut(&mut self) -> Option<BeltDataRefMut> {
        if let Some(belt) = unsafe { self.belt_data.as_mut() } {
            Some(BeltDataRefMut {
                ptr: belt
            })
        } else {
            None
        }
    }

    pub unsafe fn interactions_iter(&self) -> PartInteractionsIteratorMut {
        PartInteractionsIteratorMut::new(self.interactions)
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

    pub fn belt_section(&self) -> Option<((i16, i16, i16), (i16, i16, i16))> {
        if self.part_type != PartType::Belt.to_u16() { return None; }

        let belt = unsafe { self.belt_data.as_ref().unwrap() };
        let part1 = unsafe { belt.part1.as_ref().unwrap() };
        let part2 = unsafe { belt.part2.as_ref().unwrap() };

        Some((((part1.pos_render.x + part1.belt_loc.x as i16), (part1.pos_render.y + part1.belt_loc.y as i16), (part1.belt_width as i16)),
              ((part2.pos_render.x + part2.belt_loc.x as i16), (part2.pos_render.y + part2.belt_loc.y as i16), (part2.belt_width as i16))))
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
        PartType::Conveyor => Some((32 + (index as i16 / 7)*16, 16)),
        PartType::MortTheMouseCage => Some((48, 32)),
        PartType::Pulley => Some((16, 16)),
        PartType::Basketball => Some((32, 32)),
        PartType::Cage => Some((48, 64)),
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
        PartType::Gear => Some((40, 35)),
        PartType::Bucket => Some((40, 48)),
        PartType::EyeHook => Some((16, 16)),
        PartType::Baseball => Some((16, 15)),
        PartType::RopeSeveredEnd => None,
        PartType::Nail => Some((16, 17)),
        _ => {
            println!("Unimplemented part_image_size: {:?}", PartType::from_u16(part_type as u16));
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

#[no_mangle]
pub fn part_density(part_type: c_int) -> u16 {
    let t = PartType::from_u16(part_type as u16);
    parts::get_def(t).density
}

#[no_mangle]
pub fn part_mass(part_type: c_int) -> u16 {
    let t = PartType::from_u16(part_type as u16);
    parts::get_def(t).mass
}

#[no_mangle]
pub fn part_bounciness(part_type: c_int) -> i16 {
    let t = PartType::from_u16(part_type as u16);
    parts::get_def(t).bounciness
}

#[no_mangle]
pub fn part_friction(part_type: c_int) -> i16 {
    let t = PartType::from_u16(part_type as u16);
    parts::get_def(t).friction
}

#[no_mangle]
pub fn part_order(part_type: c_int) -> u16 {
    let t = PartType::from_u16(part_type as u16);
    let list = parts::parts_bin_order();

    list.iter().position(|&x| x == t).unwrap() as u16
}

#[no_mangle]
pub fn part_data30_flags1(part_type: c_int) -> u16 {
    let t = PartType::from_u16(part_type as u16);
    parts::get_def(t).flags1
}

#[no_mangle]
pub fn part_data30_flags3(part_type: c_int) -> u16 {
    let t = PartType::from_u16(part_type as u16);
    parts::get_def(t).flags3
}

#[no_mangle]
pub fn part_data30_size_something2(part_type: c_int) -> ShortVec {
    let t = PartType::from_u16(part_type as u16);
    let (w, h) = parts::get_def(t).size_something2;
    ShortVec { x: w as i16, y: h as i16 }
}

#[no_mangle]
pub fn part_data30_size(part_type: c_int) -> ShortVec {
    let t = PartType::from_u16(part_type as u16);
    let (w, h) = parts::get_def(t).size;
    ShortVec { x: w as i16, y: h as i16 }
}

#[no_mangle]
pub fn part_data31_render_pos_offset(part_type: c_int, state1: u16, out: &mut SByteVec) -> c_int {
    let t = PartType::from_u16(part_type as u16);
    if let Some(offsets) = parts::get_def(t).render_pos_offsets {
        let (x, y) = offsets[state1 as usize];
        out.x = x;
        out.y = y;
        1
    } else {
        0
    }
}

#[no_mangle]
pub fn part_explicit_size(part_type: c_int, index: u16, out: &mut ShortVec) -> c_int {
    let t = PartType::from_u16(part_type as u16);
    if let Some(sizes) = parts::get_def(t).explicit_sizes {
        let (w, h) = sizes[index as usize];
        out.x = w;
        out.y = h;
        1
    } else {
        0
    }
}

#[no_mangle]
pub fn part_run(part: &mut Part) {
    let t = PartType::from_u16(part.part_type as u16);
    if let Some(run) = parts::get_def(t).run_fn {
        run(part);
    }
}

#[no_mangle]
pub fn part_reset(part: &mut Part) {
    let t = PartType::from_u16(part.part_type as u16);
    if let Some(reset) = parts::get_def(t).reset_fn {
        reset(part);
    }
}

#[no_mangle]
pub fn part_bounce(part_type: c_int, part: &mut Part) -> c_int {
    let t = PartType::from_u16(part_type as u16);
    if let Some(bounce) = parts::get_def(t).bounce_fn {
        if bounce(part) {
            1
        } else {
            0
        }
    } else {
        // Default
        1
    }
}

#[no_mangle]
pub fn part_flip(part: &mut Part, orientation: c_int) {
    let t = PartType::from_u16(part.part_type as u16);
    if let Some(flip) = parts::get_def(t).flip_fn {
        flip(part, orientation as u16);
    }
}

#[no_mangle]
pub fn part_resize(part: &mut Part) {
    let t = PartType::from_u16(part.part_type as u16);
    if let Some(resize) = parts::get_def(t).resize_fn {
        resize(part);
    }
}

#[no_mangle]
pub fn part_rope(part_type: c_int, p1: &mut Part, p2: &mut Part, rope_slot: c_int, flags: u16, p1_mass: i16, p1_force: i32) -> c_int {
    let t = PartType::from_u16(part_type as u16);
    if let Some(rope) = parts::get_def(t).rope_fn {
        rope(p1, p2, rope_slot as u8, flags, p1_mass, p1_force) as c_int
    } else {
        // Default
        0
    }
}

#[no_mangle]
pub fn part_create_func(part_type: c_int, part: &mut Part) -> c_int {
    let t = PartType::from_u16(part_type as u16);
    let create = parts::get_def(t).create_fn;

    create(part);
    if let Some(reset) = parts::get_def(t).reset_fn {
        reset(part);
    }

    0
}

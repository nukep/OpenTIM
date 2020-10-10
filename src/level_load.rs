use crate::tim_c;
use crate::level_file_format;

pub fn level_load(level: &level_file_format::Level) {
    unsafe {
        tim_c::AIR_PRESSURE = level.air_pressure;
        tim_c::GRAVITY = level.gravity;
    }

    // Pre-allocating all parts, because the part-loading assigns pointers to other parts that exist before and after itself.
    let parts_count = level.static_parts.len() + level.moving_parts.len() + level.bin_parts.as_ref().map(|v| v.len()).unwrap_or(0);
    let mut parts_list: Vec<*mut tim_c::Part> = Vec::with_capacity(parts_count);
    for _ in 0..parts_count {
        parts_list.push(unsafe { tim_c::part_alloc() });
    }

    let mut index = 0;

    let part_ptr_from_index = |index: u16| {
        if index == 0xFFFF {
            std::ptr::null_mut()
        } else {
            parts_list[index as usize]
        }
    };

    for p in level.static_parts.iter() {
        let part_raw = parts_list[index];
        index += 1;

        {
            let part: &mut tim_c::Part = unsafe { part_raw.as_mut().unwrap() };
            part_load(p, part, &part_ptr_from_index);
        }

        unsafe { tim_c::insert_part_into_static_parts(part_raw); }
    }

    for p in level.moving_parts.iter() {
        let part_raw = parts_list[index];
        index += 1;

        {
            let part: &mut tim_c::Part = unsafe { part_raw.as_mut().unwrap() };
            part_load(p, part, &part_ptr_from_index);
        }

        unsafe { tim_c::insert_part_into_moving_parts(part_raw); }
    }

    for p in level.bin_parts.iter().flat_map(|x| x) {
        let part_raw = parts_list[index];
        index += 1;

        {
            let part: &mut tim_c::Part = unsafe { part_raw.as_mut().unwrap() };
            part_load(p, part, &part_ptr_from_index);
        }

        unsafe { tim_c::insert_part_into_parts_bin(part_raw); }
    }
}

fn part_load<F>(p: &level_file_format::Part, part: &mut tim_c::Part, part_ptr_from_index: F)
    where F: Fn(u16) -> *mut tim_c::Part
{
    part.part_type = p.part_type;
    part.flags1 = p.flags1;
    part.original_flags2 = p.flags2;
    part.flags2 = p.flags2;
    if let Some(flags3) = p.flags3 {
    part.original_flags2 = p.flags2;
        part.flags3 = flags3 & !(0x0010);
    }
    part.original_state1 = p.state1;
    part.state1 = p.state1;
    part.original_state2 = p.state2;
    part.state2 = p.state2;
    part.size.x = p.size.0 as i16;
    part.size.y = p.size.1 as i16;
    part.size_something2.x = p.size_something2.0 as i16;
    part.size_something2.y = p.size_something2.1 as i16;
    part.original_pos_x = p.original_pos.0;
    part.original_pos_y = p.original_pos.1;
    part.extra1 = p.extra1;

    part.belt_loc.x = p.belt_loc.0;
    part.belt_loc.y = p.belt_loc.1;
    part.belt_width = p.belt_width;
    if let Some((a, b)) = p.belt_part_indexes {
        let belt_raw = unsafe { tim_c::belt_data_alloc() };
        let belt = unsafe { belt_raw.as_mut().unwrap() };

        part.belt_data = belt_raw;
        belt.belt_part = part;
        belt.part1 = part_ptr_from_index(a);
        belt.part2 = part_ptr_from_index(b);

        if let Some(part1) = unsafe { belt.part1.as_mut() } {
            part1.belt_data = belt_raw;
        }

        if let Some(part2) = unsafe { belt.part2.as_mut() } {
            part2.belt_data = belt_raw;
        }
    }

    part.rope_loc[0].x = p.rope_1_loc.0;
    part.rope_loc[0].y = p.rope_1_loc.1;
    part.rope_loc[1].x = p.rope_2_loc.0;
    part.rope_loc[1].y = p.rope_2_loc.1;
    for &(i, rope_data) in &[(0, &p.rope_1_data), (1, &p.rope_2_data)] {
        if let Some(rope_data) = rope_data {
            let rope_raw = unsafe { tim_c::rope_data_alloc() };
            let rope = unsafe { rope_raw.as_mut().unwrap() };
    
            part.rope_data[i] = rope_raw;
            rope.rope_or_pulley_part = part;
    
            rope.part1 = part_ptr_from_index(rope_data.part1_index);
            rope.original_part1 = rope.part1;
            
            rope.part2 = part_ptr_from_index(rope_data.part2_index);
            rope.original_part2 = rope.part2;
    
            rope.part1_rope_slot = rope_data.part1_rope_slot;
            rope.original_part1_rope_slot = rope_data.part1_rope_slot;
            rope.part2_rope_slot = rope_data.part2_rope_slot;
            rope.original_part2_rope_slot = rope_data.part2_rope_slot;
    
            if let Some(part1) = unsafe { rope.part1.as_mut() } {
                part1.rope_data[rope_data.part1_rope_slot as usize] = rope;
            }
    
            if let Some(part2) = unsafe { rope.part2.as_mut() } {
                part2.rope_data[rope_data.part2_rope_slot as usize] = rope;
            }
        }
    }

    part.links_to[0] = part_ptr_from_index(p.links_to_part_indexes.0);
    part.links_to[1] = part_ptr_from_index(p.links_to_part_indexes.1);
    part.links_to_design[0] = part.links_to[0];
    part.links_to_design[1] = part.links_to[1];

    if let Some((a, b)) = p.plug_part_indexes {
        part.plug_parts[0] = part_ptr_from_index(a);
        part.plug_parts[1] = part_ptr_from_index(b);
    }

    if let Some(a) = p.pulley_part_index {
        // Assmes the part it's referring to has already been initialized
        let other_part_raw = part_ptr_from_index(a);
        let other_part = unsafe { other_part_raw.as_ref().unwrap() };
        part.rope_data[1] = other_part.rope_data[0];
    }

    unsafe {
        tim_c::part_alloc_borders_and_reset(part);
    }
}

use std::fs::File;
use std::io::BufWriter;
use std::io::prelude::*;
use crate::tim_c;
use crate::part::PartType;
use std::collections::HashSet;

pub fn write_raster_to_ppm(filename: &str, raster_buf: &[u8], raster_width: usize, raster_height: usize, raster_buf_stride: usize) -> std::io::Result<()> {
    let mut f = BufWriter::new(File::create(filename)?);
    write!(&mut f, "P3\n{} {}\n255\n", raster_width, raster_height)?;
    for y in 0..raster_height {
        for x in 0..raster_width {
            let off = x*4 + y*raster_buf_stride;
            let b = &raster_buf[off..off+4];
            write!(&mut f, "{} {} {}\n", b[0], b[1], b[2])?;
        }
    }
    Ok(())
}

fn graphviz_part_label(ptr: *const tim_c::Part) -> String {
    // note: I know this is dumb, especially for archs that don't have 64-bit pointers.
    let ptr_number = ptr as u64;
    format!("part_{:016X}", ptr_number)
}

fn graphviz_rope_label(ptr: *const tim_c::RopeData) -> String {
    // note: I know this is dumb, especially for archs that don't have 64-bit pointers.
    let ptr_number = ptr as u64;
    format!("rope_{:016X}", ptr_number)
}

fn graphviz_belt_label(ptr: *const tim_c::BeltData) -> String {
    // note: I know this is dumb, especially for archs that don't have 64-bit pointers.
    let ptr_number = ptr as u64;
    format!("belt_{:016X}", ptr_number)
}

/// 
pub fn dump_level_to_graphviz_file(filename: &str)-> std::io::Result<()> {
    let mut f = BufWriter::new(File::create(filename)?);
    let mf = &mut f;

    let mut visited_ropedatas = HashSet::new();
    let mut visited_beltdatas = HashSet::new();

    write!(mf, "digraph {{\n");
    for p in unsafe { tim_c::static_parts_iter().chain(tim_c::moving_parts_iter()) } {
        let part_label = graphviz_part_label(p);

        write!(mf, "  {} [label={:?}]\n", part_label, PartType::from_u16(p.part_type));

        for (i, &rope_data) in p.rope_data.iter().enumerate() {
            if !rope_data.is_null() {
                write!(mf, "  {} -> {} [label=rd{}]\n", part_label, graphviz_rope_label(rope_data), i+1);
                if !visited_ropedatas.contains(&rope_data) {
                    visited_ropedatas.insert(rope_data);
                    let rope_data = unsafe { rope_data.as_ref().unwrap() };
                    write!(mf, "  {} [label=\"{:?}\",style=filled,fillcolor=red]\n", graphviz_rope_label(rope_data), (rope_data.part1_rope_slot, rope_data.part2_rope_slot));
                    write!(mf, "  {} -> {} [label=main]\n", graphviz_rope_label(rope_data), graphviz_part_label(rope_data.rope_or_pulley_part));
                    if !rope_data.part1.is_null() {
                        write!(mf, "  {} -> {} [label=part1]\n", graphviz_rope_label(rope_data), graphviz_part_label(rope_data.part1));
                    }
                    if !rope_data.part2.is_null() {
                        write!(mf, "  {} -> {} [label=part2]\n", graphviz_rope_label(rope_data), graphviz_part_label(rope_data.part2));
                    }
                }
            }
        }
        if !p.belt_data.is_null() && !visited_beltdatas.contains(&p.belt_data) {
            visited_beltdatas.insert(p.belt_data);
            let belt_data = unsafe { p.belt_data.as_ref().unwrap() };
            belt_data.belt_part;
            belt_data.part1;
            belt_data.part2;
        }
        if !p.links_to[0].is_null() {
            write!(mf, "  {} -> {} [label=linksto1]\n", part_label, graphviz_part_label(p.links_to[0]));
        }
        if !p.links_to[1].is_null() {
            write!(mf, "  {} -> {} [label=linksto2]\n", part_label, graphviz_part_label(p.links_to[1]));
        }
    }
    write!(mf, "}}\n");
    Ok(())
}
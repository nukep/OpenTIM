mod image;
mod buffer_snake;
mod debug;
mod decoders;
mod resource_dos;

use std::fs::File;
use std::io::BufWriter;
use std::io::prelude::*;
use std::convert::TryInto;
use std::str;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let args: Vec<String> = std::env::args().collect();
    let root_directory = &args[1];

    let mut resources = resource_dos::from_map(root_directory, "RESOURCE.MAP")?;

    let mut filenames: Vec<String> = resources.iter_filenames().map(|s| s.into()).collect();
    // filenames.sort();
    println!("{:?}", filenames);

    // Scratch buffer for reading files
    // (our way to avoid dynamic allocations)
    let mut tmp_buf  = vec![0; 1000000];
    // let mut tmp_buf2 = vec![0; 1000000];

    let mut tim_pal_buf     = [[0,0,0,0];256];
    // let mut dynamix_pal_buf = [[0,0,0,0];256];
    // let mut sierra_pal_buf  = [[0,0,0,0];256];
    let tim_pal     = resources.read("TIM.PAL",    &mut tmp_buf).and_then(|x| resource_dos::parse_vga_palette_as_rgba(x, &mut tim_pal_buf)).unwrap();
    // let dynamix_pal = resources.read("DYNAMIX.PAL", &mut tmp_buf).and_then(|x| resource_dos::parse_vga_palette_as_rgba(x, &mut dynamix_pal_buf)).unwrap();
    // let sierra_pal  = resources.read("SIERRA.PAL", &mut tmp_buf).and_then(|x| resource_dos::parse_vga_palette_as_rgba(x, &mut sierra_pal_buf)).unwrap();

    if true {
        let raster_width = 640;
        let raster_height = 0x1800;
        let raster_buf_stride = raster_width*4;
        let mut raster_buf = vec![255; raster_buf_stride*raster_height];
        let padding = 0;
    
        let mut draw_x = 0;
        let mut draw_y = 0;
        let mut max_h = 0;

        for filename in filenames.iter() {
            if let Some(bmp) = resources.read(&filename, &mut tmp_buf).and_then(resource_dos::parse_bmp_scn) {
                println!("Opened {}", filename);
                for b in bmp {
                    let (_, (max_x, max_y)) = image::bmp_scn::decode_bounds(b.scn).unwrap();
        
                    if draw_x + max_x as usize + 1 > raster_width {
                        draw_x = 0;
                        draw_y = max_h as usize + 1 + padding;
                    }
        
                    let buf = &mut raster_buf[draw_x*4 + draw_y*raster_buf_stride..];
                    image::bmp_scn::decode_rgba8(b.scn, buf, raster_buf_stride, &tim_pal).unwrap();
        
                    draw_x += max_x as usize + 1 + padding;
                    max_h = std::cmp::max(max_h, draw_y+max_y as usize);
                }
            }
        
        }
        debug::write_raster_to_ppm("out.ppm", &raster_buf, raster_width, raster_height, raster_buf_stride)?;
    }

    Ok(())
}

// For now...
#![allow(dead_code)]

mod image;
mod buffer_snake;
mod debug;
mod decoders;
mod resource_dos;
mod math;
mod nannou;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    nannou::start();
    Ok(())
}

pub fn load_images(on_image: &mut dyn FnMut(&str, usize, u32, u32, Vec<u8>)) -> Result<(), Box<dyn std::error::Error>> {
    let args: Vec<String> = std::env::args().collect();
    let root_directory = &args[1];

    let mut resources = resource_dos::from_map(root_directory, "RESOURCE.MAP")?;
    
    // Scratch buffer for reading files
    // (our way to avoid dynamic allocations)
    let mut tmp_buf  = vec![0; 1000000];

    let mut tim_pal_buf     = [[0,0,0,0];256];
    // let mut dynamix_pal_buf = [[0,0,0,0];256];
    // let mut sierra_pal_buf  = [[0,0,0,0];256];
    let tim_pal     = resources.read("TIM.PAL",    &mut tmp_buf).and_then(|x| resource_dos::parse_vga_palette_as_rgba(x, &mut tim_pal_buf)).unwrap();
    // let dynamix_pal = resources.read("DYNAMIX.PAL", &mut tmp_buf).and_then(|x| resource_dos::parse_vga_palette_as_rgba(x, &mut dynamix_pal_buf)).unwrap();
    // let sierra_pal  = resources.read("SIERRA.PAL", &mut tmp_buf).and_then(|x| resource_dos::parse_vga_palette_as_rgba(x, &mut sierra_pal_buf)).unwrap();

    #[allow(unused_mut)]
    let mut filenames: Vec<String> = resources.iter_filenames().map(|s| s.into()).collect();

    filenames.sort();

    for f in filenames.iter() {
        println!("{}", f);
    }

    for filename in filenames.iter() {
        if let Some(bmp) = resources.read(&filename, &mut tmp_buf).and_then(resource_dos::parse_bmp_scn) {
            for (i, b) in bmp.into_iter().enumerate() {
                let (_, (max_x, max_y)) = image::bmp_scn::decode_bounds(b.scn).unwrap();

                let width = max_x as u32 + 1;
                let height = max_y as u32 + 1;

                let stride = width as usize * 4;

                let mut buf = vec![0; (width*height*4) as usize];
                image::bmp_scn::decode_rgba8(b.scn, &mut buf, stride, &tim_pal).unwrap();

                on_image(filename, i, width, height, buf);
            }
        }
    
    }

    Ok(())
}
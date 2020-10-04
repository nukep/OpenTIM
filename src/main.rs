// For now...
#![allow(dead_code)]

mod image;
mod buffer_snake;
mod debug;
mod decoders;
mod resource_dos;
mod math;
mod nannou;
mod tim_c;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let bowling_ball_raw = unsafe { tim_c::part_new(0) };
    let balloon_raw = unsafe { tim_c::part_new(4) };
    let brick_wall_raw = unsafe { tim_c::part_new(1) };
    let brick_wall2_raw = unsafe { tim_c::part_new(1) };

    {
        let bowling_ball: &mut tim_c::Part = unsafe { bowling_ball_raw.as_mut().unwrap() };
        bowling_ball.flags1 = 0x1000;
        bowling_ball.flags2 = 0x0000;
        bowling_ball.flags3 = 0x0008;
        bowling_ball.original_pos_x = 176;
        bowling_ball.original_pos_y = 100;

        let balloon: &mut tim_c::Part = unsafe { balloon_raw.as_mut().unwrap() };
        balloon.original_pos_x = 230;
        balloon.original_pos_y = 340;

        let brick_wall: &mut tim_c::Part = unsafe { brick_wall_raw.as_mut().unwrap() };
        brick_wall.original_pos_x = 176;
        brick_wall.original_pos_y = 400;
        brick_wall.size_something2.x = 16*8;
        brick_wall.flags1 = 0x6040;
        brick_wall.flags2 = 0x0180;     // was originally set
        brick_wall.flags3 = 0x0000;     

        let brick_wall2: &mut tim_c::Part = unsafe { brick_wall2_raw.as_mut().unwrap() };
        brick_wall2.original_pos_x = 176;
        brick_wall2.original_pos_y = 32;
        brick_wall2.size_something2.x = 16*8;
        brick_wall2.flags1 = 0x6040;
        brick_wall2.flags2 = 0x0180;     // was originally set
        brick_wall2.flags3 = 0x0000;     
    }

    unsafe {
        tim_c::insert_part_into_moving_parts(bowling_ball_raw);
        tim_c::insert_part_into_moving_parts(balloon_raw);
        tim_c::insert_part_into_static_parts(brick_wall_raw);
        tim_c::insert_part_into_static_parts(brick_wall2_raw);
        tim_c::restore_parts_state_from_design();
        // for _ in 0..60 {
        //     tim_c::advance_parts();
        //     tim_c::all_parts_set_prev_vars();
        // }
        
        // let bowling_ball: &mut tim_c::Part = unsafe { bowling_ball_raw.as_mut().unwrap() };
        // println!("{:?}", bowling_ball.border_points());
    }
    // println!(b)
    // tim_c::print_parts();

    // println!("{}", unsafe { tim_c::debug_part_size() });
    // println!("{}", std::mem::size_of::<tim_c::BeltData>());
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
        // println!("{}", f);
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
use std::fs::File;
use std::io::BufWriter;
use std::io::prelude::*;

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
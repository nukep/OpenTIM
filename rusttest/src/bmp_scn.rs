use std::cell::Cell;
use std::cmp;

type BmpScnResult<T> = Result<T, &'static str>;

fn decode_impl<R, P, W>(mut read: R, mut peek: P, mut plot: W) -> BmpScnResult<()>
    where R: FnMut() -> BmpScnResult<u8>,
          P: FnMut() -> BmpScnResult<u8>,
          W: FnMut(i32, i32, u8)
{
    let first_byte = read()?;

    let mut x: i32 = 0;
    let mut y: i32 = 0;

    loop {
        let input = read()?;
        let code = input & 0xC0;

        if code == 0x00 {
            y += 1;
            x -= (input & 0x3F) as i32;

            let tmp = peek()?;

            if (tmp & 0xC0) == 0x00 && (tmp & 0x3F) != 0 {
                let tmp = read()? & 0x3F;
                x -= (tmp as i32) * 64;
            }
        } else if code == 0x40 {
            let off = input & 0x3F;
            x += off as i32;

            if off == 0 {
                // Done!
                return Ok(())
            }
        } else if code == 0x80 {
            let count = input & 0x3F;
            let color = read()? + first_byte;

            for i in 0..count {
                plot(x+i as i32, y, color);
            }
            x += count as i32;
        } else /* if code == 0xC0 */ {
            let count = input & 0x3F;

            for i in (0..(count as i32/2)).map(|v| v*2) {
                let v = read()?;
                plot(x+i+0, y, (v >> 4) + first_byte);
                plot(x+i+1, y, (v & 0xF) + first_byte);
            }
            if count % 2 != 0 {
                let v = read()?;
                let i = (count-1) as i32;
                plot(x+i+0, y, (v >> 4) + first_byte);
            }

            x += count as i32;
        }
    }
}

fn decode_buf<W>(buf: &[u8], plot: W) -> BmpScnResult<()>
    where W: FnMut(i32, i32, u8)
{
    let off = Cell::new(0);
    decode_impl(
        || {
            let o = off.get();
            if let Some(&v) = buf.get(o) {
                off.set(o + 1);
                Ok(v)
            } else {
                Err("No more input")
            }
        },
        || {
            if let Some(&v) = buf.get(off.get()) {
                Ok(v)
            } else {
                Err("No more input")
            }
        },
        plot
    )
}

pub fn decode_rgba8(buf: &[u8], out: &mut [u8], stride: i32, pal: &[[u8;4];256]) -> BmpScnResult<()> {
    decode_buf(buf, |x, y, v| {
        let off = (x*4 + y*stride) as usize;
        out[off..off+4].copy_from_slice(&pal[v as usize]);
    })
}

pub fn decode_bounds(buf: &[u8]) -> BmpScnResult<((i32, i32), (i32, i32))> {
    let mut min_x = 0;
    let mut min_y = 0;
    let mut max_x = 0;
    let mut max_y = 0;

    let mut plotted = false;

    decode_buf(buf, |x, y, _| {
        if !plotted {
            min_x = x;
            max_x = x;
            min_y = y;
            max_y = y;
            plotted = true;
        } else {
            min_x = cmp::min(min_x, x);
            min_y = cmp::min(min_y, y);
            max_x = cmp::max(max_x, x);
            max_y = cmp::max(max_y, y);
        }
    })?;

    Ok(((min_x, min_y), (max_x, max_y)))
}

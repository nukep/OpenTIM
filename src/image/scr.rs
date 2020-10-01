fn decode_impl<W>(buf: &[u8], width: usize, mut plot: W)
    where W: FnMut(usize, usize, u8)
{
    let mut x = 0;
    let mut y = 0;
    for &value in buf {
        let v1 = (value >> 4) as u8;
        let v2 = (value & 0xF) as u8;
        for &v in &[v1, v2] {
            plot(x, y, v);
            x += 1;
            if x >= width {
                x = 0;
                y += 1;
            }
        }
    }
}

pub fn decode_rgba8(buf: &[u8], out: &mut [u8], width: usize, stride: usize, pal: &[[u8;4];256]) {
    decode_impl(buf, width, |x, y, v| {
        let off = x*4 + y*stride;
        out[off..off+4].copy_from_slice(&pal[v as usize]);
    })
}
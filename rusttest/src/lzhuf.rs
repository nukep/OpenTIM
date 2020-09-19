// A readaptation of lzhuf.c from Haruyasu Yoshizaki.
// Readapted by Danny Spencer (2020). Decompression only.
// Original credits:
/**************************************************************
    lzhuf.c
    written by Haruyasu Yoshizaki 1988/11/20
    some minor changes 1989/04/06
    comments translated by Haruhiko Okumura 1989/04/07
**************************************************************/

type LzhufResult<T> = Result<T, &'static str>;

const N: usize = 4096;
const F: usize = 60;
const THRESHOLD: usize = 2;

const N_CHAR: usize = 256 - THRESHOLD + F;
const T: usize = N_CHAR*2 - 1;
const R: usize = T - 1;
const MAX_FREQ: u16 = 0x8000;

const D_CODE: [u8; 256] = [
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
    0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
    0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
    0x0C, 0x0C, 0x0C, 0x0C, 0x0D, 0x0D, 0x0D, 0x0D,
    0x0E, 0x0E, 0x0E, 0x0E, 0x0F, 0x0F, 0x0F, 0x0F,
    0x10, 0x10, 0x10, 0x10, 0x11, 0x11, 0x11, 0x11,
    0x12, 0x12, 0x12, 0x12, 0x13, 0x13, 0x13, 0x13,
    0x14, 0x14, 0x14, 0x14, 0x15, 0x15, 0x15, 0x15,
    0x16, 0x16, 0x16, 0x16, 0x17, 0x17, 0x17, 0x17,
    0x18, 0x18, 0x19, 0x19, 0x1A, 0x1A, 0x1B, 0x1B,
    0x1C, 0x1C, 0x1D, 0x1D, 0x1E, 0x1E, 0x1F, 0x1F,
    0x20, 0x20, 0x21, 0x21, 0x22, 0x22, 0x23, 0x23,
    0x24, 0x24, 0x25, 0x25, 0x26, 0x26, 0x27, 0x27,
    0x28, 0x28, 0x29, 0x29, 0x2A, 0x2A, 0x2B, 0x2B,
    0x2C, 0x2C, 0x2D, 0x2D, 0x2E, 0x2E, 0x2F, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
];

const D_LEN: [u8; 256] = [
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
];

// Result ranges from 0 to 63
fn d_code(idx: u8) -> u8 {
    unsafe {
        // Relies on the fact that the table is 256 bytes
        return *D_CODE.get_unchecked(idx as usize);
    }
}

// Result ranges from 3 to 8
fn d_len(idx: u8) -> u8 {
    unsafe {
        // Relies on the fact that the table is 256 bytes
        return *D_LEN.get_unchecked(idx as usize);
    }
}

pub struct Buffers {
    text_buf: [u8; N + F - 1],
    freq: [u16; T + 1],
    prnt: [usize; T + N_CHAR],
    son: [usize; T]
}

impl Buffers {
    pub fn new() -> Self {
        Buffers {
            text_buf: [0; N + F - 1],
            freq: [0; T + 1],
            prnt: [0; T + N_CHAR],
            son: [0; T]
        }
    }
}

fn reconst(b: &mut Buffers) {
    // Collect leaf nodes in the first half of the table
    // and replace the freq by (freq + 1) / 2
    let mut j = 0;
    for i in 0..T {
        if b.son[i] >= T {
            b.freq[j] = (b.freq[i] + 1)/2;
            b.son[j] = b.son[i];
            j += 1;
        }
    }

    // Begin constructing tree by connecting sons
    for j in N_CHAR..T {
        let i = (j - N_CHAR)*2;

        let kk = i + 1;
        b.freq[j] = b.freq[i] + b.freq[kk];
        let f = b.freq[j];

        let mut k = j-1;
        while f < b.freq[k] {
            k -= 1;
        }
        k += 1;

        b.freq.copy_within(k..j, k+1);
        b.freq[k] = f;
        b.son.copy_within(k..j, k+1);
        b.son[k] = i;
    }

    // Connect prnt
    for i in 0..T {
        let k = b.son[i];
        if k >= T {
            b.prnt[k] = i;
        } else {
            b.prnt[k] = i;
            b.prnt[k+1] = i;
        }
    }
}

fn update(b: &mut Buffers, c: usize) {
    if b.freq[R] == MAX_FREQ {
        reconst(b);
    }
    let mut c = b.prnt[c + T];

    loop {
        b.freq[c] += 1;
        let k = b.freq[c];

        // If the order is disturbed, exchange nodes
        let mut l = c + 1;
        if k > b.freq[l] {
            loop {
                l += 1;
                if k <= b.freq[l] {
                    break;
                }
            }
            l -= 1;

            b.freq[c] = b.freq[l];
            b.freq[l] = k;

            let i = b.son[c];
            b.prnt[i] = l;
            if i < T {
                b.prnt[i+1] = l;
            }

            let j = b.son[l];
            b.son[l] = i;

            b.prnt[j] = c;
            if j < T {
                b.prnt[j+1] = c;
            }
            b.son[c] = j;

            c = l;
        }

        c = b.prnt[c];
        if c == 0 {
            return;
        }
    }
}

fn decode_impl<R, W>(b: &mut Buffers, mut read_bits: R, mut write: W, out_size: usize) -> LzhufResult<usize>
    where R: FnMut(u8) -> u8,
          W: FnMut(u8) -> LzhufResult<()>
{
    // Initialize buffers
    for i in 0..N_CHAR {
        b.freq[i] = 1;
        b.son[i] = i + T;
        b.prnt[i + T] = i;
    }

    let mut i = 0;
    for j in N_CHAR..=R {
        b.freq[j] = b.freq[i] + b.freq[i + 1];
        b.son[j] = i;
        b.prnt[i] = j;
        b.prnt[i + 1] = j;
        i += 2;
    }

    b.freq[T] = 0xFFFF;
    b.prnt[R] = 0;

    for i in 0..N-F {
        b.text_buf[i] = 0x20;
    }
    // End initialize buffers

    let mut r = N - F;
    let mut count = 0;

    while count < out_size {
        // Decode char
        let mut c = b.son[R];
        while c < T {
            c += read_bits(1) as usize;
            c = b.son[c];
        }
        c -= T;
        update(b, c);
        // End decode char

        if c < 256 {
            write(c as u8)?;
            b.text_buf[r] = c as u8;
            r += 1;
            r &= N-1;
            count += 1;
        } else {
            // Decode position
            let pos = {
                // Recover upper bits from table
                let mut i = read_bits(8);
                let c: u16 = (d_code(i) as u16) << 6;
                let mut j = d_len(i);

                // Read lower 6 bits verbatim
                j -= 2;
                // j is 1 to 6
                while j > 0 {
                    j -= 1;
                    i = (i << 1) + read_bits(1);
                }

                c as usize | (i & 0x3F) as usize
            };
            // End decode position

            let i = (r - pos - 1) & (N-1);
            let j = c - 255 + THRESHOLD;
            for k in 0..j {
                let c = b.text_buf[(i + k) & (N - 1)];
                write(c)?;
                b.text_buf[r] = c;
                r += 1;
                r &= N-1;
                count += 1;
            }
        }
    }

    Ok(count)
}

struct BitMsbReader<'a> {
    bit_off: usize,
    buf: &'a [u8]
}

impl<'a> BitMsbReader<'a> {
    fn read_bits(&mut self, n_bits: u8) -> u8 {
        let mut val = 0;

        for i in 0..n_bits {
            let byte_off = self.bit_off / 8;
            let bit = self.bit_off % 8;

            if byte_off >= self.buf.len() {
                // Can't read any more bytes
                // Pretend it's 0
                val = val << 1;
            } else {
                val = val << 1;
                if (self.buf[byte_off] & (1 << (7-bit))) != 0 {
                    val |= 1;
                }
            }

            self.bit_off += 1;
        }

        val
    }
}

mod tests {
    macro_rules! check_expecteds {
        ($n:expr, $a:expr, $b:expr) => {
            {
                let mut bit_reader = super::BitMsbReader { bit_off: 0, buf: $a };
                let cases = $b;
                for x in cases.iter() {
                    assert_eq!(bit_reader.read_bits($n), *x);
                }
            }
        }
    }
    #[test]
    fn bit_reader() {
        let buf = [0xCA, 0xDD];
        check_expecteds!(1, &buf, [1, 1, 0, 0, 1, 0, 1, 0,    1, 1, 0, 1, 1, 1, 0, 1,    0, 0]);
        check_expecteds!(3, &buf, [6, 2, 5, 5, 6, 4,  0]);
        check_expecteds!(4, &buf, [0xC, 0xA, 0xD, 0xD, 0]);
        check_expecteds!(8, &buf, [0xCA, 0xDD, 0, 0]);
    }
}

pub fn decode(b: &mut Buffers, buf: &[u8], out: &mut [u8]) -> LzhufResult<usize> {
    let mut bit_reader = BitMsbReader {
        bit_off: 0,
        buf: buf
    };

    let out_len = out.len();
    let mut out_off = 0;

    decode_impl(
        b,
        |n_bits| bit_reader.read_bits(n_bits),
        |v| {
            out[out_off] = v;
            out_off += 1;
            Ok(())
        },
        out_len
    )
}
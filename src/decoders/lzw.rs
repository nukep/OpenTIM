// Adapted from ScummVM (licensed GPL v2 or greater):
// https://github.com/scummvm/scummvm/blob/d11c61db1466d79ef6253814a86b04950b499ec3/engines/sci/decompressor.cpp
//
// This LZW decompression algorithm is subtlely different from other Sierra games.
// 1) The length 2+ dictionary starts at 0x101 instead of 0x102.
// 2) The reset command aligns the read stream to a 20-byte boundary.

use thiserror::Error;

#[derive(Error, Debug)]
pub enum LzwError {
    #[error("Bad token: `{0}`. Current bit offset: `{1}`")]
    BadToken(u16, usize),

    #[error("No more input")]
    NoMoreInput,

    #[error("Output buffer ran out of space")]
    NoMoreOutput
}

pub type LzwResult<T> = Result<T, LzwError>;

const LZW_MAX_BITS: u8 = 12;

fn decode_impl<'a, R, W, E>(mut read_bits: R, mut write: W, mut done: E) -> LzwResult<()>
    where R: FnMut(u8) -> LzwResult<u16>,
          W: FnMut(&[u8]) -> LzwResult<&'a [u8]>,
          E: FnMut() -> bool
{
    // Tokens are references to the data we just wrote to, to save space.
    let mut tokens: [&'a [u8]; 1<<LZW_MAX_BITS] = [&[]; 1<<LZW_MAX_BITS];

    let mut n_bits = 9;
    let mut curtoken = 0x101;
    let mut endtoken = 0x1ff;

    let mut subbit: usize = 0;

    while !done() {
        let token = read_bits(n_bits)?;
        subbit += n_bits as usize;

        if token == 0x100 {
            // Reset the dictionary

            // Seek forward to a 20-byte boundary (games like The Incredible Machine and Toons require this for some reason)
            let boundary = 8*20;
            let sb = subbit % boundary;
            if sb > 0 {
                let r = boundary - sb;
                read_bits((r%8) as u8)?;
                for _ in 0..(r/8) {
                    read_bits(8)?;
                }
                subbit += r;
            }

            n_bits = 9;
            curtoken = 0x101;
            endtoken = 0x1ff;
        } else {
            let tokenlastlength;
            let written_buf: &[u8];

            if token >= 0x100 {
                // Use token in dictionary

                if token >= curtoken {
                    return Err(LzwError::BadToken(token, subbit));
                }

                // Write the token string...

                write(tokens[token as usize])?;

                // ... then 1 byte from the next token

                written_buf = if token == curtoken-1 {
                    // The next token doesn't exist yet. It'll be the one being made right now.
                    // Take the 1st byte of the token we just wrote.
                    write(&tokens[token as usize][0..1])?
                } else {
                    write(&tokens[token as usize + 1][0..1])?
                };

                tokenlastlength = tokens[token as usize].len() + 1;
            } else {
                written_buf = write(&[token as u8])?;
                tokenlastlength = 1;
            }

            if curtoken > endtoken {
                if n_bits < LZW_MAX_BITS {
                    n_bits += 1;
                    endtoken = (endtoken << 1) + 1;     // 511 -> 1023 -> 2047 -> 4095
                }
            }

            if curtoken < (1<<LZW_MAX_BITS) {
                let s = &written_buf[written_buf.len() - tokenlastlength..];
                tokens[curtoken as usize] = s;
                curtoken += 1;
            }
        }
    }

    Ok(())
}

struct BitLsbReader<'a> {
    bit_off: usize,
    buf: &'a [u8]
}

impl<'a> BitLsbReader<'a> {
    fn read_bits(&mut self, n_bits: u8) -> LzwResult<u16> {
        let mut val = 0;

        for i in 0..n_bits {
            let byte_off = self.bit_off / 8;
            let bit = self.bit_off % 8;

            if byte_off >= self.buf.len() {
                // Can't read any more bytes
                return Err(LzwError::NoMoreInput);
            } else {
                if (self.buf[byte_off] & (1 << bit)) != 0 {
                    val |= 1 << i;
                }
            }

            self.bit_off += 1;
        }

        Ok(val)
    }

    fn bits_remaining(&self) -> usize {
        self.buf.len()*8 - self.bit_off
    }
}

use crate::buffer_snake::BufferSnake;
use std::cell::Cell;

pub fn decode<'a>(buf: &[u8], out: &'a mut [u8]) -> LzwResult<&'a [u8]> {
    let mut bit_reader = BitLsbReader {
        bit_off: 0,
        buf: buf
    };

    let mut snake = BufferSnake::new(out);
    let is_it_done = Cell::new(false);

    decode_impl(
        |n_bits| {
            // Read bits
            let r = bit_reader.read_bits(n_bits);

            if bit_reader.bits_remaining() == 0 {
                is_it_done.set(true);
            }

            r
        },
        |bytes| {
            // Write bytes
            if snake.try_write(bytes) {
                if snake.remaining() == 0 {
                    is_it_done.set(true);
                }

                Ok(snake.tail())
            } else {
                Err(LzwError::NoMoreOutput)
            }
        },
        || {
            // Is it done?
            is_it_done.get()
        },
    )?;

    Ok(snake.tail())
}

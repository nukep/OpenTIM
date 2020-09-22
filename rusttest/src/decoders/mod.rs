pub mod lzhuf;
pub mod lzw;

use thiserror::Error;
use std::convert::TryInto;

#[derive(Error, Debug)]
pub enum GenericDecodeError {
    #[error("No header")]
    NoHeader,

    #[error("Output buffer not large enough (requires `{0}` bytes)")]
    OutputBufferNotLargeEnough(u32),

    #[error("Unimplemented compression type (type `{0}`)")]
    Unimplemented(&'static str),

    #[error("Unknown compression type (type `{0}`)")]
    Unknown(u8),

    #[error("Noop output buffer ran out of space")]
    NoopNoMoreOutput,

    #[error(transparent)]
    Lzw(#[from] lzw::LzwError),

    #[error(transparent)]
    Lzhuf(#[from] lzhuf::LzhufError),
}

fn noop_decode<'a>(buf: &[u8], out: &'a mut [u8]) -> Result<&'a [u8], GenericDecodeError> {
    if buf.len() > out.len() {
        return Err(GenericDecodeError::NoopNoMoreOutput);
    }
    
    out[0..buf.len()].copy_from_slice(buf);
    Ok(out)
}

pub fn generic_decode<'a>(buf: &[u8], out: &'a mut [u8]) -> Result<&'a [u8], GenericDecodeError> {
    if buf.len() < 5 {
        return Err(GenericDecodeError::NoHeader);
    }

    let compression_type = buf[0];
    let uncompressed_size = u32::from_le_bytes(buf[1..5].try_into().unwrap());

    if uncompressed_size as usize > out.len() {
        return Err(GenericDecodeError::OutputBufferNotLargeEnough(uncompressed_size));
    }

    let buf = &buf[5..];
    let out = &mut out[0..uncompressed_size as usize];

    match compression_type {
        0 => Ok(noop_decode(buf, out)?),
        1 => Err(GenericDecodeError::Unimplemented("RLE")),
        2 => Ok(lzw::decode(buf, out)?),
        3 => Ok(lzhuf::decode(buf, out)?),
        _ => Err(GenericDecodeError::Unknown(compression_type)),
    }
}

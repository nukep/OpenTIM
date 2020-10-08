use thiserror::Error;
use byteorder::{LittleEndian, ReadBytesExt};
use std::io::Cursor;
use crate::part::PartType;

#[derive(Error, Debug)]
pub enum LevelReadError {
    #[error("Some error")]
    SomeError,

    #[error("Bad magic: `{0}`")]
    BadMagic(u16),

    #[error("Unknown Part type: `{0}`")]
    UnknownPartType(u16),

    #[error(transparent)]
    IoError(#[from] std::io::Error)
}

#[derive(Debug)]
pub struct Level {
    pub version: u16,

    pub puzzle_title: Option<String>,
    pub puzzle_objective: Option<String>,

    pub bonus: Option<(u16, u16)>,

    pub air_pressure: u16,
    pub gravity: u16,

    pub unknown: Option<(u16, u16)>,

    pub music_track: u16,

    pub static_parts: Vec<Part>,
    pub moving_parts: Vec<Part>,
    pub bin_parts: Option<Vec<Part>>,
}

#[derive(Debug)]
pub struct Part {
    pub part_type: PartType,
    pub flags1: u16,
    pub flags2: u16,
    pub flags3: Option<u16>,
    pub state1: i16,
    pub state2: i16,
    pub size: (u16, u16),
    pub size_something2: (u16, u16),
    pub original_pos: (i16, i16),
    pub extra1: i16,

    pub belt_loc: (u8, u8),
    pub belt_width: u16,
    pub belt_part_indexes: Option<(u16, u16)>,

    pub rope_1_loc: (u8, u8),
    pub rope_1_data: Option<RopeData>,
    pub rope_2_loc: (u8, u8),
    pub rope_2_data: Option<RopeData>,

    pub links_to_part_indexes: (u16, u16),

    pub plug_part_indexes: Option<(u16, u16)>,

    pub pulley_part_index: Option<u16>, // only set if type is a pulley
}

#[derive(Debug)]
pub struct RopeData {
    pub part1_index: u16,
    pub part2_index: u16,
    pub part1_rope_slot: u8,
    pub part2_rope_slot: u8
}

trait MyReadExt: std::io::Read {
    #[inline(always)]
    fn read_u16_le(&mut self) -> Result<u16, LevelReadError> {
        Ok(self.read_u16::<LittleEndian>()?)
    }

    #[inline(always)]
    fn read_i16_le(&mut self) -> Result<i16, LevelReadError> {
        Ok(self.read_i16::<LittleEndian>()?)
    }

    #[inline(always)]
    fn read_str(&mut self) -> Result<String, LevelReadError> {
        // Strings are null-terminated.

        let mut strout = String::new();

        loop {
            let c = self.read_u8()?;
            if c == 0 {
                return Ok(strout);
            }

            strout.push(c as char);
        }
    }
}
impl<R: std::io::Read> MyReadExt for R {}

pub fn read(buf: &[u8], freeform_mode: bool) -> Result<Level, LevelReadError> {
    let mut c = Cursor::new(buf);

    let magic = c.read_u16_le()?;
    if !(magic == 0xaced || magic == 0xacee) {
        return Err(LevelReadError::BadMagic(magic));
    }

    let version = c.read_u16_le()?;

    let puzzle_title;
    let puzzle_objective;
    let bonus;

    if freeform_mode {
        puzzle_title = None;
        if version >= 0x104 {
            puzzle_objective = Some(c.read_str()?);
        } else {
            puzzle_objective = None;
        }
        bonus = None;
    } else {
        puzzle_title = Some(c.read_str()?);
        puzzle_objective = Some(c.read_str()?);
        bonus = Some((c.read_u16_le()?, c.read_u16_le()?));
    }

    let air_pressure = c.read_u16_le()?;
    let gravity = c.read_u16_le()?;

    let unknown = if !freeform_mode {
        Some((c.read_u16_le()?, c.read_u16_le()?))
    } else {
        None
    };

    let music_track = c.read_u16_le()?;
    let num_static_parts = c.read_u16_le()?;
    let num_moving_parts = c.read_u16_le()?;
    let num_bin_parts = c.read_u16_le()?;

    let mut static_parts = Vec::with_capacity(num_static_parts as usize);
    for _ in 0..num_static_parts {
        static_parts.push(read_part(&mut c, version)?);
    }

    let mut moving_parts = Vec::with_capacity(num_moving_parts as usize);
    for _ in 0..num_moving_parts {
        moving_parts.push(read_part(&mut c, version)?);
    }

    let explicit_parts_bin = if version >= 0x105 {
        c.read_u16_le()? != 0
    } else {
        false
    };

    let bin_parts = if !freeform_mode || explicit_parts_bin {
        let mut bin_parts_v = Vec::with_capacity(num_bin_parts as usize);
        for _ in 0..num_bin_parts {
            bin_parts_v.push(read_part(&mut c, version)?);
        }
        Some(bin_parts_v)
    } else {
        None
    };

    Ok(Level {
        version,
        puzzle_title,
        puzzle_objective,
        bonus,
        air_pressure,
        gravity,
        unknown,
        music_track,
        static_parts,
        moving_parts,
        bin_parts,
    })
}

fn read_part<T: std::io::Read>(mut c: T, version: u16) -> Result<Part, LevelReadError> {
    let part_type_num = c.read_u16_le()?;
    let part_type = PartType::try_from_u16(part_type_num).ok_or(LevelReadError::UnknownPartType(part_type_num))?;

    let flags1 = c.read_u16_le()?;
    let flags2 = c.read_u16_le()?;
    let flags3 = if version > 0x100 {
        Some(c.read_u16_le()?)
    } else {
        None
    };

    let state1 = c.read_i16_le()?;
    let state2 = c.read_i16_le()?;

    let size            = (c.read_u16_le()?, c.read_u16_le()?);
    let size_something2 = (c.read_u16_le()?, c.read_u16_le()?);

    let original_pos    = (c.read_i16_le()?, c.read_i16_le()?);

    let extra1 = c.read_i16_le()?;

    let has_belt_data = c.read_u16_le()? != 0;
    let belt_loc = (c.read_u8()?, c.read_u8()?);
    let belt_width = c.read_u16_le()?;
    let belt_part_indexes = if has_belt_data {
        Some((c.read_u16_le()?, c.read_u16_le()?))
    } else {
        None
    };

    let has_rope_data_1 = c.read_u16_le()? != 0;
    let rope_1_loc = (c.read_u8()?, c.read_u8()?);
    let rope_1_data = if has_rope_data_1 {
        Some(RopeData {
            part1_index: c.read_u16_le()?,
            part2_index: c.read_u16_le()?,
            part1_rope_slot: c.read_u8()?,
            part2_rope_slot: c.read_u8()?,
        })
    } else {
        None
    };
    let has_rope_data_2 = c.read_u16_le()? != 0;
    let rope_2_loc = (c.read_u8()?, c.read_u8()?);
    let rope_2_data = if has_rope_data_2 {
        Some(RopeData {
            part1_index: c.read_u16_le()?,
            part2_index: c.read_u16_le()?,
            part1_rope_slot: c.read_u8()?,
            part2_rope_slot: c.read_u8()?,
        })
    } else {
        None
    };

    let links_to_part_indexes = (c.read_u16_le()?, c.read_u16_le()?);

    let plug_part_indexes = if version > 0x100 {
        Some((c.read_u16_le()?, c.read_u16_le()?))
    } else {
        None
    };

    let pulley_part_index = if part_type == PartType::Pulley {
        Some(c.read_u16_le()?)
    } else {
        None
    };
    
    // Discard extra bytes
    if version < 0x102 {
        let discard_rep = c.read_u16_le()?;
        for _ in 0..discard_rep {
            c.read_u8()?;
            c.read_u8()?;
        }
    }

    Ok(Part {
        part_type,
        flags1,
        flags2,
        flags3,
        state1,
        state2,
        size,
        size_something2,
        original_pos,
        extra1,
        belt_loc,
        belt_width,
        belt_part_indexes,

        rope_1_loc,
        rope_1_data,
        rope_2_loc,
        rope_2_data,

        links_to_part_indexes,

        plug_part_indexes,
        pulley_part_index,
    })
}
// Used to read MS-DOS distributions of The (Even More) Incredible Machine, Sid & Al's Incredible Toons, and other Sierra/Dynamix games

use std::io;
use std::str;
use std::convert::TryInto;
use std::fs::File;
use std::io::{Read, Seek};
use std::path::Path;


const CHUNK_NAME_SIZE: usize = 13;

fn dos_filename_buf_to_utf8(name_buf: &[u8]) -> &str {
    // Convert name to string slice.
    // DOS filenames (what the names are compatible with) are restricted to ASCII, so it should be safe to convert to UTF-8.
    let name_buf_before_null = name_buf.split(|&x| x == 0).next().unwrap();
    str::from_utf8(name_buf_before_null).unwrap()
}

fn read_header<'a, I: io::Read>(f: &mut I, name_buf: &'a mut [u8; CHUNK_NAME_SIZE]) -> io::Result<(&'a str, u32)> {
    // Yes, the header includes the null terminator
    // But we'll set it here just in case (we can't "trust" external asset files)
    f.read_exact(name_buf)?;
    name_buf[CHUNK_NAME_SIZE-1] = 0;

    let mut payload_size_buf: [u8; 4] = [0; 4];
    f.read_exact(&mut payload_size_buf)?;

    let payload_size = u32::from_le_bytes(payload_size_buf);

    let name = dos_filename_buf_to_utf8(name_buf);

    Ok((name, payload_size))
}

// Scan the resource file for entries until EOF is reached
// May be an acceptable alternative in case the .MAP file is missing or corrupted
fn scan_resource_file_entries<'a, I: io::Read+io::Seek, F>(f: &mut I, mut handle: F) -> io::Result<()>
    where F: FnMut(&str, u32, i64)
{
    let mut name_buf = [0; CHUNK_NAME_SIZE];

    loop {
        // TODO - use stream_position once it become stable
        let offset = f.seek(io::SeekFrom::Current(0))?.try_into().unwrap();

        let header_result = read_header(f, &mut name_buf);
        match header_result {
            Ok((name, payload_size)) => {
                handle(name, payload_size, offset);

                // Ignore the payload
                f.seek(io::SeekFrom::Current(payload_size as i64))?;
            },

            Err(e) => match e.kind() {
                io::ErrorKind::UnexpectedEof => {
                    // When parsing headers, an Unexpected EOF is actually very expected
                    return Ok(())
                },
                _ => return Err(e)
            }
        }
    }
}

const RESOURCE_NAME_SIZE: usize = 13;

fn scan_resource_map_file<I: io::Read, E, F, G, T>(f: &mut I, mut on_resource_name: E, mut on_entry: F, mut on_resource_done: G) -> io::Result<[u8; 4]>
    where E: FnMut(&str, u16) -> T,
          F: FnMut(&mut T, u32, u32),
          G: FnMut(&str, T)
{
    let mut name_buf = [0; CHUNK_NAME_SIZE];

    let mut hash_string_indexes_buf = [0; 4];
    f.read_exact(&mut hash_string_indexes_buf)?;

    let mut file_count_buf = [0; 2];
    f.read_exact(&mut file_count_buf)?;

    let file_count = u16::from_le_bytes(file_count_buf);

    let mut resource_name_buf = [0; RESOURCE_NAME_SIZE];
    let mut entry_count_buf = [0; 2];
    let mut hash_buf = [0; 4];
    let mut offset_buf = [0; 4];

    for _ in 0..file_count {
        // Yes, the header includes the null terminator
        // But we'll set it here just in case (we can't "trust" external asset files)
        f.read_exact(&mut resource_name_buf)?;
        resource_name_buf[CHUNK_NAME_SIZE-1] = 0;
        let resource_name = dos_filename_buf_to_utf8(&resource_name_buf);

        f.read_exact(&mut entry_count_buf)?;
        let entry_count = u16::from_le_bytes(entry_count_buf);

        let mut resource_obj = on_resource_name(resource_name, entry_count);

        for _ in 0..entry_count {
            f.read_exact(&mut hash_buf)?;
            f.read_exact(&mut offset_buf)?;

            let hash = u32::from_le_bytes(hash_buf);
            let offset = u32::from_le_bytes(offset_buf);

            on_entry(&mut resource_obj, hash, offset);
        }

        on_resource_done(resource_name, resource_obj);
    }

    Ok(hash_string_indexes_buf)
}

fn uppercase(c: u8) -> u8 {
    if c >= 0x61 && c <= 0x7A {
        c - 0x20
    } else {
        c
    }
}

// Return a slice that comes right after the last backslash (\) or colon (:), if it exists
fn hash_file_strip_chars(s: &[u8]) -> &[u8] {
    let mut idx = 0;
    for (i, &c) in s.iter().enumerate() {
        if c == 0x5c || c == 0x3a {
            idx = i+1;
        }
    }

    &s[idx..]
}

// Credit for initial reading: http://www.shikadi.net/moddingwiki/TIM_Resource_Format
// TEMIM_DOS: 0824:A465
pub fn hash_filename(hash_indexes: &[u8;4], s_raw: &[u8]) -> u32 {
    let s = hash_file_strip_chars(s_raw);

    let mut isum: u16 = 0;
    let mut ixor: u8 = 0;

    for &c in s {
        isum += uppercase(c) as u16;
        ixor ^= uppercase(c);
    }

    // The calculated offset is signed. An overflowing multiplication may be interpreted as a negative value (two's complement).
    let hash_offset: i16 = (isum as i16).wrapping_mul(ixor as i16);

    let mut hash: u32 = 0;
    for &idx in hash_indexes {
        let c = s.get(idx as usize).map_or(0, |v| *v);
        hash = (hash<<8) | (uppercase(c) as u32);
    }
    hash = hash.wrapping_add(hash_offset as u32);

    hash
}

#[derive(Copy, Clone, Debug)]
struct ResourceEntry {
    resource_id: usize,
    payload_offset: u64,
    payload_size: u32
}

pub struct Resources {
    resource_files: Vec<File>,

    // I considered a HashMap earlier, but the list is so small that O(n) access is not likely to be a big deal.
    filename_to_resource_entry: Vec<(String, ResourceEntry)>,
}

impl Resources {
    fn get_resource_entry(&self, name: &str) -> Option<&ResourceEntry> {
        for (filename, e) in self.filename_to_resource_entry.iter() {
            if filename == name {
                return Some(e);
            }
        }
        None
    }

    // Returns a slice from the provided buffer is successful.
    // Returns None if the entry couldn't be read (if it doesn't exist, or if there's an I/O error).
    pub fn read<'a>(&mut self, name: &str, buf: &'a mut [u8]) -> Option<&'a [u8]> {
        if let Some(&entry) = self.get_resource_entry(name) {
            let f = &mut self.resource_files[entry.resource_id];
            f.seek(io::SeekFrom::Start(entry.payload_offset)).ok()?;
            let buf_subslice = &mut buf[0..entry.payload_size as usize];
            f.read_exact(buf_subslice).ok()?;
            Some(buf_subslice)
        } else {
            None
        }
    }

    pub fn iter_filenames<'a>(&'a self) -> Box<dyn Iterator<Item = &'a str> + 'a> {
        Box::new(self.filename_to_resource_entry.iter().map(|(filename, _)| filename.as_ref()))
    }
}

pub fn from_map(root_directory: &str, map_filename: &str) -> io::Result<Resources> {
    let mut resource_file_entries: Vec<(String, Vec<(u32, u32)>)> = vec![];
    let mut total_entries = 0;
    {
        let path = Path::new(root_directory).join(map_filename);
        let mut f = File::open(path)?;
        scan_resource_map_file(&mut f, |_resource_name, entry_count| {
            // New resource file
            Vec::with_capacity(entry_count as usize)
        }, |entries, hash, offset| {
            // Each entry
            entries.push((hash, offset));
        }, |resource_name, entries| {
            // Done resource file
            total_entries += entries.len();
            resource_file_entries.push((resource_name.into(), entries));
        })?;
    }

    let mut resource_files = Vec::with_capacity(resource_file_entries.len());
    let mut filename_to_resource_entry = Vec::with_capacity(total_entries);

    let mut name_buf = [0; CHUNK_NAME_SIZE];

    // Create a lookup of filename to resource file + offset
    for (resource_id, (resource_name, entries)) in resource_file_entries.into_iter().enumerate() {
        let path = Path::new(root_directory).join(resource_name);
        let mut f = File::open(path)?;
        for (hash, offset) in entries {
            f.seek(io::SeekFrom::Start(offset as u64))?;
            let (name, payload_size) = read_header(&mut f, &mut name_buf)?;
            
            // TODO - use stream_position once it become stable
            let payload_offset: u64 = f.seek(io::SeekFrom::Current(0))?;

            filename_to_resource_entry.push((name.into(), ResourceEntry {
                resource_id,
                payload_offset,
                payload_size
            }));
        }
        resource_files.push(f);
    }

    Ok(Resources {
        resource_files: resource_files,
        filename_to_resource_entry: filename_to_resource_entry,
    })
}

pub fn parse_tags<'a, F>(buf: &'a [u8], mut on_tag: F)
    where F: FnMut(&str, &'a [u8])
{
    let mut buf = buf;
    let tag_buf = [0; 4];
    while buf.len() >= 8 {
        let tag_buf = &buf[0..4];
        if tag_buf[3] != 0x3A {
            // 0x3A is a colon (:) in ascii
            // ERROR - tag should end with a colon
            return;
        }
        for &c in &tag_buf[0..3] {
            if !(c >= 0x30 && c <= 0x7A) {
                // ERROR - tag should be ascii-ish
                return;
            }
        }
        
        let tag = str::from_utf8(tag_buf).unwrap();

        // Don't know why bit 31 is sometimes set. I've observed it on top-level tags.
        let size = u32::from_le_bytes(buf[4..8].try_into().unwrap()) & 0x7FFFFFFF;

        buf = &buf[8..];

        let payload = &buf[0..size as usize];

        on_tag(tag, payload);

        buf = &buf[size as usize..];
    }
}

fn parse_bmp_tag_inf(buf: &[u8]) -> Option<Vec<(u16, u16)>> {
    let mut buf = buf;

    if buf.len() < 2 { return None; }

    let count = u16::from_le_bytes(buf[0..2].try_into().unwrap());
    buf = &buf[2..];

    if buf.len() < (count*4) as usize { return None; }

    let mut dims = Vec::with_capacity(count as usize);

    for _ in 0..count {
        let width = u16::from_le_bytes(buf[0..2].try_into().unwrap());
        let height = u16::from_le_bytes(buf[2..4].try_into().unwrap());
        dims.push((width, height));
        buf = &buf[4..];
    }

    Some(dims)
}

fn parse_bmp_tag_off(buf: &[u8]) -> Option<Vec<u32>> {
    let mut buf = buf;

    let count = buf.len() / 4;

    let mut offs = Vec::with_capacity(count);

    for _ in 0..count {
        let offset = u32::from_le_bytes(buf[0..4].try_into().unwrap());
        offs.push(offset);
        buf = &buf[4..];
    }

    Some(offs)
}

#[derive(Debug)]
pub struct BmpScnSlice<'a> {
    pub dimensions: (u16, u16),
    pub scn: &'a [u8]
}

// Returns a vector of bitmap slices. The bitmap data is not decoded.
// Note that the "scn" buffer may include more than is needed to decode the bitmap.
pub fn parse_bmp_scn(buf: &[u8]) -> Option<Vec<BmpScnSlice>> {
    let mut inf = None;
    let mut scn = None;
    let mut off = None;

    parse_tags(buf, |tag, bmp_data| {
        if tag == "BMP:" {
            parse_tags(bmp_data, |subtag, subdata| {
                match subtag {
                    "INF:" => { inf = parse_bmp_tag_inf(subdata); },
                    "SCN:" => { scn = Some(subdata); },
                    "OFF:" => { off = parse_bmp_tag_off(subdata); },
                    _ => ()
                }
            })
        }
    });

    if let (Some(inf), Some(scn), Some(off)) = (inf, scn, off) {
        if inf.len() == off.len() {
            Some(inf.into_iter().zip(off.into_iter()).map(|(dim, offset)| {
                BmpScnSlice {
                    dimensions: dim,
                    scn: &scn[offset as usize..]
                }
            }).collect())
        } else {
            None
        }
    } else {
        None
    }
}

fn parse_scr_tag_dim(buf: &[u8]) -> Option<(u16, u16)> {
    if buf.len() < 4 {
        None
    } else {
        let width = u16::from_le_bytes(buf[0..2].try_into().unwrap());
        let height = u16::from_le_bytes(buf[2..4].try_into().unwrap());

        Some((width, height))
    }
}

pub fn parse_scr(buf: &[u8]) -> Option<((u16, u16), &[u8])> {
    let mut dim = None;
    let mut bin = None;

    parse_tags(buf, |tag, scr_data| {
        if tag == "SCR:" {
            parse_tags(scr_data, |subtag, subdata| {
                match subtag {
                    "DIM:" => { dim = parse_scr_tag_dim(subdata); },
                    "BIN:" => { bin = Some(subdata); },
                    _ => ()
                }
            })
        }
    });

    if let (Some(dim), Some(bin)) = (dim, bin) {
        Some((dim, bin))
    } else {
        None
    }
}

fn vga_6bit_to_8bit(x: u8) -> u8 {
    (x << 2) | ((x & 0x30) >> 4)
}

pub fn parse_vga_palette_as_rgba<'a>(buf: &[u8], workbuf: &'a mut [[u8;4];256]) -> Option<&'a [[u8;4];256]> {
    let mut parsed = false;

    parse_tags(buf, |tag, data| {
        if tag == "PAL:" {
            parse_tags(data, |tag, data| {
                if tag == "VGA:" {
                    if data.len() >= 256*3 {
                        for i in 0..256 {
                            workbuf[i][0] = vga_6bit_to_8bit(data[i*3+0]);
                            workbuf[i][1] = vga_6bit_to_8bit(data[i*3+1]);
                            workbuf[i][2] = vga_6bit_to_8bit(data[i*3+2]);
                            workbuf[i][3] = 255;
                        }
                        parsed = true;
                    }
                }
            });
        }
    });
    
    if parsed {
        Some(workbuf)
    } else {
        None
    }
}

mod tests {
    use super::hash_file_strip_chars;
    use super::hash_filename;

    #[test]
    fn hash_file_strip_chars_test() {
        assert_eq!(hash_file_strip_chars(b""), b"");
        assert_eq!(hash_file_strip_chars(b":"), b"");
        assert_eq!(hash_file_strip_chars(b"hello world"), b"hello world");
        assert_eq!(hash_file_strip_chars(b"hello\\world"), b"world");
        assert_eq!(hash_file_strip_chars(b"hello\\foo:::\\:world"), b"world");
    }

    #[test]
    fn hash_temim_samples() {
        let hash_indexes = [0,1,5,7];
        assert_eq!(hash_filename(&hash_indexes, b"PART41.BMP"), 0x5041389D);
        assert_eq!(hash_filename(&hash_indexes, b"PART42.BMP"), 0x50413202);
        assert_eq!(hash_filename(&hash_indexes, b"TIM.SX"),     0x5449261F);
    }
}
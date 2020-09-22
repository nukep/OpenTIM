// A mutable buffer that gets "eaten" like a snake.
// Unwritten data is mutable (the head), and written data becomes immutable (the tail).
pub struct BufferSnake<'a> {
    buf_ptr: *mut u8,
    buf_size: usize,
    idx: usize,
    _phantom: std::marker::PhantomData<&'a mut u8>
}

impl<'a> BufferSnake<'a> {
    pub fn new(buf: &'a mut [u8]) -> Self {
        BufferSnake {
            buf_ptr: buf.as_mut_ptr(),
            buf_size: buf.len(),
            idx: 0,
            _phantom: std::marker::PhantomData
        }
    }

    pub fn tail(&self) -> &'a [u8] {
        unsafe {
            std::slice::from_raw_parts(self.buf_ptr, self.idx)
        }
    }

    pub fn remaining(&self) -> usize {
        self.buf_size - self.idx
    }

    pub fn try_write(&mut self, data: &[u8]) -> bool {
        let size = data.len();

        if self.remaining() < size {
            // Not enough remaining space
            false
        } else {
            unsafe {
                std::ptr::copy_nonoverlapping(data.as_ptr(), self.buf_ptr.add(self.idx), size);
            }

            self.idx += size;

            true
        }
    }
}

#[test]
fn buffer_snake_test() {
    let mut data = vec![0; 8];

    let mut snake = BufferSnake::new(&mut data);
    assert_eq!(snake.tail(), &[]);

    snake.try_write(&[1, 2]);
    snake.try_write(&[3, 4, 5]);
    assert_eq!(snake.tail(), &[1, 2, 3, 4, 5]);
    snake.try_write(&[]);
    assert_eq!(snake.tail(), &[1, 2, 3, 4, 5]);
    snake.try_write(&[6, 7, 8]);
    assert_eq!(snake.tail(), &[1, 2, 3, 4, 5, 6, 7, 8]);
    snake.try_write(&[]);

    assert_eq!(data, &[1, 2, 3, 4, 5, 6, 7, 8]);
}
// { dg-additional-options "-w" }
extern "rust-intrinsic" {
    pub fn offset<T>(dst: *const T, offset: isize) -> *const T;
}

struct FatPtr<T> {
    data: *const T,
    len: usize,
}

union Repr<T> {
    rust: *const [T],
    rust_mut: *mut [T],
    raw: FatPtr<T>,
}

impl<T> *const [T] {
    pub const fn len(self) -> usize {
        // SAFETY: this is safe because `*const [T]` and `FatPtr<T>` have the same layout.
        // Only `std` can make this guarantee.
        let a = unsafe { Repr { rust: self }.raw };
        a.len
    }

    pub const fn as_ptr(self) -> *const T {
        self as *const T
    }
}

impl<T> *const T {
    pub const unsafe fn offset(self, count: isize) -> *const T {
        unsafe { offset(self, count) }
    }

    pub const unsafe fn add(self, count: usize) -> Self {
        unsafe { self.offset(count as isize) }
    }

    pub const fn as_ptr(self) -> *const T {
        self as *const T
    }
}

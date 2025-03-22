#![no_std]
use core::arch::asm;
use core::panic::PanicInfo;

unsafe extern "C" {
    pub fn k_printf(fmt: *const u8, ...) -> i32;
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn rust_print() {
    let msg = b"HELP THE RUST PROPAGANDA HAS INVADED OUR KERNEL OH NOOOOO\n\0";
    unsafe {
        k_printf(msg.as_ptr());
    }
}

static COMPLETE_PANIC: &str = "Panic! %s:%d - %s\n\0";
static INCOMPLETE_PANIC: &str = "Panic! %s:%d - no message\n\0";
static EMPTY_PANIC: &str = "Panic! No known location\n\0";

#[panic_handler]
unsafe fn panic(info: &PanicInfo) -> ! {
    if let Some(location) = info.location() {
        let file = location.file();
        let line = location.line();

        if !info.message().as_str().is_some_and(|v| v.is_empty()) {
            unsafe {
                k_printf(
                    COMPLETE_PANIC.as_ptr(),
                    file,
                    line,
                    info.message().as_str().unwrap_or_else(|| "empty").as_ptr(),
                );
            }
        } else {
            unsafe {
                k_printf(INCOMPLETE_PANIC.as_ptr(), file, line);
            }
        }
    } else {
        unsafe {
            k_printf(EMPTY_PANIC.as_ptr());
        }
    }

    loop {
        unsafe {
            asm!("hlt");
        }
    }
}

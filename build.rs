
fn main() {
    let c_sources = &["foo.c", "globals.c", "main.c", "part_defs.c", "draw_rope.c"];

    let mut builder = cc::Build::new();

    for filename in c_sources {
        builder.file(format!("c_src/{}", filename));
    }

    builder.compile("tim_c");
}
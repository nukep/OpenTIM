
fn main() {
    let c_sources = &["foo.c"];

    let mut builder = cc::Build::new();

    for filename in c_sources {
        builder.file(format!("c_src/{}", filename));
    }

    builder.compile("foo");
}
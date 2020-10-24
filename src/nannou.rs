use nannou::prelude::*;
use nannou::image;
use std::collections::HashMap;
use crate::tim_c;
use crate::parts;
use crate::part::PartType;
use crate::debug;
use num_enum::TryFromPrimitive;
use std::convert::TryFrom;

const SCREEN_WIDTH: u32 = 640;
const SCREEN_HEIGHT: u32 = 480;

pub fn start() {
    nannou::app(model).update(update).run();
}

/// Corresponds to NEWMOUSE.BMP in TEMIM DOS (TIM DOS doesn't have that file).
#[derive(Debug, Hash, Eq, PartialEq, TryFromPrimitive)]
#[repr(u8)]
enum MouseIcon {
    Default = 0,
    Wait,
    Pick,
    Recycle,
    FlipHorz,
    FlipVert,
    StretchHorz,
    StretchVert,

    Lock = 8,
    Belt,
    Rope,
    WhiteDottedHorz,
    WhiteDottedVert,
    GreenDottedHorz,
    GreenDottedVert,
    RedDottedHorz,
    RedDottedVert = 16,

    Play,
    Volume,
    RestartLevel,
    SelectPuzzle,
    FreeFormMachine,
    Quit,
    Save,

    Load = 24,
    Gravity,
    AirPressure,
    StartMachine,

    // The following are icons without a cursor graphic
    TrashIcon,
    StretchHorzIcon,
    StretchVertIcon,
    FlipHorzIcon,
    FlipVertIcon = 32,
    LockedIcon,
    UnlockedIcon,

    // The following have a cursor graphic again
    Paused,
    Goal,
    Adjust,
    More,
    Clear
}

#[derive(Hash, Eq, PartialEq)]
enum ImageId {
    Part(u32, usize),
    PartIcon(u32),
    Mouse(MouseIcon),
    Misc(String, usize)
}

impl ImageId {
    pub fn new(s: &str, slice_idx: usize) -> Self {
        if s == "ICONS.BMP" {
            return ImageId::PartIcon(slice_idx as u32);
        }
        if s == "NEWMOUSE.BMP" {
            let v = MouseIcon::try_from(slice_idx as u8).unwrap();
            return ImageId::Mouse(v);
        }

        if s.starts_with("PART") && s.ends_with(".BMP") {
            let int_s = &s[4..(s.len()-4)];
            if let Ok(n) = int_s.parse() {
                return ImageId::Part(n, slice_idx);
            }
        }

        ImageId::Misc(s.into(), slice_idx)
    }
}

#[derive(Copy, Clone)]
enum Flip {
    None,
    Vertical,
    Horizontal,
    Both
}

enum RenderItem {
    Image {
        id: ImageId,
        x: i32,
        y: i32,
        flip: Flip
    },
    Rope {
        x1: i32,
        y1: i32,
        x2: i32,
        y2: i32,
        sag: i32
    },
    Belt {
        x1: i32,
        y1: i32,
        width1: i32,
        x2: i32,
        y2: i32,
        width2: i32
    },
    Text {
        x: i32,
        y: i32,
        text: String
    }
}

struct Model {
    textures: HashMap<ImageId, wgpu::Texture>,
    ticks: u32,
    render_items: Vec<RenderItem>,
    
    clicked: bool,
    show_borders: bool,

    mouse_pos: Option<(i32, i32)>,
}

fn model(app: &App) -> Model {
    // The DX12 backend seems to have a bug where all textures are drawn the same.
    // So this is Vulkan-only for now.
    let window_id = app.new_window()
        .backends(wgpu::BackendBit::VULKAN)
        .title("OpenTIM")
        .size(SCREEN_WIDTH, SCREEN_HEIGHT)
        .view(view)
        .event(event)
        .build().unwrap();
    let window = app.window(window_id).unwrap();
    // window.set_cursor_visible(false);

    let mut textures = HashMap::new();

    super::load_images(&mut |filename, slice_idx, width, height, buf| {
        let image = image::RgbaImage::from_raw(width, height, buf).unwrap();
        let dynimg = image::DynamicImage::ImageRgba8(image);
        let texture = wgpu::Texture::from_image(&window, &dynimg);



        // println!("{} {} {}x{}", filename, slice_idx, width, height);

        textures.insert(ImageId::new(filename, slice_idx), texture);
    }).unwrap();

    Model { textures, ticks: 0, render_items: vec![], mouse_pos: None, clicked: false, show_borders: false }
}

// Handle events related to the window and update the model if necessary
fn event(_app: &App, model: &mut Model, event: WindowEvent) {
    // println!("{:?}", event);
    match event {
        MouseMoved(Vector2 { x, y }) => {
            model.mouse_pos = Some(((x + SCREEN_WIDTH as f32 / 2.0) as i32, (-y + SCREEN_HEIGHT as f32 / 2.0) as i32));
            // println!("{:?}", model.mouse_pos);
        },
        MouseExited => {
            model.mouse_pos = None;
        },
        KeyPressed(Key::Space) => {
            model.clicked = !model.clicked;
        },
        KeyPressed(Key::B) => {
            model.show_borders = !model.show_borders;
        },
        KeyPressed(Key::G) => {
            debug::dump_level_to_graphviz_file("out.gv").unwrap();
        },
        _ => ()
    }
}

fn update(app: &App, model: &mut Model, _update: Update) {
    if model.clicked && model.ticks % 2 == 0 {
        unsafe {
            tim_c::advance_parts();
            tim_c::all_parts_set_prev_vars();
        }
    }

    // where the first layer is above everything else
    let mut layers = vec![];
    for _ in 0..6 {
        layers.push(vec![]);
    }

    // TIM logo
    // render_items.push(RenderItem::Image { id: ImageId::Part(56, 0), x: 0, y: 0 });
    // render_items.push(RenderItem::Image { id: ImageId::Part(56, 1), x: 26, y: 0 });
    // render_items.push(RenderItem::Image { id: ImageId::Part(56, 2), x: 26+96, y: 0 });
    // render_items.push(RenderItem::Image { id: ImageId::Part(56, 3), x: 26+96+166, y: 0 });
    // render_items.push(RenderItem::Image { id: ImageId::Part(56, 4), x: 0, y: 67 });
    // render_items.push(RenderItem::Image { id: ImageId::Part(56, 5), x: 127, y: 67 });
    // render_items.push(RenderItem::Image { id: ImageId::Part(56, 6), x: 127+154, y: 67 });

    
    {
        let iter = unsafe { tim_c::static_parts_iter().chain(tim_c::moving_parts_iter()) };
        for part in iter {
            let part_type = PartType::from_u16(part.part_type);
            let mut render_items = &mut layers[parts::get_def(part_type).goobers.0 as usize];

            match part_type {
                PartType::Belt => {
                    if let Some(((x1, y1, width1), (x2, y2, width2))) = part.belt_section() {
                        render_items.push(RenderItem::Belt {
                            x1: x1 as i32,
                            y1: y1 as i32,
                            width1: width1 as i32,
                            x2: x2 as i32,
                            y2: y2 as i32,
                            width2: width2 as i32,
                        });
                    }
                },

                PartType::Rope => {
                    if let Some(sections) = part.rope_sections() {
                        for ((x1, y1), (x2, y2), sag) in sections.into_iter() {
                            render_items.push(RenderItem::Rope {
                                x1: x1 as i32,
                                y1: y1 as i32,
                                x2: x2 as i32,
                                y2: y2 as i32,
                                sag: sag as i32
                            });
                        }
                    }
                },

                PartType::BrickWall | PartType::DirtWall | PartType::WoodWall | PartType::PipeStraight => {
                    let start_x = part.pos_render.x as i32;
                    let start_y = part.pos_render.y as i32;
                    if part.size.x == 16 {
                        // vertical vall

                        let count = (part.size.y / 16) as i32;

                        render_items.push(RenderItem::Image { id: ImageId::Part(part.part_type as u32, 4), x: start_x, y: start_y, flip: Flip::None });
                        for i in 0..count-2 {
                            let image = if i % 2 == 0 { 5 } else { 6 };
                            render_items.push(RenderItem::Image { id: ImageId::Part(part.part_type as u32, image), x: start_x, y: start_y + (i+1)*16, flip: Flip::None });
                        }
                        render_items.push(RenderItem::Image { id: ImageId::Part(part.part_type as u32, 7), x: start_x, y: start_y + (count-1)*16, flip: Flip::None });
                    } else {
                        // horizontal wall

                        let count = (part.size.x / 16) as i32;

                        render_items.push(RenderItem::Image { id: ImageId::Part(part.part_type as u32, 0), x: start_x, y: start_y, flip: Flip::None });
                        for i in 0..count-2 {
                            let image = if i % 2 == 0 { 1 } else { 2 };
                            render_items.push(RenderItem::Image { id: ImageId::Part(part.part_type as u32, image), x: start_x + (i+1)*16, y: start_y, flip: Flip::None });
                        }
                        render_items.push(RenderItem::Image { id: ImageId::Part(part.part_type as u32, 3), x: start_x + (count-1)*16, y: start_y, flip: Flip::None });
                    }
                },

                _ => {
                    if part.flags2 & 0x2000 == 0 {

                        let flip = match ((part.flags2 & 0x10) != 0, (part.flags2 & 0x20) != 0) {
                            (false, false) => Flip::None,
                            (true, false) => Flip::Horizontal,
                            (false, true) => Flip::Vertical,
                            (true, true) => Flip::Both
                        };

                        if let Some(&part_images) = parts::get_def(part_type).render_images.and_then(|l| l.get(part.state1 as usize)) {
                            // This part renders multiple images together

                            let part_x = part.pos.x as i32;
                            let part_y = part.pos.y as i32;
                            for &(goober, index, x, y) in part_images {
                                // TODO - flipping. use size_something to figure positions out.

                                let mut render_items = &mut layers[goober as usize];

                                let x = x as i32;
                                let y = y as i32;
                                render_items.push(RenderItem::Image { id: ImageId::Part(part.part_type as u32, index as usize), x: part_x+x, y: part_y+y, flip: flip });
                            }
                        } else {
                            // This part renders a single image

                            let part_x = part.pos_render.x as i32;
                            let part_y = part.pos_render.y as i32;
                            render_items.push(RenderItem::Image { id: ImageId::Part(part.part_type as u32, part.state1 as usize), x: part_x, y: part_y, flip: flip });
                        }
                    }
                }
            }
        }
    }

    let mut render_items = vec![];

    for layer_items in layers.iter_mut().rev() {
        render_items.append(layer_items);
    }

    if let Some((x, y)) = model.mouse_pos {
        render_items.push(RenderItem::Image { id: ImageId::Mouse(MouseIcon::Default), x: x, y: y, flip: Flip::None });
    }

    model.render_items = render_items;

    model.ticks = model.ticks.wrapping_add(1);
}

#[inline(always)]
fn lerp(a: f32, b: f32, p: f32) -> f32 {
    (b-a)*p + a
}

/// Return an iterator of points along a quadratic bezier curve
fn quad_bezier_curve_iter(p0: (f32, f32), p1: (f32, f32), p2: (f32, f32), iters: u32) -> impl Iterator<Item = (f32, f32)> + Clone {
    (0..=iters).map(move |i| {
        let t = (i as f32) / (iters as f32);
        let x = lerp(lerp(p0.0, p1.0, t), lerp(p1.0, p2.0, t), t);
        let y = lerp(lerp(p0.1, p1.1, t), lerp(p1.1, p2.1, t), t);
        (x, y)
    })
}

fn view(app: &App, model: &Model, frame: Frame) {
    let win = app.window_rect();
    let t = app.time;

    // Crispy pixels!
    let sampler = wgpu::SamplerBuilder::new()
        // .min_filter(wgpu::FilterMode::Nearest)
        // .mag_filter(wgpu::FilterMode::Nearest)
        .into_descriptor();

    let draw = app.draw().sampler(sampler);

    // draw.background().color(MIDNIGHTBLUE);
    draw.background().rgb(0.0, 160.0/255.0, 160.0/255.0);
    // draw.background().rgb(1.0, 1.0, 1.0);

    let transform = |x: f32, y: f32| -> (f32, f32) {
        (x - (SCREEN_WIDTH as f32 / 2.0), - (y - (SCREEN_HEIGHT as f32 / 2.0)))
    };

    if model.show_borders {
        let iter = unsafe { tim_c::static_parts_iter().chain(tim_c::moving_parts_iter()) };
        for part in iter {
            let part_x = part.pos_x_hi_precision as f32 / 512.0;
            let part_y = part.pos_y_hi_precision as f32 / 512.0;

            // Draw shape origin
            {
                let (x, y) = transform(part_x, part_y);
                draw.ellipse().color(BLACK).x_y(x, y).w_h(5.0, 5.0);
            }

            // Draw bounding rectangle(s)

            {
                let ox = part.pos_render.x as f32;
                let oy = part.pos_render.y as f32;
                let (x1, y1) = transform(ox, oy);
                let (x2, y2) = transform(ox + part.size.x as f32, oy + part.size.y as f32);
                let points = vec![pt2(x1, y1), pt2(x2, y1), pt2(x2, y2), pt2(x1, y2), pt2(x1, y1)];
                draw.polyline().rgba8(255,0,0,128).points(points.into_iter());
            }

            {
                let ox = part_x;
                let oy = part_y;
                let (x1, y1) = transform(ox, oy);
                let (x2, y2) = transform(ox + part.size_something2.x as f32, oy + part.size_something2.y as f32);
                let points = vec![pt2(x1, y1), pt2(x2, y1), pt2(x2, y2), pt2(x1, y2), pt2(x1, y1)];
                draw.polyline().rgba8(0,0,255,128).points(points.into_iter());
            }

            // Draw shape border

            let border_iter = part.border_points().iter().map(|p| {
                (part_x + p.x as f32, part_y + p.y as f32)
            }).map(|(x, y)|
                transform(x, y)
            ).map(|(x, y)|
                pt2(x, y)
            );
            draw.polygon().rgba8(0,0,0,64).points(border_iter);

            // Draw normals

            // Big crazy iterator that yields a pair of the border point and the next one. (point_n, point_n+1).
            let niter = part.border_points().iter().zip(part.border_points().iter().cycle().skip(1)).take(part.border_points().iter().count());

            for (a, b) in niter {
                let normal = a.normal_angle as f32 / 65536.0;
                let ox = part_x + (a.x as f32 + b.x as f32) / 2.0;
                let oy = part_y + (a.y as f32 + b.y as f32) / 2.0;

                let s = f32::sin(normal*3.141592*2.0) * 3.0;
                let c = f32::cos(normal*3.141592*2.0) * 3.0;

                let (ox, oy) = transform(ox, oy);

                draw.line().color(BLACK).points(pt2(ox, oy), pt2(ox-s, oy+c));
            }
        }
    }

    if !model.show_borders {
        for item in model.render_items.iter() {
            match item {
                RenderItem::Image { id, x, y, flip } => {
                    if let Some(t) = model.textures.get(id) {
                        let [w, h] = t.size();
                        let (sx, sy) = transform(*x as f32 + w as f32 / 2.0, *y as f32 + h as f32 / 2.0);
                        match *flip {
                            Flip::None =>       draw.texture(t).x_y(sx, sy),
                            Flip::Horizontal => draw.texture(t).w_h(-(w as f32), h as f32).x_y(sx, sy),
                            Flip::Vertical   => draw.texture(t).w_h(w as f32,    -(h as f32)).x_y(sx, sy),
                            Flip::Both       => draw.texture(t).w_h(-(w as f32), -(h as f32)).x_y(sx, sy),
                        };
                    }
                },
                &RenderItem::Rope { x1, y1, x2, y2, sag } => {
                    let x1 = x1 as f32;
                    let y1 = y1 as f32;
                    let x2 = x2 as f32;
                    let y2 = y2 as f32;
                    let sag = sag as f32;
                    let verts = quad_bezier_curve_iter((x1, y1), ((x1+x2)/2.0, (y1+y2)/2.0 + sag), (x2, y2), 10);
                    let points_iter = verts.map(|(x, y)| transform(x, y)).map(|(x, y)| pt2(x, y));

                    // Black outline
                    draw.polyline().weight(4.0).rgba8(0,0,0,255).points(points_iter.clone());
                    // Rope color
                    draw.polyline().weight(2.0).rgba8(240,176,0,255).points(points_iter);
                },
                &RenderItem::Belt { x1, y1, width1, x2, y2, width2 } => {
                    let x1 = x1 as f32;
                    let y1 = y1 as f32;
                    let width1 = width1 as f32;
                    let x2 = x2 as f32;
                    let y2 = y2 as f32;
                    let width2 = width2 as f32;

                    let angle = f32::atan2(y2-y1, x2-x1);
                    let ss = f32::sin(angle);
                    let cc = -f32::cos(angle);
                    let x1c = x1+width1/2.0;
                    let y1c = y1+width1/2.0;
                    let x2c = x2+width2/2.0;
                    let y2c = y2+width2/2.0;

                    let x1a = x1c - ss * width1/2.0;
                    let y1a = y1c - cc * width1/2.0;
                    let x2a = x2c - ss * width2/2.0;
                    let y2a = y2c - cc * width2/2.0;
                    let x1b = x1c + ss * width1/2.0;
                    let y1b = y1c + cc * width1/2.0;
                    let x2b = x2c + ss * width2/2.0;
                    let y2b = y2c + cc * width2/2.0;

                    let (x1a, y1a) = transform(x1a, y1a);
                    let (x2a, y2a) = transform(x2a, y2a);
                    let (x1b, y1b) = transform(x1b, y1b);
                    let (x2b, y2b) = transform(x2b, y2b);

                    draw.line().weight(2.0).rgba8(0,0,0,255).points(pt2(x1a, y1a), pt2(x2a, y2a));
                    draw.line().weight(2.0).rgba8(0,0,0,255).points(pt2(x1b, y1b), pt2(x2b, y2b));

                },
                RenderItem::Text { x, y, text } => {
                    let (x, y) = transform(*x as f32, *y as f32);
                    draw.text(text).x_y(x, y).left_justify().align_text_top().color(BLACK);
                },
            }
        }
    }

    draw.to_frame(app, &frame).unwrap();
}
use nannou::prelude::*;
use nannou::image;
use std::collections::HashMap;
use crate::resource_dos;
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

enum RenderItem {
    Image {
        id: ImageId,
        x: i32,
        y: i32
    },
    Rope {
        x1: i32,
        y1: i32,
        x2: i32,
        y2: i32,
        sag: u32
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
    balloon_vel_y: i32,
    balloon_y: i32,

    mouse_pos: Option<(i32, i32)>,
}

fn model(app: &App) -> Model {
    println!("{:?}", MouseIcon::FlipVert as u32);

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
    window.set_cursor_visible(false);

    let mut textures = HashMap::new();

    super::load_images(&mut |filename, slice_idx, width, height, buf| {
        let image = image::RgbaImage::from_raw(width, height, buf).unwrap();
        let dynimg = image::DynamicImage::ImageRgba8(image);
        let texture = wgpu::Texture::from_image(&window, &dynimg);



        // println!("{} {} {}x{}", filename, slice_idx, width, height);

        textures.insert(ImageId::new(filename, slice_idx), texture);
    }).unwrap();

    Model { textures, ticks: 0, render_items: vec![], balloon_vel_y: 0, balloon_y: 480<<9, mouse_pos: None }
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
        }
        _ => ()
    }
}

fn update(app: &App, model: &mut Model, _update: Update) {
    let mut render_items = vec![];

    // TIM logo
    render_items.push(RenderItem::Image { id: ImageId::Part(56, 0), x: 0, y: 0 });
    render_items.push(RenderItem::Image { id: ImageId::Part(56, 1), x: 26, y: 0 });
    render_items.push(RenderItem::Image { id: ImageId::Part(56, 2), x: 26+96, y: 0 });
    render_items.push(RenderItem::Image { id: ImageId::Part(56, 3), x: 26+96+166, y: 0 });
    render_items.push(RenderItem::Image { id: ImageId::Part(56, 4), x: 0, y: 67 });
    render_items.push(RenderItem::Image { id: ImageId::Part(56, 5), x: 127, y: 67 });
    render_items.push(RenderItem::Image { id: ImageId::Part(56, 6), x: 127+154, y: 67 });

    let balloon_y = model.balloon_y >> 9;

    render_items.push(RenderItem::Image { id: ImageId::Part(4, 0), x: 100, y: balloon_y });
    render_items.push(RenderItem::Image { id: ImageId::Part(23, 0), x: 200-8, y: 400-8 });
    render_items.push(RenderItem::Rope { x1: 100+16, y1: balloon_y + 48, x2: 200, y2: 400, sag: 100 });
    render_items.push(RenderItem::Text { x: 0, y: 0, text: "Hello World!".into() });
    

    if let Some((x, y)) = model.mouse_pos {
        render_items.push(RenderItem::Image { id: ImageId::Mouse(MouseIcon::Default), x: x, y: y });
    }

    model.render_items = render_items;

    model.ticks = model.ticks.wrapping_add(1);
    model.balloon_y += model.balloon_vel_y;
    model.balloon_vel_y = std::cmp::max(model.balloon_vel_y.wrapping_sub(25), -2000);
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

    let transform = |x: f32, y: f32| -> (f32, f32) {
        (x - (SCREEN_WIDTH as f32 / 2.0), - (y - (SCREEN_HEIGHT as f32 / 2.0)))
    };

    for item in model.render_items.iter() {
        match item {
            RenderItem::Image { id, x, y } => {
                if let Some(t) = model.textures.get(id) {
                    let [w, h] = t.size();
                    let (sx, sy) = transform(*x as f32 + w as f32 / 2.0, *y as f32 + h as f32 / 2.0);
                    draw.texture(t)
                    .x_y(sx, sy);
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
            RenderItem::Text { x, y, text } => {
                let (x, y) = transform(*x as f32, *y as f32);
                draw.text(text).x_y(x, y).left_justify().align_text_top().color(BLACK);
            },
        }
    }

    draw.to_frame(app, &frame).unwrap();
}
use num_enum::IntoPrimitive;
use num_enum::TryFromPrimitive;
use std::convert::TryFrom;

#[derive(Copy, Clone, Debug, Eq, PartialEq, IntoPrimitive, TryFromPrimitive)]
#[repr(u16)]
pub enum PartType {
    BowlingBall = 0,
    BrickWall,
    Incline,
    TeeterTotter,
    Balloon,
    Conveyor,
    MortTheMouseCage,
    Pulley,
    Belt,
    Basketball,
    Rope,
    Cage,
    PokeyTheCat,
    JackInTheBox,
    Gear,
    BobTheFish,
    Bellows,
    Bucket,
    Cannon,
    Dynamite,
    GunBullet,
    LightSwitchOutlet,
    DynamiteWithPlunger,
    EyeHook,
    Fan,
    Flashlight,
    Generator,
    Gun,
    Baseball,
    Lightbulb,
    MagnifyingGlass,
    KellyTheMonkey,
    JackOLantern,
    HeartBalloon,
    ChristmasTree,
    BoxingGlove,
    Rocket,
    Scissors,
    SolarPanels,
    Trampoline,
    Windmill,
    Explosion,
    MortTheMouse,
    CannonBall,
    TennisBall,
    Candle,
    PipeStraight,
    PipeCurved,
    WoodWall,
    RopeSeveredEnd,
    ElectricEngine,
    Vacuum,
    Cheese,
    Nail,
    MelSchlemming,
    TitleTheEvenMore,
    TitleIncredibleMachine,
    TitleCredits,
    MelsHouse,
    SuperBall,
    DirtWall,
    ErnieTheAlligator,
    Teapot,
    Eightball,
    PinballBumper,
    LuckyClover,
}

impl PartType {
    pub fn try_from_u16(v: u16) -> Option<PartType> {
        PartType::try_from(v).ok()
    }

    pub fn from_u16(v: u16) -> PartType {
        PartType::try_from(v).unwrap()
    }

    pub fn to_u16(self) -> u16 {
        self.into()
    }
}
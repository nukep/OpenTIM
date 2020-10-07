/// Partial from TIMWIN: 1090:0000
/// Returns a value from 0 to 1024 inclusive.
#[inline(always)]
fn calculate_adj_grav(gravity: u16) -> u16 {
    if gravity < 140 {
        gravity/4 + 1
    } else if gravity > 278 {
        gravity*2
    } else {
        gravity
    }
}

/// Partial from TIMWIN: 1090:0000
/// Returns a value from 0 to 2048 inclusive.
#[inline(always)]
fn calculate_adj_air(air_pressure: u16) -> u16 {
    if air_pressure < 70 {
        air_pressure/2
    } else {
        air_pressure*16
    }
}

/// Partial from TIMWIN: 1090:0000
pub fn calculate_acceleration(gravity: u16, air_pressure: u16, density: u16) -> i16 {
    let adj_air = calculate_adj_air(air_pressure);

    if density == adj_air {
        return 0;
    }

    let adj_grav = calculate_adj_grav(gravity);

    if density > adj_air {
        (adj_grav as i32 - (adj_air as i32 * adj_grav as i32) / density as i32) as i16
    } else {
        -(adj_grav as i32 - (density as i32 * adj_grav as i32) / adj_air as i32) as i16
    }
}

/// Partial from TIMWIN: 1090:0000
pub fn calculate_terminal_velocity(air_pressure: u16) -> i16 {
    let adj_air = calculate_adj_air(air_pressure);

    // If max of adj_air is 2048 (0x0800), then this ranges from 0x1E00 to 0x2600 inclusive.
    0x2600 - adj_air as i16
}

#[cfg(test)]
mod tests {
    use super::calculate_acceleration;
    use super::calculate_terminal_velocity;

    macro_rules! atmotest {
        ($gravity: expr, $air: expr, $expected_term_vel: expr, $fixture: expr) => {
            for &(density, acceleration) in $fixture.iter() {
                assert_eq!(calculate_acceleration($gravity, $air, density), acceleration);
            }
            assert_eq!(calculate_terminal_velocity($air), $expected_term_vel);
        }
    }

    // The following fixtures were taken by inspecting the memory of the Parts table (segment 31) from the original TIMWIN.

    #[test]
    fn earth() {
        atmotest!(272, 67, 9695,
                  [(     0,  -272 ),
                   (     9,  -198 ),
                   (    11,  -182 ),
                   (   100,   183 ),
                   (  1132,   265 ),
                   (  1300,   266 ),
                   (  1322,   266 ),
                   (  1500,   267 ),
                   (  1510,   267 ),
                   (  1600,   267 ),
                   (  1800,   268 ),
                   (  1888,   268 ),
                   (  2000,   268 ),
                   (  2400,   269 ),
                   (  2832,   269 ),
                   (  3776,   270 ),
                   (  4153,   270 ),
                   (  7552,   271 ),
                   ( 14726,   272 ),
                   ( 18000,   272 ),
                   ( 21428,   272 )]);
    }

    #[test]
    fn min_gravity_earth_air() {
        atmotest!(0, 67, 9695,
                  [(     0,    -1 ),
                   (     9,    -1 ),
                   (    11,    -1 ),
                   (   100,     1 ),
                   (  1132,     1 ),
                   (  1300,     1 ),
                   (  1322,     1 ),
                   (  1500,     1 ),
                   (  1510,     1 ),
                   (  1600,     1 ),
                   (  1800,     1 ),
                   (  1888,     1 ),
                   (  2000,     1 ),
                   (  2400,     1 ),
                   (  2832,     1 ),
                   (  3776,     1 ),
                   (  4153,     1 ),
                   (  7552,     1 ),
                   ( 14726,     1 ),
                   ( 18000,     1 ),
                   ( 21428,     1 )]);
    }

    #[test]
    fn max_gravity_earth_air() {
        atmotest!(512, 67, 9695,
                  [(     0, -1024 ),
                   (     9,  -745 ),
                   (    11,  -683 ),
                   (   100,   687 ),
                   (  1132,   995 ),
                   (  1300,   999 ),
                   (  1322,   999 ),
                   (  1500,  1002 ),
                   (  1510,  1002 ),
                   (  1600,  1003 ),
                   (  1800,  1006 ),
                   (  1888,  1007 ),
                   (  2000,  1008 ),
                   (  2400,  1010 ),
                   (  2832,  1013 ),
                   (  3776,  1016 ),
                   (  4153,  1016 ),
                   (  7552,  1020 ),
                   ( 14726,  1022 ),
                   ( 18000,  1023 ),
                   ( 21428,  1023 )]);
    }

    #[test]
    fn earth_gravity_min_air() {
        atmotest!(272, 0, 9728,
                  [(     0,     0 ),
                   (     9,   272 ),
                   (    11,   272 ),
                   (   100,   272 ),
                   (  1132,   272 ),
                   (  1300,   272 ),
                   (  1322,   272 ),
                   (  1500,   272 ),
                   (  1510,   272 ),
                   (  1600,   272 ),
                   (  1800,   272 ),
                   (  1888,   272 ),
                   (  2000,   272 ),
                   (  2400,   272 ),
                   (  2832,   272 ),
                   (  3776,   272 ),
                   (  4153,   272 ),
                   (  7552,   272 ),
                   ( 14726,   272 ),
                   ( 18000,   272 ),
                   ( 21428,   272 )]);
    }

    #[test]
    fn earth_gravity_max_air() {
        atmotest!(272, 128, 7680,
                  [(     0,  -272 ),
                   (     9,  -271 ),
                   (    11,  -271 ),
                   (   100,  -259 ),
                   (  1132,  -122 ),
                   (  1300,  -100 ),
                   (  1322,   -97 ),
                   (  1500,   -73 ),
                   (  1510,   -72 ),
                   (  1600,   -60 ),
                   (  1800,   -33 ),
                   (  1888,   -22 ),
                   (  2000,    -7 ),
                   (  2400,    40 ),
                   (  2832,    76 ),
                   (  3776,   125 ),
                   (  4153,   138 ),
                   (  7552,   199 ),
                   ( 14726,   235 ),
                   ( 18000,   242 ),
                   ( 21428,   247 )]);
    }

    #[test]
    fn min_gravity_min_air() {
        atmotest!(0, 0, 9728,
                  [(     0,     0 ),
                   (     9,     1 ),
                   (    11,     1 ),
                   (   100,     1 ),
                   (  1132,     1 ),
                   (  1300,     1 ),
                   (  1322,     1 ),
                   (  1500,     1 ),
                   (  1510,     1 ),
                   (  1600,     1 ),
                   (  1800,     1 ),
                   (  1888,     1 ),
                   (  2000,     1 ),
                   (  2400,     1 ),
                   (  2832,     1 ),
                   (  3776,     1 ),
                   (  4153,     1 ),
                   (  7552,     1 ),
                   ( 14726,     1 ),
                   ( 18000,     1 ),
                   ( 21428,     1 )]);
    }

    #[test]
    fn max_gravity_max_air() {
        atmotest!(512, 128, 7680,
                  [(     0, -1024 ),
                   (     9, -1020 ),
                   (    11, -1019 ),
                   (   100,  -974 ),
                   (  1132,  -458 ),
                   (  1300,  -374 ),
                   (  1322,  -363 ),
                   (  1500,  -274 ),
                   (  1510,  -269 ),
                   (  1600,  -224 ),
                   (  1800,  -124 ),
                   (  1888,   -80 ),
                   (  2000,   -24 ),
                   (  2400,   151 ),
                   (  2832,   284 ),
                   (  3776,   469 ),
                   (  4153,   520 ),
                   (  7552,   747 ),
                   ( 14726,   882 ),
                   ( 18000,   908 ),
                   ( 21428,   927 )]);
    }
}
use std::time::SystemTime;

pub struct MMIXRng {
    state: u64
}

impl MMIXRng {
    pub fn with_seed(seed: u64) -> MMIXRng {
        MMIXRng{ state: seed }
    }

    pub fn new() -> MMIXRng {
        Self::with_seed(12345678901112131415)
    }

    pub fn with_timestamp() -> MMIXRng {
        let now = SystemTime::now();
        let since_epoch = now.duration_since(SystemTime::UNIX_EPOCH).expect("now() to duration since the Epoch");
        let unix_time_usec = since_epoch.as_micros() as u64;
        Self::with_seed(unix_time_usec)
    }

    pub fn get(&mut self) -> u64 {
        const A: u64 = 6364136223846793005;
        const C: u64 = 1442695040888963407;
        self.state = (self.state.wrapping_add(C)).wrapping_mul(A);
        return self.state
    }
}

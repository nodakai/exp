use rust::MMIXRng;

use std::collections::VecDeque;

#[derive(Debug)]
struct BinTree {
    l: Option<Box<BinTree>>,
    r: Option<Box<BinTree>>
}

impl BinTree {
    fn generate(rng: &mut MMIXRng) -> BinTree {
        let gen = &mut || {
            if ((rng.get() >> 10) & 1) != 0 {
                Box::new(Self::generate(rng)).into()
            } else {
                None
            }
        };
        let l = gen();
        let r = gen();
        BinTree{ l, r }
    }

    fn dfs_depth(&self) -> usize {
        let conv = |obt: &Option<Box<BinTree>> | {
            if let Some(bt) = obt {
                bt.dfs_depth()
            } else {
                0
            }
        };
        let l = conv(&self.l);
        let r = conv(&self.r);
        l.max(r) + 1
    }

    fn bfs_depth(&self) -> usize {
        let mut q = VecDeque::new();
        let mut depth = 1;
        q.push_back((self, depth));
        while let Some((t, d)) = q.pop_back() {
            depth = depth.max(d);
            if let Some(ref l) = t.l {
                q.push_back((&l, d + 1));
            }
            if let Some(ref r) = t.r {
                q.push_back((&r, d + 1));
            }
        }
        depth
    }
}

fn main() {
    let mut rng = if true {
        MMIXRng::with_timestamp()
    } else {
        MMIXRng::new()
    };
    let tree = BinTree::generate(&mut rng);
    println!("tree:{:#?}", tree);
    println!("depth {}", tree.dfs_depth());
    println!("depth {}", tree.bfs_depth());
}

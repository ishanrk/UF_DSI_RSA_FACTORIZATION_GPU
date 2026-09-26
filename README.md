# gpu-nfs

Work in progress.

Setting up an experimental GPU implementation of the expensive stages of
CADO-NFS, following the general approach described in
[Eric Lu's RSA-260 factorization writeup](https://cognition.com/blog/factoring-rsa-260).
This is an independent implementation based on the public description.
Initial work is focused on CADO compatibility, correctness tests, and
reproducible CPU/GPU baselines.

## Current progress

- Official [CADO-NFS](https://gitlab.inria.fr/cado-nfs/cado-nfs) is pinned as a
  submodule at `692ecb7e62f0f3bdab88ee44cc60b8ded0ea1a1b`.
- CMake provides upstream CPU reference targets and optional CUDA detection,
  with configurable architecture `100` for B200.
- Interface smoke tests cover CADO command lines, workunit validation and
  checksums, factor-base generation, and deterministic special-q enumeration.
- The benchmark recorder writes commands, revisions, parameters, timing, and
  relation counts to JSON. Unmeasured GPU fields remain `null`.
- CPU BWC references read CADO's GF(2) matrices and implement packed forward
  and transpose products, 64-column projections, and short Krylov sequences.
- `glas-todo-check` reads CADO special-q lists for 32-bit affine roots.
  An unreduced lattice basis and membership checks provide initial references.
- GPU sieve kernels, CUDA block Wiedemann, `gps1`, filtering changes, GPU
  square root, and coordinator changes are still scaffolding.

RSA-140 is the initial end-to-end correctness target; C155 is the next
comparison target. Neither has been run here. Next are comparisons against
CADO's BWC arithmetic and skew-reduced lattice setup, retaining CADO formats.

## Build and check the CPU reference

Requires CMake, Ninja, GCC/G++, GMP development headers, and Python 3.
CUDA is optional for these CPU checks; B200 builds require CUDA 12.8 or newer.

```sh
git submodule update --init third_party/cado-nfs
cmake -S . -B build -G Ninja
cmake --build build --parallel 2
CMAKE_BUILD_PARALLEL_LEVEL=2 cmake --build build --target cado-smoke
ctest --test-dir build --output-on-failure
```

Validate a CADO-generated todo list with `build/glas-todo-check -todo FILE`.
The smoke target also checks this reader against upstream `las` output.

## Read

- [Eric Lu — Factoring RSA-260](https://cognition.com/blog/factoring-rsa-260)
- [Factorization of RSA-140 using the number field sieve](https://ir.cwi.nl/pub/4524)
- [Comparing the difficulty of factorization and discrete logarithm: a 240-digit experiment](https://arxiv.org/abs/2006.06197)
- [Cofactorization on Graphics Processing Units](https://eprint.iacr.org/2014/397)
- [Iterative Sparse Matrix-Vector Multiplication for Integer Factorization on GPUs](https://link.springer.com/chapter/10.1007/978-3-642-23397-5_41)

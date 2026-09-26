# GPU NFS

Work in progress.

## Implemented

Block Wiedemann CPU references read CADO matrices and compute packed GF(2)
products, transpose products, block projections, and short Krylov sequences.

The siever code reads CADO lists of 32 bit affine special q entries, constructs
an unreduced lattice basis, and checks lattice membership. The todo checker
is tested against lists produced by CADO's `las`.

## Tasks

Ivan Koshkin: compare the GF(2) forward and transpose products in
[gf2.cpp](linalg/bwc/gpu/gf2.cpp) with CADO's CPU BWC on small matrices.
Add a comparison test that checks every output bit, including empty rows.

## To read

1. [Eric Lu, Factoring RSA 260](https://cognition.com/blog/factoring-rsa-260). Describes GPU polynomial selection, lattice sieving, block Wiedemann, filtering, and square root work, with measurements and changes to CADO's pipeline.
2. [Factorization of RSA 140 using the number field sieve](https://ir.cwi.nl/pub/4524). Reports the RSA 140 factorization and explains how improved polynomial selection reduced the computation required relative to estimates based on RSA 130.
3. [Comparing the difficulty of factorization and discrete logarithm: a 240 digit experiment](https://arxiv.org/abs/2006.06197). Compares RSA 240 factorization and a discrete logarithm computation using the same hardware and software, and reports the RSA 250 factorization.
4. [Solving homogeneous linear equations over GF(2) via block Wiedemann algorithm](https://www.ams.org/journals/mcom/1994-62-205/S0025-5718-1994-1192970-7/S0025-5718-1994-1192970-7.pdf). Develops the block Wiedemann method for sparse GF(2) systems, packing vectors into words and using a block recurrence to recover solutions with low storage requirements.
5. [Cofactorization on Graphics Processing Units](https://eprint.iacr.org/2014/397). Moves cofactorization during NFS relation collection to a GPU so CPU workers can concentrate on sieving and produce more useful relations.
6. [Iterative Sparse Matrix Vector Multiplication for Integer Factorization on GPUs](https://link.springer.com/chapter/10.1007/978-3-642-23397-5_41). Presents a CUDA implementation using a hybrid sparse matrix format to accelerate the repeated GF(2) products required by block Wiedemann and block Lanczos.

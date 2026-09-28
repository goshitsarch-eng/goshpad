# Releasing GoshPad

1. Bump `project(goshpad VERSION …)` in `CMakeLists.txt`.
2. Add a `<release version="X.Y.Z" date="…">` entry at the top of
   `data/com.goshapps.GoshPad.metainfo.xml`.
3. Update the `Current release: **X.Y.Z**` line in `README.md`.
4. Update the version strings in `tests/packaging.sh` so `just test` still
   passes.
5. Check the tag locally, then tag and push:

   ```bash
   scripts/check-version.sh vX.Y.Z
   git tag vX.Y.Z
   git push origin main vX.Y.Z
   ```

Packages are produced by the scripts, not by a checked-in CI workflow:

```bash
scripts/package-tarball.sh X.Y.Z x86_64
scripts/package-flatpak.sh X.Y.Z x86_64
```

Repeat for `aarch64` on an aarch64 machine. The artifact names are
`goshpad-vX.Y.Z-linux-<arch>.tar.gz` and `.flatpak`. Put them in one
directory with a `SHA256SUMS` file and run:

```bash
scripts/verify-release.sh <dir> X.Y.Z
```

That script expects exactly those four artifacts plus `SHA256SUMS`. The
tarball must contain an executable `bin/goshpad` of the claimed architecture.
The Flatpak bundle must contain the ref
`app/com.goshapps.GoshPad/<arch>/stable`.

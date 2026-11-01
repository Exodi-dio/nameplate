# Nameplate

Draws your own Geometry Dash username in a pickable font, with a two-colour gradient
for both the text fill and the outline.

## How it works

Open your own profile page and tap the small **Aa** button next to your name. A popup lets you:

- Cycle through bundled fonts (arrow buttons)
- See a live preview of your actual username
- Pick top/bottom colours for the text fill
- Pick top/bottom colours for the outline
- Save

Only your own account name is ever shown — the mod cannot display anyone else's name,
so it can't be used to impersonate another player.

## Fonts

All fonts are bundled with the mod (you never need to find a TTF yourself) and are
licensed under the SIL Open Font License. See `resources/licenses/` for each family's
`OFL.txt`.

## Building

This is a Geode mod for GD 2.2091 (Android). Build with the Geode SDK nightly:

```
cmake -B build -DGEODE_SDK=/path/to/geode-sdk
cmake --build build
```

See `.github/workflows/build.yml` for the CI build that produces the `.geode`.

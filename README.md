# dispImageSet
This is a small app that display the ImageSet of a NFT marker. It show all the different images with the basic infos (width, height and dpi).

## Building

Build with Emscripten emsdk 3.1.19. Run :
```
npm run build
```
If you have already build the libar.bc run instead:
```
npm run build-no-libar
```

## Example

Start a web server and try the [displayer](./example/displayer.html) example.

## Planned features

- [ ] A real demo with pick up images to let the user choose the imageSet on the fly.
- [ ] Build the libs with ES6 feature.
- [ ] Typescript definitions.

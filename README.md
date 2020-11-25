# foo_out_pulse
Native event-based PulseAudio output for foobar2000, for use in Wine.

## Setup
This component connects to 127.0.0.1 using Pulse's native TCP protocol.
First you have to enable the module for this protocol for the Pulse daemon by editing either
`/etc/pulse/default.pa` or `~/.config/pulse/default.pa` and adding or editing the following line:

`load-module module-native-protocol-tcp`

Next copy or link the Pulseaudio cookie into your Wine prefix, for example:

`ln -s ~/.config/pulse/cookie ~/.wine/drive_c/users/<name>/.config/pulse/cookie`

Make sure the Pulseaudio daemon is actually running, as it cannot be automatically activated over TCP.

For the icon to appear in the mixer, you will need to place a png of foobar's icon named `foobar` in, for example, `~/.icons/hicolor/256x256/apps/foobar.png`

Wine 5.14+ is required for Pulseaudio to be able to read the cookie file, due to <a href="https://source.winehq.org/git/wine.git/commitdiff/587732acb7834a52a2af5bb45e59899ab8ec3f5d">this change</a> - <a href="https://hydrogenaud.io/index.php?topic=119763.msg988559#msg988559">see this discussion</a> for context.

### Known bugs
- With some sound cards, playback seems to be sped up when Pulseaudio is using its default timer based scheduling mode. You could try switching to interrupt scheduling (tsched=0) or try enabling the workaround in Advanced Preferences.

## Advantages
Foobar works very well in Wine, but Wine's default Pulseaudio output has some disadvantages. Compared to using the standard output, this component
- May be less liable to cause audio dropouts when performing IO-intensive tasks such as tag writing, as it buffers as much as possible on the Pulse end,
- Has a custom fading implementation to prevent pops on seek/track change, that doesn't require reducing Pulseaudio's buffer length to work (configure in Advanced Preferences -> Playback -> Pulseaudio output),
- Doesn't resample unnecessarily, making the most of Pulseaudio's `avoid-resampling` option,
- Can show foobar's icon in the system mixer,
- Integrates with Pulseaudio's volume control.

## Todo
Currently it
- Only connects to localhost and doesn't support connecting via Unix socket like normal programs,
- Probably doesn't properly pass through channel mappings from source files correctly.

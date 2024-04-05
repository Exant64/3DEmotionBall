# 3DEmotionBall

This is a "remake" (probably like the third) of my old 3D Emotion Ball mod, hopefully the last one.
Once it's finished I'm gonna upload it to GB.

So far the main changes from the old versions are:
- Code rewritten completely from scratch
- Hero Chaos Chao halo now works properly with the special blending,
thanks to Shaddatic's help figuring it out (separating the halo model to draw outside first and a special hack
to make it z-write)
- Specular effect, also thanks to Shad's help, he showed me a pretty clean and simple technique,
using a transparent environment map overlay to "emulate" specular
- The models are loaded externally and replaceable by other mods

Plans before release:
- Config option for neutral chaos/dark chaos emote balls to use the model or sprites for the emotions (like exclamation etc.)
- DC-like specular, and being able to swap between this and the environment map version
(this is a tougher one because it would probably require custom shaders, not sure if I'll go ahead with it)
- Maaaybe an option to give Neutral Chaos a yellow emote ball color and Dark Chaos a purple one

Credit to:
- mothnox for spiky ball model
- Shad for all the help so far with the blending and the env map stuff
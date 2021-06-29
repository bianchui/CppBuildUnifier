Non Ordered Makefiles
---------------------



Nom is a parser for the android build system. The one to build android itself, not for apps.
Android uses (gnu)make, which i love for its simplicity. But it's also very slow, because people.
(Google built like 14 at the time of this writing, which are all shit)

Nom doesn't fix the build system, instead it parses the entire android tree into
structures for whatever someone else might do with it.

You could probably visualize components dependencies, or try to build a subtree (mmm is junk, come on).
The original idea behind nom was to do incremental builds really quickly, but i'm no longer paid to work on
android, because you know Nokia kinda died.


State
--------------

I don't remember. I think it can parse most of the android tree with some exceptions.
There are massive syntax errors all over the android source that gnumake just skips, but nom chokes on them.
Like missing paranthesis and unterminated quotes. You gotta fix those.


If you're planning to use this to actually build things, prelude.mk will make you very unhappy.
It just includes a minimal working environment to get stuff parsed. I doubt this will produce valid compiler commands.
If i remember correctly, i never managed to parse build/\*.mk properly, because it's just so broken.


Future
-------------

This has no future. It is MIT licensed, so go ahead and copy it into whatever you'd like to do.
I'll happily answer questions, but working for free to fix problems caused by expensive valley hipsters,
turns out to be a terrible life choice.

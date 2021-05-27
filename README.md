# y262dec

y262dec is a mpeg-1/2 video decoder

# features
* Pretty much decodes all Mpeg-1 and Mpeg-2 bitstreams to YUV
* Slice based threading

# How to build
You need cmake.
You need a C compiler supported by cmake.

In the root directory of the y262dec directory:
```bash
mkdir build
cd build
cmake -G "Visual Studio 16 2019" -A x64 ..
```
You probably need to replace the generator and architecture with what you have.
Then build !


# Running y262dec

Once you have your executable you can run the decoder like so:
```
        y262decapp <bitstream in> <yuv out>
```

# Notes

y262dec's architecture draws heavily from Michel 'Walken' Lespinasse libmpeg2 decoder. libmpeg2 is faster but lacks threading and is GPL licensed. y262dec is a bit slower but has threading and is BSD licensed.

Patent situation regarding Mpeg2 you best check with the [MPEG LA](https://www.mpegla.com) or ask your legal department.

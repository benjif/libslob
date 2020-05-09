# libslob

C++ library providing support for reading from Slob (sorted list of blobs) dictionary files.

## Background

Slob is a read-only, compressed format that was created for the [Aard dictionary reader](https://github.com/itkach/aard2-android). The keys are sorted using the [Unicode Collation Algorithm](https://www.unicode.org/reports/tr10/).

## Dictionaries

You can find a collection of dictionaries in the Slob format [here](https://github.com/itkach/slob/wiki/Dictionaries).

## Dependencies

- cmake >= 2.8
- [icu](http://site.icu-project.org/home)
- [liblzma](https://tukaani.org/xz/)
- [zlib](https://www.zlib.net)

`sudo apt install cmake liblzma-dev zlib1g-dev libicu-dev`

## Install

```
mkdir cmake
cd cmake
cmake ..
make
sudo make install
```

## Usage

### Reading

```c++
SLOBReader s_reader;
s_reader.open("wordnet-3.1.slob");

SLOBDict dict(s_reader);

auto matches = dict["searchterm"];

if (matches.size() > 0)
  ...
```

## License

BSD 2-clause license.

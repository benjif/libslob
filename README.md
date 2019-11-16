# libslob

C++ library providing support for reading from Slob (sorted list of blobs) dictionary files.

## Background

Slob is a read-only, compressed format that was created for the (https://github.com/itkach/aard2-android)[Aard dictionary reader]. The keys are sorted using the (https://www.unicode.org/reports/tr10/)[Unicode Collation Algorithm].

## Dependencies

`sudo apt install liblzma-dev zlib1g-dev libicu-dev`

- (https://tukaani.org/xz/)[liblzma]
- (https://www.zlib.net)[zlib]
- (http://site.icu-project.org/home)[icu]

## License

BSD 2-clause license.

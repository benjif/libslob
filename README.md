# libslob

C++ library providing support for reading from Slob (sorted list of blobs) dictionary files.

## Background

Slob is a read-only, compressed format that was created for the (Aard dictionary reader)[https://github.com/itkach/aard2-android]. The keys are sorted using the (Unicode Collation Algorithm)[https://www.unicode.org/reports/tr10/].

## Dependencies

- (liblzma)[https://tukaani.org/xz/]
- (zlib)[https://www.zlib.net]
- (icu)[http://site.icu-project.org/home]

## License

BSD 2-clause license.

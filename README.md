# MD5 Project

### Outline

There are many example of MD5 hash/checksum generators on internet, like the `md5sum` that comes with most unix ditributions, but as usual I like to build my own, just to get some understanding of the process involved, and to add some of my own options...

As you can read in a [Wiki][1] - `The MD5 message-digest algorithm is a widely used hash function producing a 128-bit hash value.` - as a checksum to verify data integrity, but only against unintentional corruption.

As you will read here and there, certain vunerablilites have been exposed, and one should probably use `sha1`, `sha2`, or later, but that said the MD5 sum is a simple way to verify that a file download from internet is the original, with a high degree of certainety, if the site also provides the origibal checksum.

This repository replaces my previous [MD5][2] homesite site page, which provided the MD5 source in zip files, giving the `Digest` to verify the download.

It also includes a `md5-tc.c` source, recently found on the [creationix][3] site, just as another sample to check the algorim is correct... thanks...

  [1]: https://en.wikipedia.org/wiki/MD5
  [2]: http://geoffair.com/ms/dc4w.htm#md5
  [3]: https://gist.github.com/creationix/4710780  
  
### Building

Uses [CMake][10] to generate the build files of your choice.

```
  cd build  # do not build in the root
  cmake -S .. [options] [-A Win32|x64]
  cmake --build . --config Release
```

The `options` include -

```
   * `-G "name of generator"` to select what you will use to compile and link the applications.
   * `-DCMAKE_INSTALL_PREFIX:PATH=/path/for/install`, defaults /usr/local in unix.
   * The are many other cmake options...
```

The is a build-me.bat file, which can maybe be modified to suit your environment...

  [10]: https://cmake.org/download/

### License

Where applicable, this project is under -

```
   * GNU GPL version 2
   * Copyright (c) 1089 ... 2020 - Geoff R. McLane
```

See [LICENSE.txt][20] for details.

Some source files contain their own copyright information.

  [20]: LICENSE.txt
  
### Usage

As stated, the aim of this app is to verify the checksum given on the site, if any, then, say you have downloaded `md5-01.zip` from ymy site above the command and output would be -

```
<download/dir> md5 md5-01.zip "-d=75f3e70e4b57ef48ebfc3e60112dbfc1"
Input digest is equal.
MD5 ("md5-01.zip") = 75f3e70e4b57ef48ebfc3e60112dbfc1 dated 2013-04-21 10:09:29 (utc), of 14,923 bytes.
```

Use -?, --help, to view the brief help.
   
Have FUN... Geoff... 20200610

; eof

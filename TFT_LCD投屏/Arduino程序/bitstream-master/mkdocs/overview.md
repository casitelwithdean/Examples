
Overview
================================================================================

Bitstream is a [Python] library which manages binary data as bit streams.

  - **Binary Data:** the `BitStream` class is a linearly ordered container of bits.
    The standard library is only convenient to manage binary data at the byte level. 
    Consider using BitStream instead, especially you need to address the bit level.

  - **Stream Interface:** you can only read data at the start of a stream 
    and write data at its end.
    This is a very simple way to interact with binary data, but it is also
    the pattern that comes naturally in many applications. To manage
    binary codes and formats, in my experience, random data access is 
    not a requirement.

  - **Python and Numpy Types.** BitStream has built-in readers and writers
    for the common data types with a standard binary layout: bools, 
    ASCII strings, fixed-size integers and floating-point integers. 

  - **User-Defined Types.** The list of supported types and binary 
    representation may be enlarged at will: new readers and writers 
    can be implemented and associated to specific data types.

  - **Performance.** Bitstream is a Python C-extension module that has been
    optimized for the common use cases. Hopefully, it will be fast enough 
    for your needs ! Under the hood, the [Cython] [] language and compiler 
    are used to generate this extension module.

  - **Open-Source.** Bitstream  is distributed under a [MIT license]. 
    The development takes place on [GitHub] and releases are available on [PyPI].


[Markdown]: http://daringfireball.net/projects/markdown/
[CC-BY-3.0]: http://creativecommons.org/licenses/by/3.0/
[struct]: http://docs.python.org/2/library/struct.html
[Python]: http://www.python.org/
[Cython]: http://www.cython.org
[bitarray]: https://pypi.python.org/pypi/bitarray
[bitstring]: https://code.google.com/p/python-bitstring
[MIT license]: https://github.com/boisgera/bitstream/blob/master/LICENSE.txt
[GitHub]: https://github.com/boisgera/bitstream
[PyPI]: https://pypi.python.org/pypi/bitstream/

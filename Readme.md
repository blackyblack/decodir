LZSS Iridium data decoder.

Usage: decodir <input_file> <output_file>

input_file structure:
<version> - 1 byte,
<length> - 2 bytes,
<LZSS packed data> - length bytes.

output_file is JSON encoded data.
Using the command line can convert integer representation

void repr_convert(char source_repr, char target_repr, 
                  unsigned int repr)

This function takes an unsigned integer (repr) that provides the 32-bit representation of an integer and converts it from one integer representation to another. The function prints the number in the target representation as an eight-digit hexadecimal value. Include a single newline at the end of the output. Include leading zeros in the output to pad out the eight digits. Any digits which are given as letters must be printed in lowercase.

source_repr: the character '1', '2', 'S 'or 'D', which indicates the integer representation the function is converting from. '1' indicates that the repr argument provides the 32-bit one's complement representation of a number. Likewise, '2' indicates that the input is a 32-bit two's complement representation. 'S' indicates that the input is a 32-bit sign/magnitude representation. 'D' indicates you should encode the input number in a home-grown variant of signed, binary coded decimal format, which is described at the linked page.

target_repr: the character '1', '2', 'S' or 'D', which indicates the integer representation the function is converting into. '1', '2', 'S' and 'D' have similar interpretations for the source_repr argument, except that this character indicates the target representation. 

repr: an unsigned, 32-bit value that represents an integer in the provided source representation.


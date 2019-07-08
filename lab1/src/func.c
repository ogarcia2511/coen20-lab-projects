// TwosComplement: writes the opposite-sign value of "input" in two's complement form to "output".
void TwosComplement(const int input[8], int output[8])
{
    int first = 0;
    int check = 0;

    // this block of code checks for the special case of input being all '0's
    for(int m = 0; m < 8; ++m)
    {
        if(input[m] == 1)
            check = 1;
    }

    // handling the special case: if input is all '0's then set output to all '0's and return
    if(check == 0)
    {
        for(int n = 0; n < 8; ++n)
            output[n] = 0;

        return;
    }

    // finds the first '1' in input and saves its position
    for(int i = 0; i < 8; ++i)
    {
        if(input[i] == 1)
        {
            first = i;
            break;
        }
    }

    // everything before (and including) the first '1' is copied over directly
    for(int j = 0; j <= first; ++j)
    {
        output[j] = input[j];

    }

    // everything after the first '1' is flipped from '0' to '1' or vice-versa
    for(int k = first + 1; k < 8; ++k)
    {
        if(input[k] == 1)
            output[k] = 0;
        else
            output[k] = 1;
    }

    return;
}

// Bin2Dec: converts the input "bin" from binary to a float, which is scaled down and returned.
float Bin2Dec(const int bin[8])
{
    float dec;

    // subtracts the sign bit
    dec = -(bin[7]);

    // uses method described in lab manual to add non-sign bits
    for(int i = 6; i >= 0; --i)
    {
        dec = (2 * dec) + bin[i];
    }

    // scale the result down and return it
    return (dec / 128);
}

//Dec2Bin: rounds the input "x", scales it up, and converts it to binary (saved in "bin").
void Dec2Bin(const float x, int bin[8])
{
    // scales up the input
    float num = x * 128;

    // to handle the special case of negative numbers - make them positive for math...
    if(x < 0.0)
        num = num * -1;

    // rounds the input without using external libraries (math.h)
    int val = (int)(num < 0 ? (num - 0.5) : (num + 0.5));

    int temp[8];

    // fill the binary array with leading zeros in case division does not fill array
    for(int j = 0; j < 8; ++j)
    {
        bin[j] = 0;
        temp[j] = 0;
    }

    // repeated division for the conversion from decimal to binary
    int i = 0;
    while(val != 0 && i < 8)
    {
        temp[i] = val % 2;
        ++i;
        val = val / 2;
    }

    // handling negative numbers pt. 2: ... then if the number was negative call TwosComplement to flip back
    // *also handling case of '-1', which should not get flipped
    if(x < 0.0 && x != -1.0)
        TwosComplement(temp, bin);
    else
        for(int k = 0; k < 8; ++k)
            bin[k] = temp[k];

    return;
}
#include "ril_utils.h"
#include "lv_pocket_router/src/util/debug_log.h"

static const int halfShift  = 10; /* used for shifting by 10 bits */
static const unsigned long halfBase = 0x0010000UL;
static const unsigned long halfMask = 0x3FFUL;

static const unsigned char firstByteMark[ 7 ] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

int ril_nas_convert_gsm8bit_alpha_string_to_utf8
(
    const char *gsm_data,
    uint16 gsm_data_len,
    char *utf8_buf,
    size_t utf8_buf_sz
)
{
    size_t i, j;
    uint8 hi_utf8, lo_utf8;
    uint16 unicode;
    uint16 ret_value = 0;

    unsigned int gsm_char;

    do
    {
      if ( ( gsm_data == NULL ) || ( gsm_data_len == 0 ) || ( utf8_buf == NULL ) || ( utf8_buf_sz == 0 ) )
      {
        log_e( "Invalid parameters for GSM alphabet to UTF8 conversion, input len = %d", gsm_data_len );
        break;
      }

      for ( i = 0, j = 0; i < gsm_data_len && j < utf8_buf_sz - 1; i++ )
      {
        if(gsm_data[i] == 0x0D)
        {
          /* ignoring the non-standard representation of new line (carriage return charecter is also included along with new line)*/
          log_d( "ignored CR charecter at index = %d", i);
          continue;
        }

        gsm_char = (unsigned int)gsm_data[ i ];
        if ( gsm_char <= 127 )
        {
          unicode = gsm_def_alpha_to_utf8_table[ gsm_char ];
          hi_utf8 = ( uint8 ) ( ( unicode & 0xFF00 ) >> 8 );
          lo_utf8 = ( uint8 ) unicode & 0x00FF;

          /* Make sure to only write a pair of bytes if we have space in the buffer */
          if ( (hi_utf8 > 0 && j + 1 < utf8_buf_sz - 1) || (hi_utf8 == 0) )
          {
            if ( hi_utf8 > 0 )
            {
              utf8_buf[ j++ ] = hi_utf8;
            }

            utf8_buf[ j++ ] = lo_utf8;
          }
        } // if ( gsm_char <= 127 )
        else
        {
          utf8_buf[ j++ ] = (char) gsm_char;
        }
      }

      utf8_buf[ j ] = '\0';

      ret_value =  j;
    } while(0);

    return ret_value;
}

uint16 ril_nas_ussd_unpack
(
    unsigned char *str,
    size_t str_sz,
    const unsigned char *packed_data,
    unsigned char num_bytes
)
{

  unsigned char stridx = 0;
  unsigned char pckidx = 0;
  unsigned char prev = 0;
  unsigned char curr = 0;
  unsigned char shift;

  if(packed_data != NULL && str != NULL && str_sz != 0)
  {
    if (str_sz > 255) str_sz = 255;
    while(pckidx < num_bytes && stridx < str_sz)
    {
      shift = stridx & 0x07;
      curr = packed_data[pckidx++];

      /* A 7-bit character can be split at the most between two bytes of packed
      ** data.
      */
      str[stridx++] = ( (curr << shift) | (prev >> (8-shift)) ) & 0x7F;

      /* Special case where the whole of the next 7-bit character fits inside
      ** the current byte of packed data.
      */
      if(shift == 6)
      {
        /* If the next 7-bit character is a CR (0x0D) and it is the last
        ** character, then it indicates a padding character. Drop it.
        ** Also break if we reached the end of the output string.
        */

        if(((curr >> 1) == CHAR_CR && pckidx == num_bytes) || (stridx == str_sz))
        {
          break;
        }
        str[stridx++] = curr >> 1;
      }
      prev = curr;
    }
  }
  else
  {
    log_d("FATAL : CHECK FAILED");
  }

  return stridx;

} /* ril_nas_ussd_unpack */

uint16 ril_nas_convert_gsm_def_alpha_string_to_utf8
(
  const char *gsm_data,
  char        gsm_data_len,
  char       *utf8_buf,
  size_t      utf8_buf_sz
)
{
  UTF16  num_chars;
  char  *temp_buf;
  UTF16  ret_value = 0;

  do
  {
    if ( ( gsm_data == NULL ) || ( gsm_data_len == 0 ) || ( utf8_buf == NULL )  || (utf8_buf_sz == 0) )
    {
      log_d( "Invalid parameters for GSM alphabet to UTF8 conversion, input len = %d", gsm_data_len );
      break;
    }

    /* Allocate buffer */
    temp_buf = (char *) malloc( gsm_data_len * 2 );
    if ( temp_buf == NULL )
    {
      log_d( "Fail to allocate buffer for GSM alphabet to UTF8 conversion" );
      break;
    }

    /* Unpack the string from 7-bit format into 1 char per byte format */
    num_chars = ril_nas_ussd_unpack( temp_buf, gsm_data_len * 2, (const byte *) gsm_data, gsm_data_len );

    ret_value = ril_nas_convert_gsm8bit_alpha_string_to_utf8((const char *)temp_buf, num_chars, utf8_buf, utf8_buf_sz);

    free( temp_buf );

  }while(0);

  return ret_value;

} /* ril_nas_convert_gsm_def_alpha_string_to_utf8() */


void ril_nas_ons_decode_packed_7bit_gsm_string
(
    const uint8 *src,
    size_t       src_length,
    char        *dest,
    size_t       dest_sz
)
{
    unsigned char dest_length = 0;

    if(  dest != NULL && src != NULL )
    {
        dest_length = ril_nas_convert_gsm_def_alpha_string_to_utf8( ( const char * ) src, src_length, dest, dest_sz );

        /* Spare bits is set to '0' as documented in 3GPP TS24.008 Section 10.5.3.5a, and
        the CM util function unpacks it assuming USSD packing (packing for 7 spare bits is carriage return = 0x0D).
        Thus, an '@' is appended when there are 7 spare bits. So remove it. */
        if ( !( src_length % 7 ) && !( src[ src_length - 1 ] & 0xFE ) && ( dest[ dest_length - 1 ] == '@' ) )
        {
            log_d( "Detected 7 spare bits in network name, removing trailing @");
            dest[ dest_length - 1 ] = '\0';
        }
    }
    else
    {
        log_d("FATAL : CHECK FAILED");
    }
}

int ril_nas_ConvertUTF16toUTF8
(
    const unsigned short ** sourceStart,
    const unsigned short * sourceEnd,
    unsigned char ** targetStart,
    unsigned char * targetEnd,
    int flags
)
{
    int result = 0;
    const unsigned short * source = *sourceStart;
    unsigned char * target = *targetStart;

    while ( source < sourceEnd )
    {
        unsigned long ch;
        unsigned short bytesToWrite = 0;
        const unsigned long byteMask = 0xBF;
        const unsigned long byteMark = 0x80;
        const unsigned short

        * oldSource = source; /* In case we have to back up because of target overflow. */

        ch = *source++;

        /* If we have a surrogate pair, convert to unsigned long first. */
        if ( ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END )
        {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            if ( source < sourceEnd )
            {
                unsigned long ch2 = *source;
                /* If it's a low surrogate, convert to unsigned long. */
                if ( ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END )
                {
                ch = ( ( ch - UNI_SUR_HIGH_START ) << halfShift ) + ( ch2 - UNI_SUR_LOW_START ) + halfBase;
                ++source;
                }
                /* it's an unpaired high surrogate */
                else if ( flags == 0 )
                {
                --source; /* return to the illegal value itself */
                result = 3;
                break;
                }
            }
            /* We don't have the 16 bits following the high surrogate. */
            else
            {
            --source; /* return to the high surrogate */
            result = 2;
            break;
            }
        }
        else if ( flags == 0 )
        {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            if ( ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END )
            {
            --source; /* return to the illegal value itself */
            result = 3;
            break;
            }
        }

        /* Figure out how many bytes the result will require */
        if ( ch < ( unsigned long ) 0x80 )
        {
        bytesToWrite = 1;
        }
        else if ( ch < ( unsigned long )0x800 )
        {
        bytesToWrite = 2;
        }
        else if ( ch < ( unsigned long ) 0x10000 )
        {
        bytesToWrite = 3;
        }
        else if (ch < ( unsigned long ) 0x110000 )
        {
        bytesToWrite = 4;
        }
        else
        {
        bytesToWrite = 3;
        ch = UNI_REPLACEMENT_CHAR;
        }

        target += bytesToWrite;
        if ( target > targetEnd )
        {
        source = oldSource; /* Back up source pointer! */
        target -= bytesToWrite; result = 2;
        break;
        }

        switch (bytesToWrite)
        {
        /* note: everything falls through. */
        case 4: *--target = ( unsigned char ) ( ( ch | byteMark ) & byteMask ); ch >>= 6;
        case 3: *--target = ( unsigned char ) ( ( ch | byteMark ) & byteMask ); ch >>= 6;
        case 2: *--target = ( unsigned char ) ( ( ch | byteMark ) & byteMask ); ch >>= 6;
        case 1: *--target = ( unsigned char ) ( ch | firstByteMark[ bytesToWrite ] );
        }

        target += bytesToWrite;

    }

    *sourceStart = source;
    *targetStart = target;

    return result;

}


int ril_nas_convert_ucs2_to_utf8
(
    const char *ucs2str,
    size_t ucs2str_len,
    char *utf8str,
    size_t utf8str_sz
)
{
    UTF8 utf8_buf[ MAX_USS_CHAR * 2 ];
    UTF16 *utf16SourceStart, *utf16SourceEnd;
    UTF8 *utf8Start = (UTF8 *)utf8str, *utf8End;
    ConversionResult result;
    size_t length = 0;
    size_t max_utf8_len = utf8str_sz - 1;

    if ( !ucs2str || ucs2str_len == 0 )
    {
        result = sourceExhausted;
    }
    else if ( !utf8str || utf8str_sz == 0)
    {
        result = targetExhausted;
    }
    else
    {
        utf8Start = (UTF8 *)utf8str;
        utf8End = (UTF8 *)utf8str + max_utf8_len;

        utf16SourceStart = ( UTF16 * ) ucs2str;
        utf16SourceEnd = ( UTF16 * ) ( ucs2str + ucs2str_len );

        result = ril_nas_ConvertUTF16toUTF8( (const UTF16 **) &utf16SourceStart, utf16SourceEnd,
                                                &utf8Start, utf8End, lenientConversion );
    }

    if ( result == targetExhausted )
    {
        log_d( "String has been truncated. Buffer size of %zu not large enough", utf8str_sz );
    }
    else if ( result != conversionOK )
    {
        log_d( "Error in converting ucs2 string to utf8" );
    }

    length = (size_t) ( utf8Start - (UTF8 *)utf8str );

    if ( length > max_utf8_len)
    {
        length = max_utf8_len;
        log_d( "Bug in cri_nas_ConvertUTF16toUTF8. Buffer overrun detected. "
                       "Size %zu greater than %zu",
                        length, max_utf8_len);
    }

    if(utf8str)
    {
        utf8str[length] = '\0';
    }

    return length;

}

void ril_nas_decode_operator_name_in_little_endian
(
    char *dest,
    unsigned short max_dest_length,
    int coding_scheme,
    const unsigned char *src,
    unsigned short src_length
)
{
    unsigned char data_length;

    if ( dest!= NULL && src != NULL && src_length > 0 )
    {
        data_length = ( src_length > max_dest_length ) ? max_dest_length : src_length;

        switch ( coding_scheme )
        {
            case QMI_CODING_SCHEME_CELL_BROADCAST_DATA:
                log_d( "7-bit coding scheme for NITZ ONS" );
                ril_nas_ons_decode_packed_7bit_gsm_string( src, src_length, dest, data_length );
                log_d( "NITZ 7-bit GSM str: %s\n", dest );
            break;

            case QMI_CODING_SCHEME_UCS2:
                log_d( "UC2 coding scheme for NITZ ONS, len %d", data_length );
                if ( ( data_length % 2 ) != 0 )
                {
                log_d( "Invalid UCS length %d", data_length );
                break;
                }
                (void) ril_nas_convert_ucs2_to_utf8( (char *)src, data_length, dest, data_length );
                log_d( "NITZ UCS str: %s", dest );
            break;

            default:
                log_d( "Unknown coding scheme %d for NITZ ONS", coding_scheme );
            break;
        }
    }
    else
    {
        log_d("ril_nas_decode_operator_name_in_little_endian CHECK FAILED");
    }

} // ril_nas_decode_operator_name_in_little_endian
